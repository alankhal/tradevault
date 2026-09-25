#include <drogon/drogon.h>

#include <iostream>
#include <memory>
#include <tuple>

#include "api/TradeController.hpp"
#include "tradevault/InMemoryTradeRepository.hpp"
#include "tradevault/TradeService.hpp"

int main()
{
    // Load Drogon configuration
    drogon::app().loadConfigFile("config/config.json");

    // Register /healthz endpoint
    drogon::app().registerHandler(
        "/healthz",
        [](const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&& callback)
        {
            Json::Value json;
            json["status"] = "ok";

            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
            resp->setStatusCode(drogon::k200OK);

            callback(resp);
        },
        {drogon::Get}
    );

    // 1. Create repository
    InMemoryTradeRepository repository;

    // 2. Inject repository into TradeService
    TradeService service(repository);

    // 3. Inject TradeService into TradeController
    auto controller = std::make_shared<TradeController>(service);

    // 4. Register TradeController with Drogon
    drogon::app().registerController(controller);

    // Temporary diagnostic:
    // Print every route Drogon believes is registered
    for (const auto& handler : drogon::app().getHandlersInfo())
    {
        std::cout << "Registered route: " << std::get<0>(handler) << '\n';
    }

    // Start the HTTP server
    drogon::app().run();
}