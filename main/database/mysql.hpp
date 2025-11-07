#ifndef MYSQL_POOL_HPP
#define MYSQL_POOL_HPP

#include <mysql/mysql.h>
#include <json.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <stdexcept>
#include <pyro.hpp>

using namespace nlohmann;

class MySQLPool {
private:
    std::queue<MYSQL*> connections;
    std::mutex mutex;
    std::condition_variable condition;
    std::string host, user, password, dbname;
    unsigned int port;
    size_t max_pool_size;
    std::atomic<size_t> current_pool_size;

    MYSQL* create_connection();

public:
    MySQLPool(const std::string& host, const std::string& user, 
              const std::string& password, const std::string& dbname, 
              unsigned int port = 3306, size_t max_pool_size = 20);
    
    ~MySQLPool();

    // Delete copy constructor and assignment operator
    MySQLPool(const MySQLPool&) = delete;
    MySQLPool& operator=(const MySQLPool&) = delete;

    std::shared_ptr<MYSQL> get_connection();
    void return_connection(MYSQL* conn);

    // Database operations
    int db_query(MYSQL* conn, const char *query);
    MYSQL_RES* db_store_result(MYSQL* conn);
    json db_select(MYSQL* conn, const char *query);
    void db_print_error(MYSQL* conn);
    
    // Static method to get pool instance
    static MySQLPool* getInstance();
};

#endif // MYSQL_POOL_HPP




