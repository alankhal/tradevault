#include "tradevault/TradeService.hpp"

//All of these functions take the implementation of InMemoryTradeRepo, this will save time later one when forming POSTgre 
//Added try and catch blocks so service layer can catch a database/repository failure and translate it into your application’s own error type.

TradeService::TradeService(TradeRepository& repository)
    : m_repository(repository)
{}

Result<Trade, TradeError> TradeService::createTrade(
    const std::string& instrument,
    const std::string& counterparty,
    TradeSide side,
    double price,
    int quantity
) 
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
    //At this point ID 0 means the trade has not yet been stored in the database
    Trade trade(instrument, counterparty, side, price, quantity); 

    //Uses storeTrade made in 
    try
    {
        //PostgreSQL stores the trade, generates its permanent ID,
        //and the repository returns the completed Trade object
        Trade storedTrade = m_repository.storeTrade(trade);

        return Result<Trade, TradeError>::success(storedTrade);
    }
    catch (const std::runtime_error&)
    {
        return Result<Trade, TradeError>::failure(TradeError::InternalError);
    }
}

//Get Trade Function 
Result<Trade, TradeError> TradeService::getTrade(unsigned tradeId) const 
{
    try
    {
        auto trade = m_repository.getTrade(tradeId);

        if (!trade.has_value())
        {
            return Result<Trade, TradeError>::failure(TradeError::TradeNotFound);
        }

        return Result<Trade, TradeError>::success(*trade); //Dereferences the optional and returns the Trade stored inside it
    }
    catch (const std::runtime_error&)
    {
        return Result<Trade, TradeError>::failure(TradeError::InternalError);
    }
}

//List Trades
//No class needed as an empty repository is not an error
std::vector<Trade> TradeService::listTrades() const
{
    try
    {
        return m_repository.listTrades();
    }
    catch (const std::runtime_error&)
    {
        return {};
    }
}


//Cancel Trade
Result<Trade, TradeError> TradeService::cancelTrade(unsigned tradeId)
{
    try
    {
        //Finds the trade and gets a copy of the trade
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
    catch (const std::runtime_error&)
    {
        return Result<Trade, TradeError>::failure(TradeError::InternalError);
    }
}
