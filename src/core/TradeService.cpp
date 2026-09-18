#include "tradevault/TradeService.hpp"


TradeService::TradeService(TradeRepository& repository)
    : m_repository(repository)
{}

Result<Trade, TradeError> TradeService::createTrade(const std::string& instrument, const std::string& counterparty, TradeSide side, double price, int quantity) 
{
    //Goes through all the validation we have listed
    if (!TradeValidator::isValidQuantity(quantity)) 
    {
        return Result<Trade, TradeError>::failure(TradeError::InvalidQuantity);
    }

    if (!TradeValidator::isValidPrice(price))
    {
        return Result<Trade, TradeError>::failure(TradeError::InvalidPrice);
    }

    if (!TradeValidator::isValidInstrument(instrument))
    {
        return Result<Trade, TradeError>::failure(TradeError::InvalidInstrument); 
    }

    if (!TradeValidator::isValidCounterparty(counterparty))
    {
        return Result<Trade, TradeError>::failure(TradeError::MissingCounterparty);
    }

    //Creates the offical trade object
    Trade trade(instrument, counterparty, side, price, quantity); 

    //Uses storeTrade made in 
    m_repository.storeTrade(trade);

    return Result<Trade, TradeError>::success(trade);
}

Result<Trade, TradeError> TradeService::getTrade(unsigned tradeId) const {
    
    auto trade = m_repository.getTrade(tradeId);

    if (!trade.has_value())
    {
        return Result<Trade, TradeError>::failure(TradeError::TradeNotFound);
    }

    return Result<Trade, TradeError>::success(*trade); //Why is there a pointer??? 

}

Result<Trade, TradeError> cancelTrade(unsigned tradeId);