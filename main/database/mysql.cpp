#include "mysql.hpp"
#include <iostream>
#include <chrono>

// REMOVE THIS LINE: MySQLPool* MySQLPool::instance = nullptr;

MySQLPool::MySQLPool(const std::string& host, const std::string& user, 
                     const std::string& password, const std::string& dbname, 
                     unsigned int port, size_t max_pool_size)
    : host(host), user(user), password(password), dbname(dbname), 
      port(port), max_pool_size(max_pool_size), current_pool_size(0) {
    
    // Create initial connections
    for (size_t i = 0; i < 5 && i < max_pool_size; ++i) {
        try {
            MYSQL* conn = create_connection();
            if (conn) {
                connections.push(conn);
                current_pool_size++;
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to create initial connection: " << e.what() << std::endl;
        }
    }
}

MySQLPool::~MySQLPool() {
    std::lock_guard<std::mutex> lock(mutex);
    while (!connections.empty()) {
        MYSQL* conn = connections.front();
        connections.pop();
        mysql_close(conn);
    }
}

MYSQL* MySQLPool::create_connection() {
    MYSQL* conn = mysql_init(nullptr);
    if (conn == nullptr) {
        throw std::runtime_error("mysql_init() failed");
    }

    // Set connection options - FIXED SYNTAX
    my_bool reconnect = 1;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
    mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    if (mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), 
                          dbname.c_str(), port, nullptr, 0) == nullptr) {
        mysql_close(conn);
        throw std::runtime_error("mysql_real_connect() failed: " + std::string(mysql_error(conn)));
    }

    return conn;
}

std::shared_ptr<MYSQL> MySQLPool::get_connection() {
    std::unique_lock<std::mutex> lock(mutex);
    
    // Wait for available connection or ability to create new one
    condition.wait(lock, [this]() {
        return !connections.empty() || current_pool_size < max_pool_size;
    });

    if (!connections.empty()) {
        MYSQL* conn = connections.front();
        connections.pop();
        
        // Check if connection is still alive
        if (mysql_ping(conn) != 0) {
            mysql_close(conn);
            current_pool_size--;
            // Create new connection
            conn = create_connection();
            current_pool_size++;
        }
        
        return std::shared_ptr<MYSQL>(conn, [this](MYSQL* conn) {
            this->return_connection(conn);
        });
    } else if (current_pool_size < max_pool_size) {
        // Create new connection
        MYSQL* conn = create_connection();
        current_pool_size++;
        return std::shared_ptr<MYSQL>(conn, [this](MYSQL* conn) {
            this->return_connection(conn);
        });
    }

    throw std::runtime_error("Failed to get database connection");
}

void MySQLPool::return_connection(MYSQL* conn) {
    std::lock_guard<std::mutex> lock(mutex);
    connections.push(conn);
    condition.notify_one();
}

int MySQLPool::db_query(MYSQL* conn, const char *query) {
    if (mysql_query(conn, query)) {
        return 1;
    }
    return 0;
}

MYSQL_RES* MySQLPool::db_store_result(MYSQL* conn) {
    MYSQL_RES *res = mysql_store_result(conn);
    if (res == nullptr) {
        return nullptr;
    }
    return res;
}

json MySQLPool::db_select(MYSQL* conn, const char *query) {
    if (mysql_query(conn, query) != 0) {
        return json();
    }

    MYSQL_RES *res = db_store_result(conn);
    if (res == nullptr) {
        return json();
    }

    MYSQL_FIELD *fields;
    MYSQL_ROW row;
    int num_fields = mysql_num_fields(res);
    json result_json = json::array();

    fields = mysql_fetch_fields(res);

    while ((row = mysql_fetch_row(res)) != nullptr) {
        json row_json;
        unsigned long *lengths = mysql_fetch_lengths(res);

        for (int i = 0; i < num_fields; i++) {
            if (row[i] != nullptr) {
                row_json[fields[i].name] = std::string(row[i], lengths[i]);
            } else {
                row_json[fields[i].name] = nullptr;
            }
        }
        result_json.push_back(row_json);
    }

    mysql_free_result(res);
    return result_json;
}

void MySQLPool::db_print_error(MYSQL* conn) {
    std::cerr << "MySQL Error: " << mysql_error(conn) << std::endl;
}

MySQLPool* MySQLPool::getInstance() {
    static MySQLPool instance(
        Server.env()["DB_HOST"].c_str(),
        Server.env()["DB_USER"].c_str(),
        Server.env()["DB_PW"].c_str(),
        Server.env()["DB_NAME"].c_str(),
        Server.env().count("DB_PORT") ? std::stoi(Server.env()["DB_PORT"]) : 3306,
        20  // max pool size
    );
    return &instance;
}



