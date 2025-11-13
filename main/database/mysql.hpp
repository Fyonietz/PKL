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
#include <vector>
#include <variant>

using namespace nlohmann;

// Helper class for prepared statements
class PreparedStatement {
private:
    MYSQL_STMT* stmt;
    std::vector<MYSQL_BIND> binds;
    std::vector<std::variant<std::string, int, double, long long, std::nullptr_t>> values;
    std::vector<unsigned long> lengths;
    std::vector<my_bool> is_nulls;
    int current_pos = 1;  // Auto-increment position
    
public:
    PreparedStatement(MYSQL* conn, const std::string& query);
    ~PreparedStatement();
    
    // Chainable bind methods (auto position)
    PreparedStatement* bind(const std::string& value);
    PreparedStatement* bind(const char* value);
    PreparedStatement* bind(int value);
    PreparedStatement* bind(double value);
    PreparedStatement* bind(long long value);
    PreparedStatement* bind(std::nullptr_t);
    
    // Bind by explicit position (1-indexed)
    PreparedStatement* bind(int pos, const std::string& value);
    PreparedStatement* bind(int pos, const char* value);
    PreparedStatement* bind(int pos, int value);
    PreparedStatement* bind(int pos, double value);
    PreparedStatement* bind(int pos, long long value);
    PreparedStatement* bind(int pos, std::nullptr_t);
    
    // Execute the statement
    bool execute();
    
    // Get affected rows
    unsigned long long affected_rows();
    
    // Get last insert id
    unsigned long long insert_id();
    
    // Get result set for SELECT queries
    json get_results();
    
private:
    void bind_internal(int pos, const std::string& value);
    void bind_internal(int pos, int value);
    void bind_internal(int pos, double value);
    void bind_internal(int pos, long long value);
    void bind_internal(int pos, std::nullptr_t);
};

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
    
    // Prepared statement SELECT - returns JSON directly
    json db_select_prep(MYSQL* conn, const std::string& query, 
                        std::function<void(PreparedStatement*)> bind_params = nullptr);
    
    void db_print_error(MYSQL* conn);
    
    // Prepared statement operations
    PreparedStatement* db_prep(MYSQL* conn, const std::string& query);
    
    // Static method to get pool instance
    static MySQLPool* getInstance();
};

#endif // MYSQL_POOL_HPP
