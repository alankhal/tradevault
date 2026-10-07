#pragma once

#include "tradevault/TradeRepository.hpp"
#include <optional>

class InMemoryTradeRepository : public TradeRepository {

private:
    std::vector<Trade> m_trades; 

    // The in-memory repository owns IDs when it is being used.
    unsigned m_nextId{1};

public:
    // Store
    Trade storeTrade(const Trade& trade) override;

    // Find
    std::optional<Trade> getTrade(unsigned tradeId) const override;

    // List
    std::vector<Trade> listTrades() const override;

    // Update
    void updateTrade(const Trade& trade) override;

};

