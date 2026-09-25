#include "tradevault/InMemoryTradeRepository.hpp"


// Store
void InMemoryTradeRepository::storeTrade(const Trade& trade) { 
	m_trades.push_back(trade);  
}

// Find 
//Used Binary search in order to speed up the search process and used 
std::optional<Trade> InMemoryTradeRepository::getTrade(unsigned tradeId) const { 
	
	//Error handling to make sure there is no compile error 
	if (m_trades.empty()) {
		return std::nullopt; 
	}
	
	int left = 0; 
	int right = static_cast<int>(m_trades.size()) - 1;

	while (left <= right) {
		
		int mid = left + (right - left) / 2;

		if (m_trades[mid].getTradeId() == tradeId) {
                return m_trades[mid];        
		} 
		if (m_trades[mid].getTradeId() < tradeId) {
			left = mid + 1; 
		} else {
			right = mid - 1;         
		}
	
	} 
	//Since we used optional for our trade, it must return from optional library, we will later used TradeError to account for nullopt if found 
	return std::nullopt;
}

// List out items within the vector 
std::vector<Trade> InMemoryTradeRepository::listTrades() const{
	return m_trades; 
}

//Update 
//We use another binary search in order to find the tradeId as the 
void InMemoryTradeRepository::updateTrade(const Trade& trade) { 
	if (m_trades.empty()) {
		return;     
	}

	unsigned tradeId = trade.getTradeId(); 

	int left = 0; 
	int right = static_cast<int>(m_trades.size()) - 1; //Static is needed as m_trades returns an assigned number, if you assign one to the other it silently squeezes it down, 
													   //if the size is ever past 2.1 billion you would get a wrong/ negative number, hence static make it an int preventing this potential bug

	while (left <= right) {
            int mid = left + (right - left) / 2;     

			unsigned currentId = m_trades[mid].getTradeId();

			if (currentId == tradeId) {
                m_trades[mid] = trade;


				return; //Base return to end the function early without a return value 
			}

			if (currentId < tradeId) {
				left = mid + 1; 
			} else {
				right = mid - 1; 
			}
	}

}