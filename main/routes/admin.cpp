#include "models.hpp"
#include "mysql.hpp"
#include <exception>
#include <iostream>
#include <middleware.hpp>
#include <phoenix.hpp>

route("/api/admin/user/create", create_user) {
  auto authInfo = CheckAuthToken(connection,Auth::Roles::Admin);
  if(!authInfo){
    Server.Response(connection,401,"Unauthorized","");
    return 401;
  }
  std::string post = Server.Read(connection);
  CORS(connection);

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

route("/api/admin/user/read", read_user) {
  std::cout << "=== /api/admin/user/read started ===" << std::endl;
  
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  std::cout << "Auth result: " << (authInfo ? "SUCCESS" : "FAILED") << std::endl;
  
  CORS(connection);
  
  if(!authInfo){
    std::cout << "Returning 401 Unauthorized" << std::endl;
    Server.Response(connection, 401, "Unauthorized", R"({"error":"Unauthorized"})");
    return 401;
  }
  
  if (!ASYNC) {
    Server.Response(connection, 500, "Error", R"({"Message":"ASYNC mode Error"})");
    return 500;
  }
  
  try {
    std::cout << "Starting database query..." << std::endl;
    
    auto a = Server.method.async([&]() -> std::pair<int, std::string> {
      auto db = MySQLPool::getInstance();
      auto conn = db->get_connection();

      nlohmann::json query = db->db_select(conn.get(), "SELECT * FROM Users");
      std::cout << "Query result type: " << query.type_name() << std::endl;
      std::cout << "Query result size: " << query.size() << std::endl;
      
      if (query.is_array() && !query.empty()) {
        std::string result = query.dump();
        std::cout << "First 100 chars of result: " << result.substr(0, 100) << std::endl;
        return std::make_pair(200, result);
      } else {
        return std::make_pair(404, R"({"error":"No users found"})");
      }
    });
    
    auto [status, response_data] = a.get();
    std::cout << "Async completed with status: " << status << std::endl;
    Server.Response(connection, status, status == 200 ? "Ok" : "Error", response_data);
    return status;

  } catch (std::exception &e) {
    std::cerr << "Internal Error: " << e.what() << std::endl;
    Server.Response(connection, 500, "Error", R"({"error":"Internal Server Error"})");
    return 500;
  }
}
