#include "api/TradeJson.hpp"

namespace
{
    std::string sideToString(TradeSide side)
    {
        switch (side)
        {
        case TradeSide::Buy:
            return "Buy";

        case TradeSide::Sell:
            return "Sell";
        }

        return "Unknown";
    }

    std::string statusToString(TradeStatus status)
    {
        switch (status)
        {
        case TradeStatus::Booked:
            return "Booked";

        case TradeStatus::Cancelled:
            return "Cancelled";
        }

        return "Unknown";
    }
} // closes anonymous namespace

Json::Value TradeJson::toJson(const Trade& trade)
{
    Json::Value body;

    body["id"] = trade.getTradeId();
    body["instrument"] = trade.getInstrument();
    body["counterparty"] = trade.getCounterparty();
    body["side"] = sideToString(trade.getSide());
    body["status"] = statusToString(trade.getStatus());
    body["price"] = trade.getPrice();
    body["quantity"] = trade.getQuantity();

    return body;
}