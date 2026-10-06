#include "tradevault/Trade.hpp"
#include <utility>

//Initial Constructor/Initilization list 
Trade::Trade(const std::string& instrument, const std::string& counterparty, TradeSide side,
             double price, int quantity)
    : tradeId_(m_nextId++),
      m_instrument(instrument),
      m_counterparty(counterparty),
      m_side(side),
      m_status(TradeStatus::Booked),
      m_price(price),
      m_quantity(quantity),
      m_clock(std::chrono::system_clock::now()) {}

//Recounstruction constructor 
Trade::Trade(
    unsigned tradeId,
    std::string instrument,
    std::string counterparty,
    TradeSide side,
    TradeStatus status,
    double price,
    int quantity,
    std::chrono::system_clock::time_point timestamp
)
    : tradeId_(tradeId),
      m_instrument(std::move(instrument)),
      m_counterparty(std::move(counterparty)),
      m_side(side),
      m_status(status),
      m_price(price),
      m_quantity(quantity),
      m_clock(timestamp)
{
}



// Existing persisted trade, prevents someone from casucally inventing a new trade instead of pulling from the database 
Trade Trade::fromPersistence(
    unsigned tradeId,
    std::string instrument,
    std::string counterparty,
    TradeSide side,
    TradeStatus status,
    double price,
    int quantity,
    std::chrono::system_clock::time_point timestamp
)
{
    return Trade{
        tradeId,
        std::move(instrument),
        std::move(counterparty),
        side,
        status,
        price,
        quantity,
        timestamp
    };
}




// Getters for values
unsigned Trade::getTradeId() const { return tradeId_; }

std::string Trade::getInstrument() const { return m_instrument; }

std::string Trade::getCounterparty() const { return m_counterparty; }

TradeSide Trade::getSide() const { return m_side; }

TradeStatus Trade::getStatus() const { return m_status; }

double Trade::getPrice() const { return m_price; }

int Trade::getQuantity() const { return m_quantity; }

std::chrono::system_clock::time_point Trade::getTimestamp() const { return m_clock; }

//Method Functions 

bool Trade::markCancelled() {
    if (m_status == TradeStatus::Cancelled) {
        return false;
    }

    m_status = TradeStatus::Cancelled;
    return true;
}