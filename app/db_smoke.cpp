#include <pqxx/pqxx>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

int main()
{
    // Read the PostgreSQL connection information from the environment
    // instead of storing database credentials directly in source code.
    const char* dbConnection =
        std::getenv("TRADEVAULT_DB_CONNECTION");

    if (dbConnection == nullptr)
    {
        throw std::runtime_error(
            "TRADEVAULT_DB_CONNECTION environment variable is not set"
        );
    }

    const std::string connectionString{dbConnection};

    // Open the smoke-test connection using the environment configuration.
    pqxx::connection connection{connectionString};

    std::cout << "Connected to PostgreSQL successfully.\n";

    return 0;
}