#pragma once

#include <functional>
#include <drogon/HttpController.h>
#include "tradevault/TradeService.hpp"

class TradeController : public drogon::HttpController<TradeController, false>
{
private:
    TradeService& m_service;

public:
    // Controller cannot exist without being given a TradeService
    explicit TradeController(TradeService& service);

    METHOD_LIST_BEGIN

    // GET /trades/{id}
    ADD_METHOD_TO(TradeController::getTrade, "/trades/{id}", drogon::Get);
    ADD_METHOD_TO(TradeController::listTrades, "/trades", drogon::Get);

    METHOD_LIST_END
    
    //Requires an Id parameter
    void getTrade(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, unsigned id) const;
    
    //Does not require ID parameter 
    void listTrades(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const;
};