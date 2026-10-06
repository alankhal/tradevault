#include "tradevault/PostgresTradeRepository.hpp"

#include <stdexcept>
#include <pqxx/pqxx>
#include <utility>
#include <format>

//The actual database information 
PostgresTradeRepository::PostgresTradeRepository(std::string connectionString): m_connectionString(std::move(connectionString)){}

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

    std::string timestampToString(std::chrono::system_clock::time_point timestamp)
    {
        // Postgres stores microsecond precision, so truncate to that
        auto micros = std::chrono::floor<std::chrono::microseconds>(timestamp);
        return std::format("{:%Y-%m-%d %H:%M:%S}+00", micros);  
    }

    TradeSide stringToSide(const std::string& value)
    {
        if (value == "Buy") {return TradeSide::Buy;}

        if (value == "Sell"){return TradeSide::Sell;}

        throw std::invalid_argument("Unknown TradeSide value");
    }

    TradeStatus stringToStatus(const std::string& value)
    {
        if (value == "Booked"){return TradeStatus::Booked;}

        if (value == "Cancelled"){return TradeStatus::Cancelled;}

        throw std::invalid_argument("Unknown TradeStatus value");
    }

    std::chrono::system_clock::time_point microsToTimestamp(long long micros)
    {
         return std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::microseconds{micros})
        };
    }
}


//Keep trades from database
void PostgresTradeRepository::storeTrade(const Trade& trade)
{
    pqxx::connection connection{m_connectionString}; //Establishes the connection to the database using libpqxx
    pqxx::work transaction{connection}; //This is what begins the database work, and introduces the database to the actual C++ code, thus beggining the transaction  

    const std::string sql =     //Enters the users trade information 
        "INSERT INTO trades "
        "(trade_id, instrument, counterparty, side, status, price, quantity, timestamp) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8)";

    transaction.exec(   //This gives libpqxx the actual data
        sql,
        pqxx::params{
            trade.getTradeId(),
            trade.getInstrument(),
            trade.getCounterparty(),
            sideToString(trade.getSide()),
            statusToString(trade.getStatus()),
            trade.getPrice(),
            trade.getQuantity(),
            timestampToString(trade.getTimestamp())
        }
    );

    transaction.commit();
}

//Get singular Trade from the database
std::optional<Trade> PostgresTradeRepository::getTrade(unsigned tradeId) const
{
    // Opens a connection to PostgreSQL using the repository's stored connection string.
    pqxx::connection connection{m_connectionString};

    // Starts a read-only transaction for safely querying the database.
    pqxx::read_transaction transaction{connection};

    /* Executes a SELECT query for the trade with the requested ID.
    $1 is a parameter placeholder replaced by tradeId through pqxx::params.
    The timestamp is converted by PostgreSQL into microseconds since the Unix epoch
    so it can be converted back into a C++ time_point more easily.*/
    const auto result = transaction.exec(
        "SELECT trade_id, instrument, counterparty, side, status, price, quantity, "
        "(EXTRACT(EPOCH FROM timestamp) * 1000000)::bigint AS timestamp_micros "
        "FROM trades WHERE trade_id = $1",
        pqxx::params{tradeId}
    );

    // If no row was returned, the trade does not exist and std::nullopt represents an empty std::optional<Trade>.
    if (result.size() == 0)
    {
        return std::nullopt;
    }

    // Extracts the first returned database row, trade_id is the primary key, so there should only ever be one matching row.
    const auto row = result[0];

    // Reconstructs the existing Trade object using the exact values stored in PostgreSQL.
    // This avoids generating a new ID, status, or timestamp like the normal Trade constructor would.
    return Trade::fromPersistence(


        row["trade_id"].as<unsigned>(),
        row["instrument"].as<std::string>(),
        row["counterparty"].as<std::string>(),

        // Reads the database string ("Buy"/"Sell") and converts it back into TradeSide.
        stringToSide(row["side"].as<std::string>()),
        
        // Reads the database string ("Booked"/"Cancelled") and converts it back into TradeStatus.
        stringToStatus(row["status"].as<std::string>()),

        row["price"].as<double>(),
        row["quantity"].as<int>(),

        // Converts epoch microseconds from PostgreSQL back into a C++ system_clock::time_point.
        microsToTimestamp(row["timestamp_micros"].as<long long>())
    );
}

//List Trades from the Database
std::vector<Trade> PostgresTradeRepository::listTrades() const
{
    // Connect to PostgreSQL.
    pqxx::connection connection{m_connectionString};

    // Start a read-only transaction.
    pqxx::read_transaction transaction{connection};

    // Retrieve every stored trade & ORDER BY trade_id keeps the returned order predictable.
    const auto result = transaction.exec(
        "SELECT trade_id, instrument, counterparty, side, status, price, quantity, "
        "(EXTRACT(EPOCH FROM timestamp) * 1000000)::bigint AS timestamp_micros "
        "FROM trades ORDER BY trade_id"
    );

    // Vector that will contain the reconstructed Trade objects.
    std::vector<Trade> trades;

    // Optional optimization: we already know how many rows were returned.
    trades.reserve(result.size());

    // Go through every database row.
    for (const auto& row : result)
    {
         
        trades.push_back(Trade::fromPersistence(
            row["trade_id"].as<unsigned>(),
            row["instrument"].as<std::string>(),
            row["counterparty"].as<std::string>(),
            stringToSide(row["side"].as<std::string>()),
            stringToStatus(row["status"].as<std::string>()),
            row["price"].as<double>(),
            row["quantity"].as<int>(),
            microsToTimestamp(row["timestamp_micros"].as<long long>())
        ));


    }

    // Return every trade retrieved from PostgreSQL.
    return trades;
}


//Actually updating the database row without changing the TradeID
void PostgresTradeRepository::updateTrade(const Trade& trade)
{
    // Connect to PostgreSQL.
    pqxx::connection connection{m_connectionString};

    // Begin a write transaction.
    pqxx::work transaction{connection};

    // Update the existing database row that matches this Trade's ID.
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

        const auto result = transaction.exec(   //This gives libpqxx the actual data
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



    // UPDATE ... WHERE trade_id = $8 changes 0 rows if no trade has that ID.
    if (result.affected_rows() == 0)
    {
        throw std::runtime_error("updateTrade: no trade with id " + std::to_string(trade.getTradeId()));
    }
    
    transaction.commit();
}