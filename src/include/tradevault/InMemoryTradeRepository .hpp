#include "tradevault/TradeRepository.hpp"

class InMemoryTradeRepository : public TradeRepository {

private:
    std::vector<Trade> m_trades; 

public:
    // Store
    void storeTrade(const Trade& other) override;

    // Find
    std::optional<Trade> getTrade(unsigned tradeId) const override;

    // List
    std::vector<Trade> listTrades() const override;

    // Update
    void updateTrade(const Trade& trade) override;

};

