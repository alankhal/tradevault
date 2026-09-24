#include <drogon/drogon.h>
#include <memory>

#include "api/TradeController.hpp"
#include "tradevault/InMemoryTradeRepository.hpp"
#include "tradevault/TradeService.hpp"



int main()
{
    // Load config/config.json
    drogon::app().loadConfigFile("config/config.json");

// Registered healthz here
drogon::app().registerHandler(
    "/healthz",
    //Lambda used as named function is not writtenn here
    [](const drogon::HttpRequestPtr& req,
       std::function<void(const drogon::HttpResponsePtr&)>&& callback)
    {
        //Impliment json response for question 
        Json::Value json;
        json["status"] = "ok";

        auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
        resp->setStatusCode(drogon::k200OK);

        callback(resp);
    },

    {drogon::Get}
);


// 1. Create repository

// 2. Give repository to TradeService

// 3. Create TradeController and give it TradeService

// 4. Register controller with Drogon


    // Run the server
    drogon::app().run();
}