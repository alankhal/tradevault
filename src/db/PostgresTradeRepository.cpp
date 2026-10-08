#include "tradevault/PostgresTradeRepository.hpp"

#include <stdexcept>
#include <pqxx/pqxx>
#include <format>


//The actual database information
//The repository now receives the shared connection pool instead of a connection string.
//This allows all repository methods to reuse existing PostgreSQL connections.
PostgresTradeRepository::PostgresTradeRepository(
    PostgresConnectionPool& connectionPool
)
    //Keep a reference to the shared pool created by the application.
    : m_connectionPool(connectionPool)
{}


//Require namespace for enums in order to properly transfer them for database data types
namespace
{
    std::string sideToString(TradeSide side)
    {
        switch (side)
        {
            case TradeSide::Buy:  return "Buy";
            case TradeSide::Sell: return "Sell";
        }

        throw std::invalid_argument("Unknown TradeSide value");
    }


    std::string statusToString(TradeStatus status)
    {
        switch (status)
        {
            case TradeStatus::Booked:    return "Booked";
            case TradeStatus::Cancelled: return "Cancelled";
        }

        throw std::invalid_argument("Unknown TradeStatus value");
    }


    std::string timestampToString(
        std::chrono::system_clock::time_point timestamp
    )
    {
        //Postgres stores microsecond precision, so truncate to that
        auto micros =
            std::chrono::floor<std::chrono::microseconds>(timestamp);

        return std::format(
            "{:%Y-%m-%d %H:%M:%S}+00",
            micros
        );
    }


    TradeSide stringToSide(const std::string& value)
    {
        if (value == "Buy")
        {
            return TradeSide::Buy;
        }

        if (value == "Sell")
        {
            return TradeSide::Sell;
        }

        throw std::invalid_argument("Unknown TradeSide value");
    }


    TradeStatus stringToStatus(const std::string& value)
    {
        if (value == "Booked")
        {
            return TradeStatus::Booked;
        }

        if (value == "Cancelled")
        {
            return TradeStatus::Cancelled;
        }

        throw std::invalid_argument("Unknown TradeStatus value");
    }


    std::chrono::system_clock::time_point microsToTimestamp(
        long long micros
    )
    {
        return std::chrono::system_clock::time_point{
            std::chrono::duration_cast<
                std::chrono::system_clock::duration
            >(
                std::chrono::microseconds{micros}
            )
        };
    }
}


//Keep trades from database
Trade PostgresTradeRepository::storeTrade(const Trade& trade)
{
    try
    {
        //Borrow an already-open PostgreSQL connection from the shared pool.
        //We no longer create a brand-new database connection for every request.
        auto connection = m_connectionPool.acquire();

        //Begin a write transaction using the borrowed connection.
        //The * is needed because connection is a shared_ptr to pqxx::connection.
        pqxx::work transaction{*connection};

        //PostgreSQL generates the trade ID and returns it after inserting the row.
        const std::string sql =
            "INSERT INTO trades "
            "(instrument, counterparty, side, status, price, quantity, timestamp) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7) "
            "RETURNING trade_id";

        const auto result = transaction.exec(
            sql,
            pqxx::params{
                trade.getInstrument(),
                trade.getCounterparty(),
                sideToString(trade.getSide()),
                statusToString(trade.getStatus()),
                trade.getPrice(),
                trade.getQuantity(),
                timestampToString(trade.getTimestamp())
            }
        );

        //Read the permanent ID PostgreSQL generated for this Trade.
        const unsigned generatedId =
            result[0]["trade_id"].as<unsigned>();

        //Commit makes the INSERT permanent in PostgreSQL.
        transaction.commit();

        //Rebuild the Trade using the permanent database ID.
        return Trade::fromPersistence(
            generatedId,
            trade.getInstrument(),
            trade.getCounterparty(),
            trade.getSide(),
            trade.getStatus(),
            trade.getPrice(),
            trade.getQuantity(),
            trade.getTimestamp()
        );

        //When this function ends, connection goes out of scope.
        //The connection pool's custom shared_ptr behavior returns the
        //connection to the pool instead of destroying it.
    }
    catch (const pqxx::failure& error)
    {
        //Convert any PostgreSQL/libpqxx failure into a standard C++ exception
        //so the rest of TradeVault does not depend on libpqxx exception types.
        throw std::runtime_error(
            "storeTrade database failure: " +
            std::string{error.what()}
        );
    }
}


//Get singular Trade from the database
std::optional<Trade> PostgresTradeRepository::getTrade(
    unsigned tradeId
) const
{
    try
    {
        //Borrow an existing PostgreSQL connection from the pool.
        auto connection = m_connectionPool.acquire();

        //Starts a read-only transaction using the borrowed connection.
        pqxx::read_transaction transaction{*connection};

        /*
        Executes a SELECT query for the trade with the requested ID.
        $1 is a parameter placeholder replaced by tradeId through pqxx::params.
        The timestamp is converted by PostgreSQL into microseconds since the Unix epoch
        so it can be converted back into a C++ time_point more easily.
        */
        const auto result = transaction.exec(
            "SELECT trade_id, instrument, counterparty, side, status, price, quantity, "
            "(EXTRACT(EPOCH FROM timestamp) * 1000000)::bigint AS timestamp_micros "
            "FROM trades WHERE trade_id = $1",
            pqxx::params{tradeId}
        );

        //If no row was returned, the trade does not exist and
        //std::nullopt represents an empty std::optional<Trade>.
        if (result.size() == 0)
        {
            return std::nullopt;
        }

        //Extracts the first returned database row.
        //trade_id is the primary key, so there should only ever be one matching row.
        const auto row = result[0];

        //Reconstructs the existing Trade object using the exact values stored in PostgreSQL.
        //This avoids generating a new ID, status, or timestamp.
        return Trade::fromPersistence(
            row["trade_id"].as<unsigned>(),
            row["instrument"].as<std::string>(),
            row["counterparty"].as<std::string>(),

            //Reads the database string ("Buy"/"Sell") and converts it back into TradeSide.
            stringToSide(
                row["side"].as<std::string>()
            ),

            //Reads the database string ("Booked"/"Cancelled") and converts it back into TradeStatus.
            stringToStatus(
                row["status"].as<std::string>()
            ),

            row["price"].as<double>(),
            row["quantity"].as<int>(),

            //Converts epoch microseconds from PostgreSQL back into a C++ time_point.
            microsToTimestamp(
                row["timestamp_micros"].as<long long>()
            )
        );
    }
    catch (const pqxx::failure& error)
    {
        throw std::runtime_error(
            "getTrade database failure: " +
            std::string{error.what()}
        );
    }
}


//List Trades from the Database
std::vector<Trade> PostgresTradeRepository::listTrades() const
{
    try
    {
        //Borrow an existing PostgreSQL connection from the pool.
        auto connection = m_connectionPool.acquire();

        //Start a read-only transaction using the borrowed connection.
        pqxx::read_transaction transaction{*connection};

        //Retrieve every stored trade.
        //ORDER BY trade_id keeps the returned order predictable.
        const auto result = transaction.exec(
            "SELECT trade_id, instrument, counterparty, side, status, price, quantity, "
            "(EXTRACT(EPOCH FROM timestamp) * 1000000)::bigint AS timestamp_micros "
            "FROM trades ORDER BY trade_id"
        );

        //Vector that will contain the reconstructed Trade objects.
        std::vector<Trade> trades;

        //Optional optimization: we already know how many rows were returned.
        trades.reserve(result.size());

        //Go through every database row.
        for (const auto& row : result)
        {
            trades.push_back(
                Trade::fromPersistence(
                    row["trade_id"].as<unsigned>(),
                    row["instrument"].as<std::string>(),
                    row["counterparty"].as<std::string>(),
                    stringToSide(
                        row["side"].as<std::string>()
                    ),
                    stringToStatus(
                        row["status"].as<std::string>()
                    ),
                    row["price"].as<double>(),
                    row["quantity"].as<int>(),
                    microsToTimestamp(
                        row["timestamp_micros"].as<long long>()
                    )
                )
            );
        }

        //Return every trade retrieved from PostgreSQL.
        return trades;
    }
    catch (const pqxx::failure& error)
    {
        throw std::runtime_error(
            "listTrades database failure: " +
            std::string{error.what()}
        );
    }
}


//Actually updating the database row without changing the TradeID
void PostgresTradeRepository::updateTrade(const Trade& trade)
{
    try
    {
        //Borrow an existing PostgreSQL connection from the pool.
        auto connection = m_connectionPool.acquire();

        //Begin a write transaction using the borrowed connection.
        pqxx::work transaction{*connection};

        //Update the existing database row that matches this Trade's ID.
        const std::string sql =
            "UPDATE trades "
            "SET instrument = $1, "
            "counterparty = $2, "
            "side = $3, "
            "status = $4, "
            "price = $5, "
            "quantity = $6, "
            "timestamp = $7 "
            "WHERE trade_id = $8";  //Without this line, an update could modify every trade in the table

        const auto result = transaction.exec(
            sql,
            pqxx::params{
                trade.getInstrument(),
                trade.getCounterparty(),
                sideToString(trade.getSide()),
                statusToString(trade.getStatus()),
                trade.getPrice(),
                trade.getQuantity(),
                timestampToString(trade.getTimestamp()),
                trade.getTradeId()
            }
        );

        //UPDATE ... WHERE trade_id = $8 changes 0 rows if no trade has that ID.
        if (result.affected_rows() == 0)
        {
            throw std::runtime_error(
                "updateTrade: no trade with id " +
                std::to_string(trade.getTradeId())
            );
        }

        //Make the UPDATE permanent.
        transaction.commit();
    }
    catch (const pqxx::failure& error)
    {
        throw std::runtime_error(
            "updateTrade database failure: " +
            std::string{error.what()}
        );
    }
}