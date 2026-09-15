//This will be used as the base/interface class for this project for other files

#pragma once
#include <vector>
#include <string>
#include "Trade.hpp"


class TradeRepository {
 
    public: 
    // Store 
    virtual void storeTrade(const Trade& other) = 0; 

    // The purpose of using std::optional is because it can return nothing if there is nothing within the return value put, this prevents possible errors and allows us to place an if statement for when !trade
    virtual std::optional<Trade> getTrade(unsigned tradeId) const = 0; 

    // List
    virtual std::vector<Trade> listTrades() const = 0; 
    
    //Destructor 
    virtual ~TradeRepository() = default;
};





