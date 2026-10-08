//Maintains reusable PostgreSQL connections 

#pragma once

#include <pqxx/pqxx>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>


class PostgresConnectionPool
{
public:
    // Creates a fixed number of PostgreSQL connections when the application starts.
    PostgresConnectionPool(
        const std::string& connectionString,
        std::size_t poolSize
    );

    // Gets an available connection from the pool.
    std::shared_ptr<pqxx::connection> acquire();

private:
    std::mutex m_mutex;
    std::condition_variable m_condition;

    // Connections that are currently available for use.
    std::queue<std::shared_ptr<pqxx::connection>> m_availableConnections;
};