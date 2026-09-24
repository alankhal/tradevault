//Big distinction on this portion of the project is that Result.hpp is how TradeService communicates success or failure back to TradeController

#include "api/TradeController.hpp"

//Use initializer list to set up constructor
TradeController::TradeController(TradeService& service)
    : m_service(service){}


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