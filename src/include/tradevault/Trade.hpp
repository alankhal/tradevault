#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <chrono>

//Enums declared earlier as the compiler needs to introduce these types before being asked to use them 
enum class TradeSide { Buy, Sell };

enum class TradeStatus { Booked, Cancelled };

class Trade {
    private:
        //All varaibles that would be listed in Trade must be stated in private in order to avoid trade fabrication from the rest of the program
        static inline unsigned m_nextId = 1;  // Initialized at 1 in the header, no need to restate it in class, easier to increment 
        unsigned tradeId_; //unsigned used in order to give approximetly 4 billion ID's 
        std::string m_instrument;
        std::string m_counterparty;
        TradeSide m_side;
        TradeStatus m_status;
        double m_price; 
        int m_quantity; 
        std::chrono::system_clock::time_point m_clock; // Contain Date, Hours, Minutes and Seconds in military time format, written in UTC time format 

    public: 
        //Rule of 0 operations as this class does not manually manage any resouces  
            Trade(const std::string& instrument, const std::string& counterparty, TradeSide side, double price, int quantity); : tradeId_(m_nextId++), m_instrument(instrument), m_countrerparty(counterparty), 
            m_price(price), m_quantity(quantity), m_clock(std::chrono::system_clock::now()){};
        

        //Getters for values
        Trade Id getTradeId() const;
        std::string getInstrument() const;
        std::string getCounterparty() const;
        TradeSide getSide() const;
        TradeStatus getStatus() const;
        double getPrice() const;
        int getQuantity() const;

        std::chrono::system_clock::time_point getTimestamp() const;

        bool markCancelled();
};