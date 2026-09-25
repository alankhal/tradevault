//Big distinction on this portion of the project is that Result.hpp is how TradeService communicates success or failure back to TradeController

#include "api/TradeController.hpp"

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

        Json::Value errorBody; //Json has its own name space, similar to something such as std::optional, Json::Value is apart of it

        errorBody["error"] = tradeErrorMessage(result.error());  

        auto response = drogon::HttpResponse::newHttpJsonResponse(errorBody);

        response->setStatusCode(drogon::k404NotFound); //response is provided by Drogons HttpResponse class, which is HTTP's response object

        callback(response); //callback is a drogon feature, essentailly marking that the request is done
        return;
    }

    // SUCCESS PATH
    const Trade& trade = result.value(); //Retrieves the successful Trade that is place inside Result<Trade, TradeError>, and refers to the one already inside the result 

    Json::Value body; 

    body["id"] = trade.getTradeId();
    body["instrument"] = trade.getInstrument();
    body["counterparty"] = trade.getCounterparty();
    body["price"] = trade.getPrice();
    body["quantity"] = trade.getQuantity();

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
        // Create one JSON object for this Trade
        Json::Value tradeJson;

        //Since its looping through the trades from trade service, it takes that and appends it to Json object which is then put into a JSON array, essentailly acting as a funnel 
        tradeJson["id"] = trade.getTradeId();
        tradeJson["instrument"] = trade.getInstrument();
        tradeJson["counterparty"] = trade.getCounterparty();
        tradeJson["price"] = trade.getPrice();
        tradeJson["quantity"] = trade.getQuantity();

        body.append(tradeJson);
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
void createTrade(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const 
{
    // 1. Get JSON body from request
    auto jsonBody = req->getJsonObject();  //Drogons incoming HTTP request, pretty much says "Take the body of this request and parse it as JSON if possible

    // 2. If body is not valid JSON, return HTTP 400
    if (!jsonBody){
        Json::Value errorBody;
        errorBody["error"] = "Invalid JSON body";
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(errorBody);
        response->setStatusCode(drogon::k400BadRequest);
        
        callback(response);
        return;  //Without return code could continue to extracting fields even though jsonBody is null 
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
        errorBody["error"] = "Side must be either Buy or Sell";

        auto response = drogon::HttpResponse::newHttpJsonResponse(errorBody);
        response->setStatusCode(drogon::k400BadRequest);

        callback(response);
        return;
    }
    // 5. Call m_service.createTrade()
    auto result = m_service.createTrade(instrument, counterparty, side, price, quantity);

    // 6. If result failed:create JSON error, status 400, callback, return
    
    if (!result.hasValue()){
        Json::Value errorBody; 
        errorBody["error"] = tradeErrorMessage(result.error());  

        auto response = drogon::HttpResponse::newHttpJsonResponse(errorBody);
        response->setStatusCode(drogon::k400NotFound); 

        callback(response); 
        return;
    }


    // 7. Get successful Trade
    const Trade& trade = result.value();

    // 8. Convert Trade to JSON
        Json::Value body;
        body["id"] = trade.getTradeId();
        body["instrument"] = trade.getInstrument();
        body["counterparty"] = trade.getCounterparty();
        body["price"] = trade.getPrice();
        body["quantity"] = trade.getQuantity();


    // 9. Create response
    auto response = drogon::HttpResponse::newHttpJsonResponse(body); 
    
    // 10. Use HTTP 201 Created
    response->setStatusCode(drogon::k201Created);

    // 11. callback(response)
    callback(response); 
}

