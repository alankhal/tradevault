#include "tradevault/PostgresConnectionPool.hpp"

#include <stdexcept>


PostgresConnectionPool::PostgresConnectionPool(
    const std::string& connectionString,
    std::size_t poolSize
)
{
    // A pool with zero connections would cause acquire()
    // to wait forever, so reject it immediately.
    if (poolSize == 0)
    {
        throw std::invalid_argument(
            "PostgresConnectionPool size must be greater than zero"
        );
    }

    // Create all PostgreSQL connections once when the pool starts.
    for (std::size_t i = 0; i < poolSize; ++i)
    {
        m_availableConnections.push(
            std::make_shared<pqxx::connection>(connectionString)
        );
    }
}


std::shared_ptr<pqxx::connection> PostgresConnectionPool::acquire()
{
    // Only one thread at a time can access the connection queue.
    std::unique_lock<std::mutex> lock{m_mutex}; 

    // If every connection is currently being used,
    // wait until another request returns one to the pool.
    m_condition.wait(
        lock,
        [this]()
        {
            return !m_availableConnections.empty();
        }
    );

    // Take one available connection out of the pool.
    auto connection = m_availableConnections.front();
    m_availableConnections.pop();

    // Return a temporary shared_ptr representing the borrowed connection.
    // When this shared_ptr goes out of scope, the custom deleter places
    // the real connection back into the pool instead of destroying it.
    return std::shared_ptr<pqxx::connection>(
        connection.get(),

        [this, connection](pqxx::connection*) mutable
        {
            {
                std::lock_guard<std::mutex> lock{m_mutex};

                // Make the connection available for another request.
                m_availableConnections.push(std::move(connection));
            }

            // Wake one request that may be waiting for a connection.
            m_condition.notify_one();
        }
    );
}