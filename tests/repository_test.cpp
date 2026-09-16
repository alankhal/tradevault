#include "tradevault/InMemoryTradeRepository.hpp"
#include "tradevault/Trade.hpp"

#include <gtest/gtest.h>

//See if proper Trade object is built as well as repository methods 
TEST(InMemoryTradeRepositoryTest, StoresAndRetrievesTrade)
{
    InMemoryTradeRepository repository;

    Trade trade(
        "AAPL",
        "RBC",
        TradeSide::Buy,
        200.00,
        100
    );

    repository.storeTrade(trade);

    auto result = repository.getTrade(trade.getTradeId());

    //ASSERT used as if it fails it will stop the test immediatley 
    ASSERT_TRUE(result.has_value());

    //EXPECT used in order to check the condition but continue running the rest of the test if it fails 
    EXPECT_EQ(result->getTradeId(), trade.getTradeId());
    EXPECT_EQ(result->getInstrument(), trade.getInstrument());
    EXPECT_EQ(result->getCounterparty(), trade.getCounterparty());
    EXPECT_EQ(result->getSide(), trade.getSide());
    EXPECT_DOUBLE_EQ(result->getPrice(), trade.getPrice());
    EXPECT_EQ(result->getQuantity(), trade.getQuantity());
}

//Purposeful Failed Test that is missing a trade 
TEST(InMemoryTradeRepositoryTest, ReturnsNulloptForMissingTrade)
{
    InMemoryTradeRepository repository;

    auto result = repository.getTrade(999999);

    EXPECT_FALSE(result.has_value());
}

//Tests an Empty List 
TEST(InMemoryTradeRepositoryTest, ListsZeroTradesWhenRepositoryIsEmpty)
{
    InMemoryTradeRepository repository;

    auto trades = repository.listTrades();

    EXPECT_TRUE(trades.empty());
}

//Several Trades being Tested 
TEST(InMemoryTradeRepositoryTest, ListsAllStoredTrades)
{
    InMemoryTradeRepository repository;

    Trade firstTrade(
        "AAPL",
        "RBC",
        TradeSide::Buy,
        200.00,
        100
    );

    Trade secondTrade(
        "MSFT",
        "TD",
        TradeSide::Sell,
        450.00,
        50
    );

    repository.storeTrade(firstTrade);
    repository.storeTrade(secondTrade);

    auto trades = repository.listTrades();

    ASSERT_EQ(trades.size(), 2u);

    EXPECT_EQ(trades[0].getTradeId(), firstTrade.getTradeId());
    EXPECT_EQ(trades[1].getTradeId(), secondTrade.getTradeId());
}


//updateTrade() test to see if it operates accordingly 
TEST(InMemoryTradeRepositoryTest, UpdatesExistingTrade)
{
    InMemoryTradeRepository repository;

    Trade trade(
        "AAPL",
        "RBC",
        TradeSide::Buy,
        200.00,
        100
    );

    repository.storeTrade(trade);

    Trade updatedTrade = trade;

    updatedTrade.markCancelled();

    repository.updateTrade(updatedTrade);

    auto result = repository.getTrade(trade.getTradeId());

    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->getStatus(), TradeStatus::Cancelled);
}


//Testing Binary Search Boundaries 
TEST(InMemoryTradeRepositoryTest, FindsTradesAtBinarySearchBoundaries)
{
    InMemoryTradeRepository repository;

    Trade first(
        "AAPL",
        "RBC",
        TradeSide::Buy,
        200.00,
        100
    );

    Trade middle(
        "MSFT",
        "TD",
        TradeSide::Buy,
        450.00,
        50
    );

    Trade last(
        "GOOG",
        "BMO",
        TradeSide::Sell,
        180.00,
        25
    );

    repository.storeTrade(first);
    repository.storeTrade(middle);
    repository.storeTrade(last);

    EXPECT_TRUE(repository.getTrade(first.getTradeId()).has_value());
    EXPECT_TRUE(repository.getTrade(middle.getTradeId()).has_value());
    EXPECT_TRUE(repository.getTrade(last.getTradeId()).has_value());
}