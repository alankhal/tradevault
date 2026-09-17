//This file tests for edge cases and that the Validation code/namespace located in TradeError.hpp is working correctly for each scenario presented
#include "tradevault/TradeError.hpp"
#include <gtest/gtest.h>
//Used in order to be able to write infinite to prove it will reject it 
#include <limits>


TEST(TradeValidatorTest, AcceptsPositiveQuantity)
{
    EXPECT_TRUE(TradeValidator::isValidQuantity(100));
}


TEST(TradeValidatorTest, RejectsZeroQuantity)
{
    EXPECT_FALSE(TradeValidator::isValidQuantity(0));
}


TEST(TradeValidatorTest, RejectsNegativeQuantity)
{
    EXPECT_FALSE(TradeValidator::isValidQuantity(-100));
}


TEST(TradeValidatorTest, AcceptsPositivePrice)
{
    EXPECT_TRUE(TradeValidator::isValidPrice(200.50));
}


TEST(TradeValidatorTest, RejectsZeroPrice)
{
    EXPECT_FALSE(TradeValidator::isValidPrice(0.0));
}


TEST(TradeValidatorTest, RejectsNegativePrice)
{
    EXPECT_FALSE(TradeValidator::isValidPrice(-200.50));
}


TEST(TradeValidatorTest, RejectsInfinitePrice)
{
    double infinity = std::numeric_limits<double>::infinity();

    EXPECT_FALSE(TradeValidator::isValidPrice(infinity));
}


TEST(TradeValidatorTest, RejectsNaNPrice)
{
    double nan = std::numeric_limits<double>::quiet_NaN();

    EXPECT_FALSE(TradeValidator::isValidPrice(nan));
}


TEST(TradeValidatorTest, AcceptsNonEmptyInstrument)
{
    EXPECT_TRUE(TradeValidator::isValidInstrument("AAPL"));
}


TEST(TradeValidatorTest, RejectsEmptyInstrument)
{
    EXPECT_FALSE(TradeValidator::isValidInstrument(""));
}


TEST(TradeValidatorTest, AcceptsNonEmptyCounterparty)
{
    EXPECT_TRUE(TradeValidator::isValidCounterparty("RBC"));
}


TEST(TradeValidatorTest, RejectsEmptyCounterparty)
{
    EXPECT_FALSE(TradeValidator::isValidCounterparty(""));
}