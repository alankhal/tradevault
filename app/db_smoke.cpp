//Inital database page
//Uses try and catch handling as libpqxx reports database failures through exceptions 

#include <iostream>
#include <pqxx/pqxx>

int main()
{
    try
    {
        // 1. Create a pqxx database connection.
        pqxx::connection connection{"host=localhost port=5432 dbname=tradevault user=postgres password=kX7mP2wN5v."};
        
        // 2. Work is unit of database work using that channel 
        pqxx::work transaction{connection};

        // 3. Start a transaction.
        transaction.exec("SELECT current_database();");

        // 4. Execute: SELECT current_database();
        pqxx::result result = transaction.exec("SELECT current_database();");

        // 5. Print the returned database name.
        std::string name = result[0][0].as<std::string>();

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Database error: " << e.what() << '\n';
        return 1;
    }
}