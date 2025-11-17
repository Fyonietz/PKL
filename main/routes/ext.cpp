#include "mysql.hpp"
#include <exception>
#include <iostream>
#include <middleware.hpp>
#include <phoenix.hpp>

route("/api/roles/read", ext_roles) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);

  CORS(connection);

  if (!authInfo) {
    Server.Response(connection, 401, "Unauthorized",
                    R"({"error":"Unauthorized"})");
    return 401;
  }

  // Method check (only allow GET, others return 405)
  if (!Method(connection, "GET")) {
    return 405;
  }
  // ASYNC block
  if (ASYNC) {
    try {
      // Async task
      auto a = Server.method.async([&]() -> std::pair<int, std::string> {
        auto db = MySQLPool::getInstance();
        auto conn = db->get_connection();

        // Query roles
        nlohmann::json query = db->db_select(conn.get(), "SELECT * FROM Roles");
        std::string result = query.dump();
        return std::make_pair(200, result);
      });

      // Retrieve result from async
      auto [status, response_data] = a.get();
      Server.Response(connection, status, status == 200 ? "Ok" : "Error",
                      response_data);
      return status;

    } catch (std::exception &e) {
      // Error handling for exceptions
      std::cerr << "Exception in /api/ext/roles: " << e.what() << std::endl;
      Server.Response(connection, 500, "Internal Server Error",
                      R"({"error": "Internal Server Error"})");
      return 500;
    }
  }

  // Fallback return 500 if ASYNC is not true or any error occurs
  return 500;
}

route("/api/jurusan/read", ext_jurusan) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);

  CORS(connection);

  if (!authInfo) {
    Server.Response(connection, 401, "Unauthorized",
                    R"({"error":"Unauthorized"})");
    return 401;
  }

  // Method check (only allow GET, others return 405)
  if (!Method(connection, "GET")) {
    return 405;
  }
  // ASYNC block
  if (ASYNC) {
    try {
      // Async task
      auto a = Server.method.async([&]() -> std::pair<int, std::string> {
        auto db = MySQLPool::getInstance();
        auto conn = db->get_connection();

        // Query roles
        nlohmann::json query =
            db->db_select(conn.get(), "SELECT * FROM Jurusan");
        std::string result = query.dump();
        return std::make_pair(200, result);
      });

      // Retrieve result from async
      auto [status, response_data] = a.get();
      Server.Response(connection, status, status == 200 ? "Ok" : "Error",
                      response_data);
      return status;

    } catch (std::exception &e) {
      // Error handling for exceptions
      std::cerr << "Exception in /api/ext/Jurusan: " << e.what() << std::endl;
      Server.Response(connection, 500, "Internal Server Error",
                      R"({"error": "Internal Server Error"})");
      return 500;
    }
  }

  // Fallback return 500 if ASYNC is not true or any error occurs
  return 500;
}
