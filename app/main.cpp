#include <drogon/drogon.h>

int main()
{
    // Get Drogon's application

    // Load config/config.json
    drogon::app().loadConfigFile("config/config.json");

    // Run the server
    drogon::app().run();
}