//Offers typical functions within a stock microservice that allows the user to create, get, list and cancel there trades 
//Combines all of the features we have established thus far

#pragma once

#include "tradevault/Trade.hpp"
#include "tradevault/TradeError.hpp"
#include "tradevault/TradeRepository.hpp"
#include "tradevault/Result.hpp"

#include <string>
#include <vector>


class TradeService
{
private:
/*
Private member is extrememly important as each TradeService object contain a member function who is pointing to TradeRepository allowing for the use 
the use of TradeService's methods, this makes it easier to later pivot towards an in memory trade repository to postgre, rather than having to rewrite everything from scratch 
*/
    TradeRepository& m_repository; 

public:
    //Using explicit in order to be able to perform dependency injection 
    explicit TradeService(TradeRepository& repository);

    Result<Trade, TradeError> createTrade(
        const std::string& instrument,
        const std::string& counterparty,
        TradeSide side,
        double price,
        int quantity
    );

    Result<Trade, TradeError> getTrade(unsigned tradeId) const;

    std::vector<Trade> listTrades() const;


    Result<Trade, TradeError> cancelTrade(unsigned tradeId);
};