
#ifndef MYSQL_HPP
#define MYSQL_HPP

#include <mysql/mysql.h>
#include <json.hpp>
#include <string>
#include <cstdlib>  // For std::getenv
#include <stdexcept>
#include <mutex>
#include <pyro.hpp> // Assuming Server.env() is provided by this

using namespace nlohmann;

class MySQL {
private:
    MYSQL *conn;
    static MySQL* instance;
    static std::mutex mutex;

    // Private constructor to prevent instantiation
    MySQL(const std::string& host, const std::string& user, const std::string& password, const std::string& dbname, unsigned int port);

public:
    // Deleted copy constructor and assignment operator to enforce singleton behavior
    MySQL(const MySQL&) = delete;
    MySQL& operator=(const MySQL&) = delete;

    // Static method to get the singleton instance
    static MySQL* getInstance();

    // Destructor to clean up the connection
    ~MySQL();

    // Public methods to perform database operations
    MYSQL* getConnection();
    int db_query(const char *query);
    MYSQL_RES* db_store_result();
    json db_select(const char *query);
    void db_print_error();
};

#endif // MYSQL_HPP




