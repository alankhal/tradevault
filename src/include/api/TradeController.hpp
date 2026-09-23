#pragma once

#include <functional>
#include <drogon/HttpController.h>
#include "tradevault/TradeService.hpp"

class TradeController: public drogon::HttpController<TradeController, false> 
{
private:
    TradeService& m_service;

public:
    //Takes a reference from tradeservice, meaning a controller cannot exist without being given a TradeService  
    explicit TradeController(TradeService& service);

    METHOD_LIST_BEGIN

    // First route goes here:
    // GET /trades/{id}
    ADD_METHOD_TO(TradeController::getTrade, "/trades/{id}", drogon::Get); 

    METHOD_LIST_END

    // First handler here
    void getTrade (const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                  std::unsigned id) const;
};