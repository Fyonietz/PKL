#include "models.hpp"
#include "mysql.hpp"
#include <exception>
#include <iostream>
#include <middleware.hpp>
#include <phoenix.hpp>
#include <string>

route("/api/admin/user/create", create_user) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);

  CORS(connection);

  if (!authInfo) {
    Server.Response(connection, 401, "Unauthorized",
                    R"({"error":"Unauthorized"})");
    return 401;
  }
  std::string post = Server.Read(connection);

  if (!ASYNC) {
    Server.Response(connection, 500, "Error",
                    R"({"Message":"ASYNC mode disabled"})");
    return 500;
  }

  // Main logic runs when ASYNC is true
  auto a = Server.method.async([&]() {
    try {
      auto db = MySQLPool::getInstance();
      auto conn = db->get_connection();

      nlohmann::json post_as_json = nlohmann::json::parse(post);

      Users.id = post_as_json.value("Id", 0);
      Users.nama = post_as_json.value("Nama", "");
      Users.password = post_as_json.value("Password", "");
      Users.rolesId = post_as_json.value("Roles", 0);

      Users.kelasId = post_as_json.value("KelasId", 1);
      Users.jurusanId = post_as_json.value("JurusanId", 1);
      Users.namaPembimbing = post_as_json.value("NamaPembimbing", "");

      Users.tempatPerusahaanId = post_as_json.value("TempatPerusahaanId", 1);
      Users.siswaDibimbing = post_as_json.value("SiswaDibimbing", "");
      Users.jurusanKaprodiId = post_as_json.value("JurusanKaprodiId", 1);
      Users.jurusanPerusahaanId = post_as_json.value("JurusanPerusahaanId", 1);
      // Token creation
      std::string recipe = Users.nama + Users.password + "akankupakai";
      std::string token = Auth::tokenizer(std::move(recipe));

      // Build query (use Escape for safety)

      std::string query = "INSERT INTO Users ("
                          "Nama, Password, Token, Roles, KelasId, JurusanId, "
                          "NamaPembimbing, TempatPerusahaanId, SiswaDibimbing, "
                          "JurusanKaprodiId, JurusanPerusahaanId"
                          ") VALUES ('" +
                          Escape(Users.nama) + "', '" + Escape(Users.password) +
                          "', '" + Escape(token) + "', " +
                          std::to_string(Users.rolesId) + ", " +
                          std::to_string(Users.kelasId) + ", " +
                          std::to_string(Users.jurusanId) + ", '" +
                          Escape(Users.namaPembimbing) + "', " +
                          std::to_string(Users.tempatPerusahaanId) + ", '" +
                          Escape(Users.siswaDibimbing) + "', " +
                          std::to_string(Users.jurusanKaprodiId) + ", " +
                          std::to_string(Users.jurusanPerusahaanId) + ");";
      db->db_query(conn.get(), query.c_str());

      Server.Response(connection, 200, "Ok",
                      R"({"Message":"User Registered Succesfully"})");
      return 200;
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      Server.Response(connection, 500, "Error",
                      R"({"Message":"Internal Server Error"})");
      return 500;
    }
  });

  a.get();
  return 200;
}

route("/api/admin/user/read", read_user_test) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  CORS(connection);

  if (!authInfo) {
    Server.Response(connection, 401, "Unauthorized",
                    R"({"error":"Unauthorized"})");
    return 401;
  }

  if (!ASYNC) {
    Server.Response(connection, 500, "Error",
                    R"({"Message":"ASYNC mode Error"})");
    return 500;
  }

  // Read the POST data BEFORE entering the async function
  std::string post_data;
  const struct mg_request_info *req_info = mg_get_request_info(connection);

  if (strcmp(req_info->request_method, "POST") == 0) {
    post_data = Server.Read(connection);
  }

  try {
    auto a = Server.method.async(
        [&, post_data]()
            -> std::pair<int, std::string> { // Capture post_data by value
          auto db = MySQLPool::getInstance();
          auto conn = db->get_connection();
          nlohmann::json query;

          if (strcmp(req_info->request_method, "GET") == 0) {
            query = db->db_select(conn.get(), "SELECT * FROM Users");
          } else if (strcmp(req_info->request_method, "POST") == 0) {
            if (!post_data.empty()) {
              try {
                nlohmann::json post_as_json = nlohmann::json::parse(post_data);
                std::string key = post_as_json.value("where", "Roles");
                const auto &value = post_as_json["value"];
                if (value.is_number_integer()) {
                  // IMPORTANT: Escape the key to prevent SQL injection
                  char escaped_key[key.length() * 2 + 1];
                  mysql_real_escape_string(conn.get(), escaped_key, key.c_str(),
                                           key.length());
                  int valInt = value.get<int>();
                  std::string querys = "SELECT * FROM Users WHERE " +
                                       std::string(escaped_key) + "=" +
                                       std::to_string(valInt);
                  query = db->db_select(conn.get(), querys.c_str());
                } else if (value.is_string()) {
                  // IMPORTANT: Escape the key to prevent SQL injection
                  char escaped_key[key.length() * 2 + 1];
                  mysql_real_escape_string(conn.get(), escaped_key, key.c_str(),
                                           key.length());
                  std::string valString = value.get<std::string>();
                  std::string querys = "SELECT * FROM Users WHERE " +
                                       std::string(escaped_key) + "=" + "'" +
                                       valString + "'";
                  query = db->db_select(conn.get(), querys.c_str());
                }

              } catch (const std::exception &e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
                return std::make_pair(400, R"({"error":"Invalid JSON"})");
              }
            } else {
              return std::make_pair(400, R"({"error":"No POST data"})");
            }
          } else {
            return std::make_pair(405, R"({"error":"Method Not Allowed"})");
          }

          if (query.is_array() && !query.empty()) {
            std::string result = query.dump();
            return std::make_pair(200, result);
          } else {
            return std::make_pair(404, R"({"error":"No users found"})");
          }
        });

    auto [status, response_data] = a.get();
    Server.Response(connection, status, status == 200 ? "Ok" : "Error",
                    response_data);
    return status;

  } catch (std::exception &e) {
    std::cerr << "Internal Error: " << e.what() << std::endl;
    Server.Response(connection, 500, "Error",
                    R"({"error":"Internal Server Error"})");
    return 500;
  }
}
