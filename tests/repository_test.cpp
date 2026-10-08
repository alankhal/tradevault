#include "tradevault/InMemoryTradeRepository.hpp"
#include "tradevault/Trade.hpp"

#include <gtest/gtest.h>

// Tests that a proper Trade object is stored and can be retrieved from the repository
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

    // storeTrade assigns the permanent repository ID and returns the stored Trade
    Trade storedTrade = repository.storeTrade(trade);

    // Use the stored Trade's ID because the original Trade still has temporary ID 0
    auto result = repository.getTrade(storedTrade.getTradeId());

    // ASSERT is used because if this fails, the test stops immediately
    // We cannot safely check the Trade fields below if no Trade was returned
    ASSERT_TRUE(result.has_value());

    // EXPECT checks each condition but continues running the rest of the test if one fails
    EXPECT_EQ(result->getTradeId(), storedTrade.getTradeId());
    EXPECT_EQ(result->getInstrument(), storedTrade.getInstrument());
    EXPECT_EQ(result->getCounterparty(), storedTrade.getCounterparty());
    EXPECT_EQ(result->getSide(), storedTrade.getSide());
    EXPECT_DOUBLE_EQ(result->getPrice(), storedTrade.getPrice());
    EXPECT_EQ(result->getQuantity(), storedTrade.getQuantity());
}


// Purposeful failed lookup test using an ID that does not exist
TEST(InMemoryTradeRepositoryTest, ReturnsNulloptForMissingTrade)
{
    InMemoryTradeRepository repository;

    auto result = repository.getTrade(999999);

    // A missing Trade should return std::nullopt
    EXPECT_FALSE(result.has_value());
}


// Tests that an empty repository returns an empty list
TEST(InMemoryTradeRepositoryTest, ListsZeroTradesWhenRepositoryIsEmpty)
{
    InMemoryTradeRepository repository;

    auto trades = repository.listTrades();

    EXPECT_TRUE(trades.empty());
}


// Tests that several stored Trades can all be returned from the repository
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

    // Capture the stored versions because the repository assigns their permanent IDs
    Trade storedFirstTrade = repository.storeTrade(firstTrade);
    Trade storedSecondTrade = repository.storeTrade(secondTrade);

    auto trades = repository.listTrades();

    ASSERT_EQ(trades.size(), 2u);

    // Compare against the stored Trades rather than the original temporary Trades
    EXPECT_EQ(trades[0].getTradeId(), storedFirstTrade.getTradeId());
    EXPECT_EQ(trades[1].getTradeId(), storedSecondTrade.getTradeId());
}


// Tests updateTrade() to make sure an existing stored Trade is updated correctly
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

    // Capture the stored Trade because it now contains the repository-assigned ID
    Trade storedTrade = repository.storeTrade(trade);

    // Create a copy of the persisted Trade so the correct ID is preserved
    Trade updatedTrade = storedTrade;

    updatedTrade.markCancelled();

    // Update the existing repository entry using its permanent ID
    repository.updateTrade(updatedTrade);

    auto result = repository.getTrade(storedTrade.getTradeId());

    ASSERT_TRUE(result.has_value());

    EXPECT_EQ(result->getStatus(), TradeStatus::Cancelled);
}


// Tests the first, middle, and last positions used by the repository's binary search
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

    // Capture each stored Trade so we have the IDs assigned by the repository
    Trade storedFirst = repository.storeTrade(first);
    Trade storedMiddle = repository.storeTrade(middle);
    Trade storedLast = repository.storeTrade(last);

    // Confirm that binary search can find Trades at all important boundaries
    EXPECT_TRUE(repository.getTrade(storedFirst.getTradeId()).has_value());
    EXPECT_TRUE(repository.getTrade(storedMiddle.getTradeId()).has_value());
    EXPECT_TRUE(repository.getTrade(storedLast.getTradeId()).has_value());
}