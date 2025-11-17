#include "models.hpp"
#include "mysql.hpp"
#include <exception>
#include <iostream>
#include <middleware.hpp>
#include <phoenix.hpp>
#include <string>

route("/api/admin/jurusan/create",jurusan_create){
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
  auto a = Server.method.async([&](){
    try{
      auto db = MySQLPool::getInstance();
      auto cursor = db->get_connection();  
      Jurusan.kode = post_as_json.value("kode","");
      Jurusan.nama = post_as_json.value("nama","");
      Jurusan.deskripsi = post_as_json.value("deksripsi","");
      auto stmt = db->db_prep(cursor.get(),"INSERT INTO Jurusan(nama,kode,deskripsi) VALUES (?,?,?)");
      stmt->bind(Jurusan.nama)->bind(Jurusan.kode)->bind(Jurusan.deskripsi)->execute();
      delete stmt;
      Server.Response(connection,200,"Ok","");
      return 200;
    }catch(std::exception& e){
      std::cerr << e.what() << std::endl;
      Server.Response(connection, 500, "Error",
                      R"({"Message":"Internal Server Error"})");
      return 500;
    }
  });
  return a.get();
}

route("/api/admin/jurusan/read",jurusan_read){
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
  try{
    auto a = Server.method.async([&]()->std::pair<int,std::string>{
      auto db = MySQLPool::getInstance();
      auto cursor = db->get_connection();
      nlohmann::json query;

      try{
        query = db->db_select(cursor.get(),"SELECT * FROM Jurusan");
        Server.Response(connection,202,"Ok",query.dump());
        return {202,"Ok"};
      }catch(std::exception& e){
        Server.Response(connection,500,"Error",e.what());
        return {500,e.what()};
      }
    });
    auto [status,response] = a.get();
    return status;
  }catch(std::exception& e){
    Server.Response(connection,500,"OK",R"("Error":"On Async")");
    return 500;
  }
}

route("/api/admin/jurusan/delete",delete_jurusan){
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
  auto a = Server.method.async([&](){
    try{
      auto db = MySQLPool::getInstance();
      auto cursor = db->get_connection();  
      Jurusan.id = post_as_json.value("id",0);
      auto stmt = db->db_prep(cursor.get(),"DELETE FROM Jurusan WHERE id=?");
      stmt->bind(Jurusan.id)->execute();
      delete stmt;
      Server.Response(connection,200,"Ok","");
      return 200;
    }catch(std::exception& e){
      std::cerr << e.what() << std::endl;
      Server.Response(connection, 500, "Error",
                      R"({"Message":"Internal Server Error"})");
      return 500;
    }
  });
  return a.get();
}
