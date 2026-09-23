  #include "tradevault/api/TradeController.hpp"

#include "tradevault/TradeController.hpp"

//Use initializer list to set up constructor
TradeController::TradeController(TradeService& service)
    : m_service(service){}

void TradeController::getTrade(
    const drogon::HttpRequestPtr& /*req*/,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    std::unsigned id) const
{
    auto trade = m_service.getTrade(id);   // adapt to your TradeService API

    if (!trade)
    {
        Json::Value err;
        err["error"] = "trade not found";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
        return;
    }

    Json::Value body;
    body["id"]       = static_cast<Json::unsigned>(trade->id);
    body["symbol"]   = trade->symbol;
    body["quantity"] = trade->quantity;
    body["price"]    = trade->price;

    callback(drogon::HttpResponse::newHttpJsonResponse(body));
}