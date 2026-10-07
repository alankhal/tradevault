#include <drogon/drogon.h>

#include <iostream>
#include <memory>
#include <tuple>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "api/TradeController.hpp"
#include "tradevault/TradeService.hpp"
#include "tradevault/PostgresTradeRepository.hpp"

int main()
{
    // Read the PostgreSQL connection information from the environment variable.
    const char* dbConnection =
        std::getenv("TRADEVAULT_DB_CONNECTION");

    // Stop startup if the database connection information was not provided.
    if (dbConnection == nullptr)
    {
        throw std::runtime_error(
            "TRADEVAULT_DB_CONNECTION environment variable is not set"
        );
    }

    // Convert the environment variable into a std::string.
    const std::string connectionString{dbConnection};

    // Load Drogon configuration
    drogon::app().loadConfigFile("config/config.json");

    // Register /healthz endpoint
    drogon::app().registerHandler(
        "/healthz",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback)
        {
            Json::Value json;
            json["status"] = "ok";

            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
            resp->setStatusCode(drogon::k200OK);

            callback(resp);
        },
        {drogon::Get}
    );

    // 1. Create PostgreSQL repository
    PostgresTradeRepository repository{connectionString};

    // 2. Inject repository into TradeService
    TradeService service{repository};

    // 3. Inject TradeService into TradeController
    // Double check if shared pointer is the best option here?
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