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

  if (!ASYNC) {
    Server.Response(connection, 500, "Error",
                    R"({"Message":"ASYNC mode disabled"})");
    return 500;
  }
  std::string post = Server.Read(connection);
  nlohmann::json post_as_json = nlohmann::json::parse(post,nullptr,false);
  // Main logic runs when ASYNC is true
  auto a = Server.method.async([&]() {
    try {
      auto db = MySQLPool::getInstance();
      auto conn = db->get_connection();


      Users.id = post_as_json.value("Id", 0);
      Users.nama = post_as_json.value("Nama", "");
      Users.password = post_as_json.value("Password", "");
      Users.rolesId = post_as_json.value("Roles", 1);
      
      //Siswa and Pembimbing
      Users.kelasId = post_as_json.value("KelasId", 1);
      Users.jurusanId = post_as_json.value("JurusanId", 1);
      
      //Perusahaan Field 
      Perusahaan.alamat = post_as_json.value("perusahaan_alamat","");
      Perusahaan.benefit = post_as_json.value("perusahaan_benefit","");
      // Token creation
      std::string recipe = Users.nama + Users.password + "akankupakai";
      std::string token = Auth::tokenizer(std::move(recipe));
      
      //Query
      auto stmt1 = db->db_prep(conn.get(),"INSERT INTO Users(nama,password,token,roles_id) VALUE (?,?,?,?)" );
      stmt1->bind(1,Users.nama)->bind(2,Users.password)->bind(3,token)->bind(4,Users.rolesId)->execute();
      long long userId = stmt1->insert_id();
      delete stmt1;
      switch(Users.rolesId){
        case 3:{//Perusahaan
          auto stmt = db->db_prep(conn.get(),"INSERT INTO Perusahaan(id,benefit,alamat,jurusan_id) VALUES (?,?,?,?)");
          stmt->bind(1,userId)->bind(3,Perusahaan.alamat)->bind(2,Perusahaan.benefit)->bind(4,Users.jurusanId)->execute();
          delete stmt;
        break;
        }
        case 4:{//Pembimbing
          auto stmt = db->db_prep(conn.get(),"INSERT INTO Pembimbing(id,jurusan_id) VALUES (?,?)");
          stmt->bind(1,userId)->bind(2,Users.jurusanId)->execute();
          delete stmt;
        break;
        } 
        case 5:{//Siswa
          auto stmt = db->db_prep(conn.get(),"INSERT INTO Siswa(id,jurusan_id,kelas_id) VALUES (?,?,?)");
          stmt->bind(1,userId)->bind(2,Users.jurusanId)->bind(3,Users.kelasId)->execute();
          delete stmt;
        break;
        }
      }
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

  try {
    auto a = Server.method.async([&]() -> std::pair<int, std::string> {
      auto db = MySQLPool::getInstance();
      auto conn = db->get_connection();
      nlohmann::json query;

      try {
        query = db->db_select(
            conn.get(),
            R"(SELECT Users.id,Users.Nama,Users.Password,Roles.Nama as Role,Jurusan.nama As `Jurusan`,Kelas.nama As Kelas 
  FROM Users 
  LEFT JOIN Siswa ON `Users`.id = `Siswa`.id
  LEFT JOIN Roles ON Users.roles_id = Roles.id 
  LEFT JOIN Jurusan ON `Siswa`.jurusan_id=`Jurusan`.id
  LEFT JOIN `Kelas` ON `Siswa`.kelas_id = `Kelas`.id)");
      } catch (const std::exception &e) {
        std::cerr << "DB select error: " << e.what() << std::endl;
        return std::make_pair(400,
                              R"({"error":"Invalid JSON or DB query failed"})");
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
  } catch (const std::exception &e) {
    std::cerr << "Internal Error: " << e.what() << std::endl;
    Server.Response(connection, 500, "Error",
                    R"({"error":"Internal Server Error"})");
    return 500;
  }
}

route("/api/admin/user/delete", delete_user) {
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
  std::string raw_post = Server.Read(connection);
  try{
      nlohmann::json post = nlohmann::json::parse(raw_post,nullptr,false);
      Server.method.async([&](){
      auto db = MySQLPool::getInstance();
      auto cursor = db->get_connection();
      int id = post["id"].get<int>();
      try{
        auto query = db->db_prep(cursor.get(),"DELETE FROM Users WHERE id=?");
        query->bind(1,id)->execute();

        delete query;
        Server.Response(connection,202,"Ok",R"("Message":"User Deleted Succesfully")");
        return 200;
      }catch(const std::exception& e){
        std::cerr << "Error: " << e.what() << std::endl;
        Server.Response(connection,500,"Internal Server Error","");
        return 500;
        }

      });

  }catch(const std::exception &e){
    std::cerr << "Error: " << e.what() << std::endl;
    return 500;
  }
  return 200;
}
