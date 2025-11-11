#include "mysql.hpp"
#include <chrono>
#include <cstring>
#include <iostream>

// PreparedStatement implementation
PreparedStatement::PreparedStatement(MYSQL *conn, const std::string &query) {
  stmt = mysql_stmt_init(conn);
  if (!stmt) {
    throw std::runtime_error("mysql_stmt_init() failed");
  }

  if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
    std::string error = mysql_stmt_error(stmt);
    mysql_stmt_close(stmt);
    throw std::runtime_error("mysql_stmt_prepare() failed: " + error);
  }

  // Get parameter count
  unsigned long param_count = mysql_stmt_param_count(stmt);
  if (param_count > 0) {
    binds.resize(param_count);
    values.resize(param_count);
    lengths.resize(param_count);
    is_nulls.resize(param_count);

    // Initialize binds
    memset(binds.data(), 0, sizeof(MYSQL_BIND) * param_count);
  }
}

PreparedStatement::~PreparedStatement() {
  if (stmt) {
    mysql_stmt_close(stmt);
  }
}

void PreparedStatement::bind_internal(int pos, const std::string &value) {
  int idx = pos - 1; // Convert to 0-indexed
  if (idx < 0 || idx >= binds.size()) {
    throw std::out_of_range("Parameter position out of range");
  }

  values[idx] = value;
  lengths[idx] = value.length();
  is_nulls[idx] = 0;

  binds[idx].buffer_type = MYSQL_TYPE_STRING;
  binds[idx].buffer = (char *)std::get<std::string>(values[idx]).c_str();
  binds[idx].buffer_length = lengths[idx];
  binds[idx].length = &lengths[idx];
  binds[idx].is_null = &is_nulls[idx];
}

void PreparedStatement::bind_internal(int pos, int value) {
  int idx = pos - 1;
  if (idx < 0 || idx >= binds.size()) {
    throw std::out_of_range("Parameter position out of range");
  }

  values[idx] = value;
  is_nulls[idx] = 0;

  binds[idx].buffer_type = MYSQL_TYPE_LONG;
  binds[idx].buffer = (char *)&std::get<int>(values[idx]);
  binds[idx].is_null = &is_nulls[idx];
}

void PreparedStatement::bind_internal(int pos, double value) {
  int idx = pos - 1;
  if (idx < 0 || idx >= binds.size()) {
    throw std::out_of_range("Parameter position out of range");
  }

  values[idx] = value;
  is_nulls[idx] = 0;

  binds[idx].buffer_type = MYSQL_TYPE_DOUBLE;
  binds[idx].buffer = (char *)&std::get<double>(values[idx]);
  binds[idx].is_null = &is_nulls[idx];
}

void PreparedStatement::bind_internal(int pos, long long value) {
  int idx = pos - 1;
  if (idx < 0 || idx >= binds.size()) {
    throw std::out_of_range("Parameter position out of range");
  }

  values[idx] = value;
  is_nulls[idx] = 0;

  binds[idx].buffer_type = MYSQL_TYPE_LONGLONG;
  binds[idx].buffer = (char *)&std::get<long long>(values[idx]);
  binds[idx].is_null = &is_nulls[idx];
}

void PreparedStatement::bind_internal(int pos, std::nullptr_t) {
  int idx = pos - 1;
  if (idx < 0 || idx >= binds.size()) {
    throw std::out_of_range("Parameter position out of range");
  }

  values[idx] = nullptr;
  is_nulls[idx] = 1;

  binds[idx].buffer_type = MYSQL_TYPE_NULL;
  binds[idx].is_null = &is_nulls[idx];
}

// Chainable auto-position bind methods
PreparedStatement *PreparedStatement::bind(const std::string &value) {
  bind_internal(current_pos++, value);
  return this;
}

PreparedStatement *PreparedStatement::bind(const char *value) {
  bind_internal(current_pos++, std::string(value));
  return this;
}

PreparedStatement *PreparedStatement::bind(int value) {
  bind_internal(current_pos++, value);
  return this;
}

PreparedStatement *PreparedStatement::bind(double value) {
  bind_internal(current_pos++, value);
  return this;
}

PreparedStatement *PreparedStatement::bind(long long value) {
  bind_internal(current_pos++, value);
  return this;
}

PreparedStatement *PreparedStatement::bind(std::nullptr_t) {
  bind_internal(current_pos++, nullptr);
  return this;
}

// Explicit position bind methods
PreparedStatement *PreparedStatement::bind(int pos, const std::string &value) {
  bind_internal(pos, value);
  return this;
}

PreparedStatement *PreparedStatement::bind(int pos, const char *value) {
  bind_internal(pos, std::string(value));
  return this;
}

PreparedStatement *PreparedStatement::bind(int pos, int value) {
  bind_internal(pos, value);
  return this;
}

PreparedStatement *PreparedStatement::bind(int pos, double value) {
  bind_internal(pos, value);
  return this;
}

PreparedStatement *PreparedStatement::bind(int pos, long long value) {
  bind_internal(pos, value);
  return this;
}

PreparedStatement *PreparedStatement::bind(int pos, std::nullptr_t) {
  bind_internal(pos, nullptr);
  return this;
}

bool PreparedStatement::execute() {
  if (!binds.empty()) {
    if (mysql_stmt_bind_param(stmt, binds.data())) {
      std::cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt)
                << std::endl;
      return false;
    }
  }

  if (mysql_stmt_execute(stmt)) {
    std::cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt)
              << std::endl;
    return false;
  }

  return true;
}

unsigned long long PreparedStatement::affected_rows() {
  return mysql_stmt_affected_rows(stmt);
}

unsigned long long PreparedStatement::insert_id() {
  return mysql_stmt_insert_id(stmt);
}

json PreparedStatement::get_results() {
  MYSQL_RES *result_metadata = mysql_stmt_result_metadata(stmt);
  if (!result_metadata) {
    return json::array();
  }

  unsigned int num_fields = mysql_num_fields(result_metadata);
  MYSQL_FIELD *fields = mysql_fetch_fields(result_metadata);

  // Prepare result bindings
  std::vector<MYSQL_BIND> result_binds(num_fields);
  std::vector<char> buffers[num_fields];
  std::vector<unsigned long> result_lengths(num_fields);
  std::vector<my_bool> result_is_nulls(num_fields);

  memset(result_binds.data(), 0, sizeof(MYSQL_BIND) * num_fields);

  for (unsigned int i = 0; i < num_fields; i++) {
    buffers[i].resize(1024); // Allocate buffer
    result_binds[i].buffer_type = MYSQL_TYPE_STRING;
    result_binds[i].buffer = buffers[i].data();
    result_binds[i].buffer_length = 1024;
    result_binds[i].length = &result_lengths[i];
    result_binds[i].is_null = &result_is_nulls[i];
  }

  if (mysql_stmt_bind_result(stmt, result_binds.data())) {
    mysql_free_result(result_metadata);
    return json::array();
  }

  json result_json = json::array();

  while (mysql_stmt_fetch(stmt) == 0) {
    json row_json;
    for (unsigned int i = 0; i < num_fields; i++) {
      if (result_is_nulls[i]) {
        row_json[fields[i].name] = nullptr;
      } else {
        row_json[fields[i].name] =
            std::string(buffers[i].data(), result_lengths[i]);
      }
    }
    result_json.push_back(row_json);
  }

  mysql_free_result(result_metadata);
  return result_json;
}

// MySQLPool implementation (existing code remains the same)
MySQLPool::MySQLPool(const std::string &host, const std::string &user,
                     const std::string &password, const std::string &dbname,
                     unsigned int port, size_t max_pool_size)
    : host(host), user(user), password(password), dbname(dbname), port(port),
      max_pool_size(max_pool_size), current_pool_size(0) {

  for (size_t i = 0; i < 5 && i < max_pool_size; ++i) {
    try {
      MYSQL *conn = create_connection();
      if (conn) {
        connections.push(conn);
        current_pool_size++;
      }
    } catch (const std::exception &e) {
      std::cerr << "Failed to create initial connection: " << e.what()
                << std::endl;
    }
  }
}

MySQLPool::~MySQLPool() {
  std::lock_guard<std::mutex> lock(mutex);
  while (!connections.empty()) {
    MYSQL *conn = connections.front();
    connections.pop();
    mysql_close(conn);
  }
}

MYSQL *MySQLPool::create_connection() {
  MYSQL *conn = mysql_init(nullptr);
  if (conn == nullptr) {
    throw std::runtime_error("mysql_init() failed");
  }

  my_bool reconnect = 1;
  mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
  mysql_options(conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");

  if (mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                         dbname.c_str(), port, nullptr, 0) == nullptr) {
    mysql_close(conn);
    throw std::runtime_error("mysql_real_connect() failed: " +
                             std::string(mysql_error(conn)));
  }

  return conn;
}

std::shared_ptr<MYSQL> MySQLPool::get_connection() {
  std::unique_lock<std::mutex> lock(mutex);

  condition.wait(lock, [this]() {
    return !connections.empty() || current_pool_size < max_pool_size;
  });

  if (!connections.empty()) {
    MYSQL *conn = connections.front();
    connections.pop();

    if (mysql_ping(conn) != 0) {
      mysql_close(conn);
      current_pool_size--;
      conn = create_connection();
      current_pool_size++;
    }

    return std::shared_ptr<MYSQL>(
        conn, [this](MYSQL *conn) { this->return_connection(conn); });
  } else if (current_pool_size < max_pool_size) {
    MYSQL *conn = create_connection();
    current_pool_size++;
    return std::shared_ptr<MYSQL>(
        conn, [this](MYSQL *conn) { this->return_connection(conn); });
  }

  throw std::runtime_error("Failed to get database connection");
}

void MySQLPool::return_connection(MYSQL *conn) {
  std::lock_guard<std::mutex> lock(mutex);
  connections.push(conn);
  condition.notify_one();
}

int MySQLPool::db_query(MYSQL *conn, const char *query) {
  if (mysql_query(conn, query)) {
    return 1;
  }
  return 0;
}

MYSQL_RES *MySQLPool::db_store_result(MYSQL *conn) {
  MYSQL_RES *res = mysql_store_result(conn);
  if (res == nullptr) {
    return nullptr;
  }
  return res;
}

json MySQLPool::db_select(MYSQL *conn, const char *query) {
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

void MySQLPool::db_print_error(MYSQL *conn) {
  std::cerr << "MySQL Error: " << mysql_error(conn) << std::endl;
}

PreparedStatement *MySQLPool::db_prep(MYSQL *conn, const std::string &query) {
  return new PreparedStatement(conn, query);
}

MySQLPool *MySQLPool::getInstance() {
  static MySQLPool instance(
      Server.env()["DB_HOST"].c_str(), Server.env()["DB_USER"].c_str(),
      Server.env()["DB_PW"].c_str(), Server.env()["DB_NAME"].c_str(),
      Server.env().count("DB_PORT") ? std::stoi(Server.env()["DB_PORT"]) : 3306,
      20);
  return &instance;
}
