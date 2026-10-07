//Code is formatted and organized similar to that of the Inmemory implementation, what varies is 
#pragma once

#include "tradevault/TradeRepository.hpp"
#include <string>

class PostgresTradeRepository : public TradeRepository {
private:
    std::string m_connectionString;

public:
    explicit PostgresTradeRepository(std::string connectionString);  //Connection string will eventually contain information such as the host, port, name of database and the database password 
                                                                     // Explicit also written in this case in order to make the user type out the exact connection information rather than a random string 

    Trade storeTrade(const Trade& trade) override;
    std::optional<Trade> getTrade(unsigned tradeId) const override;
    std::vector<Trade> listTrades() const override;
    void updateTrade(const Trade& trade) override;
};