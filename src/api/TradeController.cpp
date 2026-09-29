//Big distinction on this portion of the project is that Result.hpp is how TradeService communicates success or failure back to TradeController

#include "api/TradeController.hpp"
#include <drogon/drogon.h>

/*---------------CORE API OPERATIONS----------------------------------------------------------------------------------------------*/
//Use initializer list to set up constructor
TradeController::TradeController(TradeService& service)
    : m_service(service){}





//GetTrade for JSON 
void TradeController::getTrade(
    const drogon::HttpRequestPtr&,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    unsigned id) const
{

    //Ask the service layer for the trade
    auto result = m_service.getTrade(id);

    //Failure Path
    if (!result.hasValue())
    {

        callback(HttpErrorMapper::toResponse(result.error()));
        return;
    }

    // SUCCESS PATH
    const Trade& trade = result.value(); //Retrieves the successful Trade that is place inside Result<Trade, TradeError>, and refers to the one already inside the result 

    Json::Value body = TradeJson::toJson(trade);

    auto response = drogon::HttpResponse::newHttpJsonResponse(body);
    response->setStatusCode(drogon::k200OK);

    callback(response);
}


//List out groups of trades for JSON 
void TradeController::listTrades(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
    // 1. Ask TradeService for all trades
    auto trades = m_service.listTrades();

    // 2. Create a JSON array
    Json::Value body(Json::arrayValue);

    // 3. Loop through every Trade
    for (const auto& trade : trades)
    {
        //Since its looping through the trades from trade service, it takes that and appends it to Json object which is then put into a JSON array, essentailly acting as a funnel 
        body.append(TradeJson::toJson(trade));
    }

    // 4. Create HTTP JSON response
    auto response = drogon::HttpResponse::newHttpJsonResponse(body); 
     
    // 5. Set status to 200 OK
    response->setStatusCode(drogon::k200OK);

    // 6. callback(response)
    callback(response);
}


//POST FUNCTIONS
//Creating an incoming Trade Obj into JSON 
void TradeController::createTrade(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const 
{
    // 1. Get JSON body from request
    auto jsonBody = req->getJsonObject();  //Drogons incoming HTTP request, pretty much says "Take the body of this request and parse it as JSON if possible"

    // 2. If body is not valid JSON, return HTTP 400
    if (!jsonBody)
    {
        Json::Value errorBody;
        errorBody["type"] = "about:blank";
        errorBody["title"] = "Bad Request";
        errorBody["status"] = 400;
        errorBody["detail"] = "Invalid JSON body";

        auto response = drogon::HttpResponse::newHttpJsonResponse(errorBody);
        response->setStatusCode(drogon::k400BadRequest);

        callback(response);
        return;
    }


    // 3. Extract fields from JSON
    std::string instrument = (*jsonBody)["instrument"].asString();  //Since getJsonObject returns a smart pointer its important to keep a pointer by *jsonBody 
    std::string counterparty = (*jsonBody)["counterparty"].asString();
    std::string sideText = (*jsonBody)["side"].asString();
    double price = (*jsonBody)["price"].asDouble();
    int quantity = (*jsonBody)["quantity"].asInt();

    // 4. Convert sideText into TradeSide
    TradeSide side;

    if (sideText == "Buy")
    {
        side = TradeSide::Buy;
    }
    else if (sideText == "Sell")
    {
        side = TradeSide::Sell;
    }
    else
    {
        Json::Value errorBody;
        errorBody["type"] = "about:blank";
        errorBody["title"] = "Bad Request";
        errorBody["status"] = 400;
        errorBody["detail"] = "Side must be either Buy or Sell";

        auto response = drogon::HttpResponse::newHttpJsonResponse(errorBody);
        response->setStatusCode(drogon::k400BadRequest);

        callback(response);
        return;
    }
    // 5. Call m_service.createTrade()
    auto result = m_service.createTrade(instrument, counterparty, side, price, quantity);

    // 6. If result failed:create JSON error, status 400, callback, return
    
    if (!result.hasValue())
    {   
        callback(HttpErrorMapper::toResponse(result.error()));
        return;
    }

    // 7. Get successful Trade
    const Trade& trade = result.value();

    // 8. Convert Trade to JSON
    Json::Value body = TradeJson::toJson(trade);


    // 9. Create response
    auto response = drogon::HttpResponse::newHttpJsonResponse(body); 
    
    // 10. Use HTTP 201 Created
    response->setStatusCode(drogon::k201Created);

    // 11. callback(response)
    callback(response); 
}


//Implemented JSON object for cancelling trades 
void TradeController::cancelTrade(const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&& callback, unsigned id) const
{
    // 1. Ask TradeService to cancel the trade
    auto result = m_service.cancelTrade(id);

    // 2. Handle cancellation failure
    if (!result.hasValue())
    {
        callback(HttpErrorMapper::toResponse(result.error()));
        return;
    }

    // 3. Retrieve successfully cancelled Trade
    const Trade& trade = result.value();

    // 4. Convert Trade to JSON
    Json::Value body = TradeJson::toJson(trade);

    // 5. Create response
    auto response = drogon::HttpResponse::newHttpJsonResponse(body);

    // 6. Successful cancellation
    response->setStatusCode(drogon::k200OK);

    // 7. Send response
    callback(response);
}
/*---------------CORE API OPERATIONS----------------------------------------------------------------------------------------------*/