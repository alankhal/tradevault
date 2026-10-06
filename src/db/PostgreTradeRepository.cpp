#include "tradevault/PostgresTradeRepository.hpp"

#include <stdexcept>
#include <pqxx/pqxx>
#include <utility>
#include <format>


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
}



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

std::optional<Trade> PostgresTradeRepository::getTrade(unsigned tradeId) const
{
    // TODO

    return std::nullopt;
}

std::vector<Trade> PostgresTradeRepository::listTrades() const
{
    // TODO

    return {};
}

void PostgresTradeRepository::updateTrade(const Trade& trade)
{
    // TODO
}