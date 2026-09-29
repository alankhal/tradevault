#include <gtest/gtest.h>
#include <drogon/drogon.h>

#include "api/TradeController.hpp"
#include "tradevault/InMemoryTradeRepository.hpp"
#include "tradevault/TradeService.hpp"


class TradeControllerTest : public ::testing::Test
{
protected:
    InMemoryTradeRepository repository;
    TradeService service{repository};
    TradeController controller{service};


    // Helper function used to create a trade through the controller
    drogon::HttpResponsePtr createTrade(
        const std::string& instrument = "AAPL",
        const std::string& counterparty = "Goldman Sachs",
        const std::string& side = "Buy",
        double price = 215.50,
        int quantity = 10)
    {
        Json::Value body;
        body["instrument"] = instrument;
        body["counterparty"] = counterparty;
        body["side"] = side;
        body["price"] = price;
        body["quantity"] = quantity;

        auto request = drogon::HttpRequest::newHttpRequest();
        request->setMethod(drogon::Post);
        request->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        request->setBody(body.toStyledString());

        drogon::HttpResponsePtr response;

        controller.createTrade(request, [&response](const drogon::HttpResponsePtr& resp)
        {
            response = resp;
        });

        return response;
    }


    // Helper function used to retrieve one trade through the controller
    drogon::HttpResponsePtr getTrade(unsigned id)
    {
        auto request = drogon::HttpRequest::newHttpRequest();

        drogon::HttpResponsePtr response;

        controller.getTrade(request, [&response](const drogon::HttpResponsePtr& resp)
        {
            response = resp;
        }, id);

        return response;
    }


    // Helper function used to retrieve all trades through the controller
    drogon::HttpResponsePtr listTrades()
    {
        auto request = drogon::HttpRequest::newHttpRequest();

        drogon::HttpResponsePtr response;

        controller.listTrades(request, [&response](const drogon::HttpResponsePtr& resp)
        {
            response = resp;
        });

        return response;
    }


    // Helper function used to cancel a trade through the controller
    drogon::HttpResponsePtr cancelTrade(unsigned id)
    {
        auto request = drogon::HttpRequest::newHttpRequest();

        drogon::HttpResponsePtr response;

        controller.cancelTrade(request, [&response](const drogon::HttpResponsePtr& resp)
        {
            response = resp;
        }, id);

        return response;
    }
};


// Tests that a valid POST request successfully creates a trade and returns HTTP 201 with the correct JSON data
TEST_F(TradeControllerTest, CreatesTrade)
{
    auto response = createTrade();

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k201Created);

    auto json = response->getJsonObject();

    ASSERT_NE(json, nullptr);

    EXPECT_EQ((*json)["instrument"].asString(), "AAPL");
    EXPECT_EQ((*json)["counterparty"].asString(), "Goldman Sachs");
    EXPECT_EQ((*json)["side"].asString(), "Buy");
    EXPECT_EQ((*json)["status"].asString(), "Booked");
    EXPECT_DOUBLE_EQ((*json)["price"].asDouble(), 215.50);
    EXPECT_EQ((*json)["quantity"].asInt(), 10);
}


// Tests that a trade with quantity 0 is rejected and returns HTTP 400 Bad Request
TEST_F(TradeControllerTest, RejectsInvalidQuantity)
{
    auto response = createTrade("AAPL", "Goldman Sachs", "Buy", 215.50, 0);

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k400BadRequest);

    auto json = response->getJsonObject();

    ASSERT_NE(json, nullptr);
    EXPECT_EQ((*json)["status"].asInt(), 400);
    EXPECT_EQ((*json)["title"].asString(), "Bad Request");
}


// Tests that an invalid side string is rejected and returns HTTP 400 Bad Request
TEST_F(TradeControllerTest, RejectsInvalidSide)
{
    auto response = createTrade("AAPL", "Goldman Sachs", "WrongSide", 215.50, 10);

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k400BadRequest);
}


// Tests that a successfully created trade can later be retrieved by its ID
TEST_F(TradeControllerTest, GetsExistingTrade)
{
    auto createResponse = createTrade();

    ASSERT_NE(createResponse, nullptr);

    auto createdJson = createResponse->getJsonObject();

    ASSERT_NE(createdJson, nullptr);

    unsigned id = (*createdJson)["id"].asUInt();

    auto response = getTrade(id);

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k200OK);

    auto json = response->getJsonObject();

    ASSERT_NE(json, nullptr);

    EXPECT_EQ((*json)["id"].asUInt(), id);
    EXPECT_EQ((*json)["instrument"].asString(), "AAPL");
}


// Tests that requesting a trade that does not exist returns HTTP 404 Not Found
TEST_F(TradeControllerTest, ReturnsNotFoundForMissingTrade)
{
    auto response = getTrade(999999);

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k404NotFound);

    auto json = response->getJsonObject();

    ASSERT_NE(json, nullptr);

    EXPECT_EQ((*json)["status"].asInt(), 404);
    EXPECT_EQ((*json)["title"].asString(), "Not Found");
}


// Tests that GET /trades returns every trade currently stored in the repository
TEST_F(TradeControllerTest, ListsTrades)
{
    createTrade("AAPL", "Goldman Sachs", "Buy", 215.50, 10);
    createTrade("MSFT", "JP Morgan", "Sell", 420.00, 5);

    auto response = listTrades();

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k200OK);

    auto json = response->getJsonObject();

    ASSERT_NE(json, nullptr);
    ASSERT_TRUE(json->isArray());
    EXPECT_EQ(json->size(), 2);
}


// Tests that listing trades from an empty repository returns HTTP 200 and an empty JSON array
TEST_F(TradeControllerTest, ListsEmptyRepository)
{
    auto response = listTrades();

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k200OK);

    auto json = response->getJsonObject();

    ASSERT_NE(json, nullptr);
    ASSERT_TRUE(json->isArray());
    EXPECT_EQ(json->size(), 0);
}


// Tests that an existing trade can be cancelled and its returned status becomes Cancelled
TEST_F(TradeControllerTest, CancelsTrade)
{
    auto createResponse = createTrade();

    auto createdJson = createResponse->getJsonObject();

    ASSERT_NE(createdJson, nullptr);

    unsigned id = (*createdJson)["id"].asUInt();

    auto response = cancelTrade(id);

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k200OK);

    auto json = response->getJsonObject();

    ASSERT_NE(json, nullptr);

    EXPECT_EQ((*json)["id"].asUInt(), id);
    EXPECT_EQ((*json)["status"].asString(), "Cancelled");
}


// Tests that attempting to cancel the same trade twice returns HTTP 409 Conflict
TEST_F(TradeControllerTest, RejectsSecondCancellation)
{
    auto createResponse = createTrade();

    auto createdJson = createResponse->getJsonObject();

    ASSERT_NE(createdJson, nullptr);

    unsigned id = (*createdJson)["id"].asUInt();

    auto firstResponse = cancelTrade(id);

    ASSERT_NE(firstResponse, nullptr);
    ASSERT_EQ(firstResponse->getStatusCode(), drogon::k200OK);

    auto secondResponse = cancelTrade(id);

    ASSERT_NE(secondResponse, nullptr);
    EXPECT_EQ(secondResponse->getStatusCode(), drogon::k409Conflict);

    auto json = secondResponse->getJsonObject();

    ASSERT_NE(json, nullptr);

    EXPECT_EQ((*json)["status"].asInt(), 409);
    EXPECT_EQ((*json)["title"].asString(), "Conflict");
}


// Tests that attempting to cancel a trade that does not exist returns HTTP 404 Not Found
TEST_F(TradeControllerTest, ReturnsNotFoundWhenCancellingMissingTrade)
{
    auto response = cancelTrade(999999);

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k404NotFound);
}

// Tests that malformed JSON cannot be parsed and returns HTTP 400 Bad Request
TEST_F(TradeControllerTest, RejectsMalformedJson)
{
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Post);
    request->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    request->setBody("{ invalid json }");

    drogon::HttpResponsePtr response;

    controller.createTrade(request, [&response](const drogon::HttpResponsePtr& resp)
    {
        response = resp;
    });

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k400BadRequest);

    auto json = response->getJsonObject();

    ASSERT_NE(json, nullptr);
    EXPECT_EQ((*json)["status"].asInt(), 400);
    EXPECT_EQ((*json)["title"].asString(), "Bad Request");
}


// Tests that a request missing the required instrument field is rejected with HTTP 400 Bad Request
TEST_F(TradeControllerTest, RejectsMissingInstrument)
{
    Json::Value body;
    body["counterparty"] = "Goldman Sachs";
    body["side"] = "Buy";
    body["price"] = 215.50;
    body["quantity"] = 10;

    auto request = drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Post);
    request->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    request->setBody(body.toStyledString());

    drogon::HttpResponsePtr response;

    controller.createTrade(request, [&response](const drogon::HttpResponsePtr& resp)
    {
        response = resp;
    });

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k400BadRequest);

    auto json = response->getJsonObject();

    ASSERT_NE(json, nullptr);
    EXPECT_EQ((*json)["status"].asInt(), 400);
    EXPECT_EQ((*json)["title"].asString(), "Bad Request");
}