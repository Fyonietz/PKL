#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql.hpp>
#include <json.hpp>
#include <iostream>
using namespace nlohmann;

// Initialize the static member
MySQL* MySQL::instance = nullptr;
std::mutex MySQL::mutex;

MySQL::MySQL(const std::string& host, const std::string& user, const std::string& password, const std::string& dbname, unsigned int port) {
    conn = mysql_init(nullptr);
    if (conn == nullptr) {
        throw std::runtime_error("mysql_init() failed");
    }

    if (mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0) == nullptr) {
        mysql_close(conn);
        throw std::runtime_error("mysql_real_connect() failed: " + std::string(mysql_error(conn)));
    }
}

MySQL* MySQL::getInstance() {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance == nullptr) {
        const char* host = Server.env()["DB_HOST"].c_str();
        const char* user = Server.env()["DB_USER"].c_str();
        const char* password = Server.env()["DB_PW"].c_str();
        const char* dbname = Server.env()["DB_NAME"].c_str();
        unsigned int port = 3306;
        if (Server.env().count("DB_PORT")) {
            port = std::stoi(Server.env()["DB_PORT"]);
        }

        instance = new MySQL(host, user, password, dbname, port);
    }
    return instance;
}

MySQL::~MySQL() {
    if (conn != nullptr) {
        mysql_close(conn);
    }
}

MYSQL* MySQL::getConnection() {
    return conn;
}

int MySQL::db_query(const char *query) {
    if (mysql_query(conn, query)) {
        return 1; // Query failed
    }
    return 0; // Query succeeded
}

MYSQL_RES* MySQL::db_store_result() {
    MYSQL_RES *res = mysql_store_result(conn);
    if (res == nullptr) {
        return nullptr; // Return nullptr on error
    }
    return res;
}

json MySQL::db_select(const char *query) {
    if (mysql_query(conn, query) != 0) {
        return json();  // Return empty JSON if query fails
    }

    MYSQL_RES *res = db_store_result();
    if (res == nullptr) {
        return json();  // Return empty JSON if result fetching fails
    }

    MYSQL_FIELD *field;
    MYSQL_ROW row;
    int num_fields = mysql_num_fields(res);  // Get the number of columns
    json result_json = json::array();  // JSON array to hold results

    while ((row = mysql_fetch_row(res)) != nullptr) {
        json row_json;  // Create JSON object for the row
        unsigned long *lengths = mysql_fetch_lengths(res);  // Get column lengths

        for (int i = 0; i < num_fields; i++) {
            field = mysql_fetch_field(res);  // Get column metadata
            if (row[i] != nullptr) {
                row_json[field->name] = std::string(row[i], lengths[i]);
            } else {
                row_json[field->name] = nullptr;  // NULL as JSON null
            }
        }
        result_json.push_back(row_json);  // Add row to JSON array
    }

    mysql_free_result(res);  // Free result set
    return result_json;
}

void MySQL::db_print_error() {
    std::cerr << "MySQL Error: " << mysql_error(conn) << std::endl;
}



