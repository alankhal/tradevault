#include <gtest/gtest.h>

#include "tradevault/PostgresTradeRepository.hpp"
#include "tradevault/Trade.hpp"

#include <pqxx/pqxx>


//Testing if store trade works accordingly 
TEST(PostgresTradeRepositoryTest, StoresTrade)
{
    const std::string connectionString =
        "dbname=tradevault user=postgres password=kX7mP2wN5v. host=localhost port=5432";

    PostgresTradeRepository repository{connectionString};

    Trade trade{
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        225.50,
        100
    };

    // Remove an existing row from an earlier test run.
    {
        pqxx::connection connection{connectionString};
        pqxx::work transaction{connection};

        transaction.exec(
            "DELETE FROM trades WHERE trade_id = $1",
            pqxx::params{trade.getTradeId()}
        );

        transaction.commit();
    }

    // Store the trade in PostgreSQL.
    repository.storeTrade(trade);

    // Query PostgreSQL directly to verify the insertion.
    pqxx::connection connection{connectionString};
    pqxx::work transaction{connection};

    const auto result = transaction.exec(
        "SELECT trade_id, instrument, counterparty, side, status, price, quantity "
        "FROM trades WHERE trade_id = $1",
        pqxx::params{trade.getTradeId()}
    );

    ASSERT_EQ(result.size(), 1);

    const auto row = result[0];

    EXPECT_EQ(row["trade_id"].as<unsigned>(), trade.getTradeId());
    EXPECT_EQ(row["instrument"].as<std::string>(), trade.getInstrument());
    EXPECT_EQ(row["counterparty"].as<std::string>(), trade.getCounterparty());
    EXPECT_EQ(row["side"].as<std::string>(), "Buy");
    EXPECT_EQ(row["status"].as<std::string>(), "Booked");
    EXPECT_EQ(row["price"].as<double>(), trade.getPrice());
    EXPECT_EQ(row["quantity"].as<int>(), trade.getQuantity());

    // Clean up the inserted trade.
    transaction.exec(
        "DELETE FROM trades WHERE trade_id = $1",
        pqxx::params{trade.getTradeId()}
    );

    transaction.commit();
}


TEST(PostgresTradeRepositoryTest, RetrievesStoredTrade)
{
    const std::string connectionString =
        "dbname=tradevault user=postgres password=kX7mP2wN5v. host=localhost port=5432";

    PostgresTradeRepository repository{connectionString};

    Trade trade{
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        225.50,
        100
    };

    // Remove a row with the same ID in case it exists from an earlier test run.
    {
        pqxx::connection connection{connectionString};
        pqxx::work transaction{connection};

        transaction.exec(
            "DELETE FROM trades WHERE trade_id = $1",
            pqxx::params{trade.getTradeId()}
        );

        transaction.commit();
    }

    // Store the Trade using the PostgreSQL repository.
    repository.storeTrade(trade);

    // Retrieve the same Trade using getTrade().
    const auto retrievedTrade = repository.getTrade(trade.getTradeId());

    // Make sure getTrade() actually returned a Trade.
    ASSERT_TRUE(retrievedTrade.has_value());

    // Verify that the reconstructed Trade contains the original stored values.
    EXPECT_EQ(retrievedTrade->getTradeId(), trade.getTradeId());
    EXPECT_EQ(retrievedTrade->getInstrument(), trade.getInstrument());
    EXPECT_EQ(retrievedTrade->getCounterparty(), trade.getCounterparty());
    EXPECT_EQ(retrievedTrade->getSide(), trade.getSide());
    EXPECT_EQ(retrievedTrade->getStatus(), trade.getStatus());
    EXPECT_DOUBLE_EQ(retrievedTrade->getPrice(), trade.getPrice());
    EXPECT_EQ(retrievedTrade->getQuantity(), trade.getQuantity());

    // PostgreSQL stores our timestamp at microsecond precision,
    // so compare both timestamps after flooring them to microseconds.
    EXPECT_EQ(
        std::chrono::floor<std::chrono::microseconds>(retrievedTrade->getTimestamp()),
        std::chrono::floor<std::chrono::microseconds>(trade.getTimestamp())
    );

    // Clean up the row after the test.
    pqxx::connection connection{connectionString};
    pqxx::work transaction{connection};

    transaction.exec(
        "DELETE FROM trades WHERE trade_id = $1",
        pqxx::params{trade.getTradeId()}
    );

    transaction.commit();
}

TEST(PostgresTradeRepositoryTest, ListsStoredTrades)
{
    const std::string connectionString = "dbname=tradevault user=postgres password=kX7mP2wN5v. host=localhost port=5432";

    PostgresTradeRepository repository{connectionString};

    Trade trade1{
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        225.50,
        100
    };

    Trade trade2{
        "MSFT",
        "Morgan Stanley",
        TradeSide::Sell,
        410.25,
        50
    };

    // Remove these IDs if they remain from an earlier test run.
    {
        pqxx::connection connection{connectionString};
        pqxx::work transaction{connection};

        transaction.exec(
            "DELETE FROM trades WHERE trade_id = $1 OR trade_id = $2",
            pqxx::params{
                trade1.getTradeId(),
                trade2.getTradeId()
            }
        );

        transaction.commit();
    }

    // Store both trades.
    repository.storeTrade(trade1);
    repository.storeTrade(trade2);

    // Retrieve every trade currently stored in PostgreSQL.
    const auto trades = repository.listTrades();

    bool foundTrade1 = false;
    bool foundTrade2 = false;

    // Check that both of the trades we inserted appear in the returned vector.
    for (const auto& trade : trades)
    {
        if (trade.getTradeId() == trade1.getTradeId())
        {
            foundTrade1 = true;
        }

        if (trade.getTradeId() == trade2.getTradeId())
        {
            foundTrade2 = true;
        }
    }

    EXPECT_TRUE(foundTrade1);
    EXPECT_TRUE(foundTrade2);

    // Clean up the inserted rows.
    pqxx::connection connection{connectionString};
    pqxx::work transaction{connection};

    transaction.exec(
        "DELETE FROM trades WHERE trade_id = $1 OR trade_id = $2",
        pqxx::params{
            trade1.getTradeId(),
            trade2.getTradeId()
        }
    );

    transaction.commit();
}

TEST(PostgresTradeRepositoryTest, UpdatesStoredTrade)
{
    const std::string connectionString = "dbname=tradevault user=postgres password=kX7mP2wN5v. host=localhost port=5432";

    PostgresTradeRepository repository{connectionString};

    Trade trade{
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        225.50,
        100
    };

    // Remove the same ID if it was left behind by an earlier test run.
    {
        pqxx::connection connection{connectionString};
        pqxx::work transaction{connection};

        transaction.exec(
            "DELETE FROM trades WHERE trade_id = $1",
            pqxx::params{trade.getTradeId()}
        );

        transaction.commit();
    }

    // Store the original Booked trade.
    repository.storeTrade(trade);

    EXPECT_EQ(trade.getStatus(), TradeStatus::Booked);

    // Change the Trade's status in C++.
    ASSERT_TRUE(trade.markCancelled());

    EXPECT_EQ(trade.getStatus(), TradeStatus::Cancelled);

    // Persist the changed Trade back into PostgreSQL.
    repository.updateTrade(trade);

    // Read the Trade back from PostgreSQL.
    const auto updatedTrade = repository.getTrade(trade.getTradeId());

    ASSERT_TRUE(updatedTrade.has_value());

    // Confirm that PostgreSQL now contains the updated status.
    EXPECT_EQ(updatedTrade->getStatus(), TradeStatus::Cancelled);

    // Confirm that the Trade ID was not changed during the update.
    EXPECT_EQ(updatedTrade->getTradeId(), trade.getTradeId());

    // Clean up the row after the test.
    pqxx::connection connection{connectionString};
    pqxx::work transaction{connection};

    transaction.exec(
        "DELETE FROM trades WHERE trade_id = $1",
        pqxx::params{trade.getTradeId()}
    );

    transaction.commit();
}

TEST(PostgresTradeRepositoryTest, ThrowsWhenUpdatingMissingTrade)
{
    const std::string connectionString = "dbname=tradevault user=postgres password=kX7mP2wN5v. host=localhost port=5432";

    PostgresTradeRepository repository{connectionString};

    Trade trade{
        "AAPL",
        "Goldman Sachs",
        TradeSide::Buy,
        225.50,
        100
    };

    // Make sure this Trade ID does not already exist in PostgreSQL.
    {
        pqxx::connection connection{connectionString};
        pqxx::work transaction{connection};

        transaction.exec(
            "DELETE FROM trades WHERE trade_id = $1",
            pqxx::params{trade.getTradeId()}
        );

        transaction.commit();
    }

    // updateTrade() should fail because no PostgreSQL row has this ID.
    EXPECT_THROW(
        repository.updateTrade(trade),
        std::runtime_error
    );
}