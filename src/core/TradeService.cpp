#include "tradevault/TradeService.hpp"

//All of these functions take the implementation of InMemoryTradeRepo, this will save time later one when forming POSTgre 

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

//Get Trade Function 
Result<Trade, TradeError> TradeService::getTrade(unsigned tradeId) const {
    
    auto trade = m_repository.getTrade(tradeId);

    if (!trade.has_value())
    {
        return Result<Trade, TradeError>::failure(TradeError::TradeNotFound);
    }

    return Result<Trade, TradeError>::success(*trade); //Why is there a pointer??? 

}

//List Trades
//No class needed as an empty repository is not an error
std::vector<Trade> TradeService::listTrades() const {
    return m_repository.listTrades();
}

//Cancel Trade
Result<Trade, TradeError> TradeService::cancelTrade(unsigned tradeId)
{   //Finds the trade and gets a copy of the trade
    auto trade = m_repository.getTrade(tradeId);

    if (!trade.has_value())
    {
        return Result<Trade, TradeError>::failure(TradeError::TradeNotFound);
    }
    //Finds out if the trade has been cancelled or not
    if (!trade->markCancelled())
    {
        return Result<Trade, TradeError>::failure(TradeError::TradeAlreadyCancelled);
    }
    //Updates the actual offical trade as we presently have only pulled a copy 
    m_repository.updateTrade(*trade);

    return Result<Trade, TradeError>::success(*trade);
}