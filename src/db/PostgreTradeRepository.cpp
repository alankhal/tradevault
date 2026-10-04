#include "tradevault/PostgresTradeRepository.hpp"

#include <pqxx/pqxx>
#include <utility>

PostgresTradeRepository::PostgresTradeRepository(std::string connectionString): m_connectionString(std::move(connectionString)){}

//Require namespace for enums in order to properly transfer them for database data types 
namespace
{
    std::string sideToString(TradeSide side)
    {
        switch (side)
        {
            case TradeSide::Buy:  return "BUY";
            case TradeSide::Sell: return "SELL";
        }
        throw std::invalid_argument("Unknown TradeSide value");
    }

    std::string statusToString(TradeStatus status)
    {
        switch (status)
        {
            case TradeStatus::Pending:   return "PENDING";
            case TradeStatus::Filled:    return "FILLED";
            case TradeStatus::Cancelled: return "CANCELLED";
        }
        throw std::invalid_argument("Unknown TradeStatus value");
    }
}



void PostgresTradeRepository::storeTrade(const Trade& trade)
{
    pqxx::connection connection{m_connectionString};

    pqxx::work transaction{connection};

    //Insert written htis way in order to prevent SQL injection and keep data safer rather than trade.getInstrument 
    const std::string sql = "INSERT INTO trades " "(trade_id, instrument, counterparty, side, status, price, quantity, timestamp) " 
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8)";

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