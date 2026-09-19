// unit tests for the trade service: creating, fetching, listing and cancelling trades.
// every test builds its own fresh in-memory repository, so no test shares state with another.

#include <gtest/gtest.h>

#include "tradevault/InMemoryTradeRepository.hpp"
#include "tradevault/TradeService.hpp"


// ---------------------------------------------------------
// create trade
// ---------------------------------------------------------

// happy path: a fully valid trade is created, comes back with the right
// fields, and is actually saved to the repository.
TEST(TradeServiceTest, CreatesValidTrade)
{
    // setup: empty repository and a service that writes to it
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // action: book a valid buy of 100 aapl at 190.50 with goldman sachs
    auto result = service.createTrade(
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        190.50,
        100
    );

    // check: creation succeeded. assert (not expect) stops the test here on
    // failure, so value() is never called on an error result.
    ASSERT_TRUE(result.hasValue());

    const Trade& trade = result.value();

    // check: every field on the returned trade matches what was passed in
    EXPECT_EQ(trade.getInstrument(), "AAPL");
    EXPECT_EQ(trade.getCounterparty(), "Goldman Sachs");
    EXPECT_EQ(trade.getSide(), TradeSide::Buy);
    EXPECT_DOUBLE_EQ(trade.getPrice(), 190.50);
    EXPECT_EQ(trade.getQuantity(), 100);

    // check: a newly created trade starts in the booked state
    EXPECT_EQ(trade.getStatus(), TradeStatus::Booked);

    // check: createTrade() actually stored the trade, not just returned it.
    // this goes straight to the repository, which returns a std::optional
    // (hence has_value() here instead of hasValue()).
    auto storedTrade = repository.getTrade(trade.getTradeId());

    ASSERT_TRUE(storedTrade.has_value());
    EXPECT_EQ(storedTrade->getTradeId(), trade.getTradeId());
}


// validation: a negative quantity is rejected with InvalidQuantity.
TEST(TradeServiceTest, RejectsInvalidQuantity)
{
    // setup: empty repository and service
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // action: same trade as the happy path, but with a quantity of -100
    auto result = service.createTrade(
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        190.50,
        -100
    );

    // check: creation failed with the quantity error specifically
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error(), TradeError::InvalidQuantity);

    // check: invalid trades never reach storage
    EXPECT_TRUE(repository.listTrades().empty());
}


// validation: a negative price is rejected with InvalidPrice.
TEST(TradeServiceTest, RejectsInvalidPrice)
{
    // setup: empty repository and service
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // action: same trade as the happy path, but with a price of -190.50
    auto result = service.createTrade(
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        -190.50,
        100
    );

    // check: creation failed with the price error specifically
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error(), TradeError::InvalidPrice);

    // check: nothing was written to the repository
    EXPECT_TRUE(repository.listTrades().empty());
}


// validation: an empty instrument name is rejected with InvalidInstrument.
TEST(TradeServiceTest, RejectsEmptyInstrument)
{
    // setup: empty repository and service
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // action: same trade as the happy path, but with a blank instrument
    auto result = service.createTrade(
        "",
        "Goldman Sachs",
        TradeSide::Buy,
        190.50,
        100
    );

    // check: creation failed with the instrument error specifically
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error(), TradeError::InvalidInstrument);

    // check: nothing was written to the repository
    EXPECT_TRUE(repository.listTrades().empty());
}


// validation: an empty counterparty is rejected with MissingCounterparty.
TEST(TradeServiceTest, RejectsMissingCounterparty)
{
    // setup: empty repository and service
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // action: same trade as the happy path, but with a blank counterparty
    auto result = service.createTrade(
        "AAPL",
        "",
        TradeSide::Buy,
        190.50,
        100
    );

    // check: creation failed with the counterparty error specifically
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error(), TradeError::MissingCounterparty);

    // check: nothing was written to the repository
    EXPECT_TRUE(repository.listTrades().empty());
}


// boundary: a price of exactly zero is rejected with InvalidPrice.
// this catches a validation check written as "< 0" instead of "<= 0".
TEST(TradeServiceTest, RejectsZeroPrice)
{
    // setup: empty repository and service
    InMemoryTradeRepository repository;
    TradeService service(repository);
 
    // action: same trade as the happy path, but with a price of 0.00
    auto result = service.createTrade(
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        0.00,
        100
    );
 
    // check: creation failed with the price error specifically
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error(), TradeError::InvalidPrice);
 
    // check: nothing was written to the repository
    EXPECT_TRUE(repository.listTrades().empty());
}


TEST(TradeServiceTest, RejectsZeroQuantity)
{
    InMemoryTradeRepository repository;
    TradeService service(repository);

    auto result = service.createTrade(
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        190.50,
        0
    );

    ASSERT_FALSE(result.hasValue());

    EXPECT_EQ(
        result.error(),
        TradeError::InvalidQuantity
    );

    EXPECT_TRUE(repository.listTrades().empty());
}



// ---------------------------------------------------------
// get trade
// ---------------------------------------------------------

// lookup: a trade that was created can be fetched back by its id,
// with all of its fields intact.
TEST(TradeServiceTest, GetsExistingTrade)
{
    // setup: empty repository and service
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // setup: create a trade to look up (a sell of 50 msft at 420.00)
    auto createResult = service.createTrade(
        "MSFT",
        "Morgan Stanley",
        TradeSide::Sell,
        420.00,
        50
    );

    ASSERT_TRUE(createResult.hasValue());

    // grab the id the service assigned to the new trade
    unsigned tradeId = createResult.value().getTradeId();

    // action: fetch the trade back through the service
    auto result = service.getTrade(tradeId);

    // check: the lookup succeeded
    ASSERT_TRUE(result.hasValue());

    // check: the fetched trade matches the one that was created
    EXPECT_EQ(result.value().getTradeId(), tradeId);
    EXPECT_EQ(result.value().getInstrument(), "MSFT");
    EXPECT_EQ(result.value().getCounterparty(), "Morgan Stanley");
    EXPECT_EQ(result.value().getSide(), TradeSide::Sell);
    EXPECT_DOUBLE_EQ(result.value().getPrice(), 420.00);
    EXPECT_EQ(result.value().getQuantity(), 50);
}


// lookup: asking for an id that was never created returns TradeNotFound.
TEST(TradeServiceTest, ReturnsNotFoundForMissingTrade)
{
    // setup: empty repository, so no id can exist yet
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // action: look up an arbitrary id that was never created
    auto result = service.getTrade(999999);

    // check: the lookup failed with the not-found error
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error(), TradeError::TradeNotFound);
}


// ---------------------------------------------------------
// list trades
// ---------------------------------------------------------

// listing: an empty repository gives back an empty list rather than an error.
TEST(TradeServiceTest, ListsEmptyRepository)
{
    // setup: empty repository and service
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // action: list everything
    auto trades = service.listTrades();

    // check: nothing comes back
    EXPECT_TRUE(trades.empty());
}


// listing: every created trade shows up, in the order it was created.
TEST(TradeServiceTest, ListsStoredTrades)
{
    // setup: empty repository and service
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // setup: create a first trade (aapl buy)
    auto firstResult = service.createTrade(
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        190.50,
        100
    );

    // setup: create a second, different trade (msft sell)
    auto secondResult = service.createTrade(
        "MSFT",
        "Morgan Stanley",
        TradeSide::Sell,
        420.00,
        50
    );

    // check: both creations succeeded before relying on their values
    ASSERT_TRUE(firstResult.hasValue());
    ASSERT_TRUE(secondResult.hasValue());

    // action: list everything in the repository
    auto trades = service.listTrades();

    // check: exactly two trades came back
    ASSERT_EQ(trades.size(), 2u);

    // check: the first listed trade is the first one created
    EXPECT_EQ(
        trades[0].getTradeId(),
        firstResult.value().getTradeId()
    );

    // check: the second listed trade is the second one created
    EXPECT_EQ(
        trades[1].getTradeId(),
        secondResult.value().getTradeId()
    );
}


// ---------------------------------------------------------
// cancel trade
// ---------------------------------------------------------

// cancel: a booked trade can be cancelled, and the new status is saved.
TEST(TradeServiceTest, CancelsExistingTrade)
{
    // setup: empty repository and service
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // setup: create a trade to cancel
    auto createResult = service.createTrade(
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        190.50,
        100
    );

    ASSERT_TRUE(createResult.hasValue());

    // grab the id the service assigned to the new trade
    unsigned tradeId = createResult.value().getTradeId();

    // action: cancel the trade
    auto cancelResult = service.cancelTrade(tradeId);

    // check: the cancel succeeded
    ASSERT_TRUE(cancelResult.hasValue());

    // check: the returned trade shows the cancelled status
    EXPECT_EQ(
        cancelResult.value().getStatus(),
        TradeStatus::Cancelled
    );

    // check: fetch it again to make sure the cancelled state was
    // persisted by updateTrade(), not just set on the returned copy
    auto storedResult = service.getTrade(tradeId);

    ASSERT_TRUE(storedResult.hasValue());

    EXPECT_EQ(
        storedResult.value().getStatus(),
        TradeStatus::Cancelled
    );
}


// cancel: cancelling an id that doesn't exist returns TradeNotFound.
TEST(TradeServiceTest, ReturnsNotFoundWhenCancellingMissingTrade)
{
    // setup: empty repository, so no id can exist yet
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // action: try to cancel an arbitrary id that was never created
    auto result = service.cancelTrade(999999);

    // check: the cancel failed with the not-found error
    ASSERT_FALSE(result.hasValue());
    EXPECT_EQ(result.error(), TradeError::TradeNotFound);
}


// cancel: a trade can only be cancelled once; a second attempt
// returns TradeAlreadyCancelled.
TEST(TradeServiceTest, RejectsAlreadyCancelledTrade)
{
    // setup: empty repository and service
    InMemoryTradeRepository repository;
    TradeService service(repository);

    // setup: create a trade to cancel
    auto createResult = service.createTrade(
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        190.50,
        100
    );

    ASSERT_TRUE(createResult.hasValue());

    // grab the id the service assigned to the new trade
    unsigned tradeId = createResult.value().getTradeId();

    // setup: first cancellation, which should succeed
    auto firstCancellation = service.cancelTrade(tradeId);

    ASSERT_TRUE(firstCancellation.hasValue());

    // action: try to cancel the same trade a second time
    auto secondCancellation = service.cancelTrade(tradeId);

    // check: the second attempt failed with the already-cancelled error
    ASSERT_FALSE(secondCancellation.hasValue());

    EXPECT_EQ(
        secondCancellation.error(),
        TradeError::TradeAlreadyCancelled
    );
}