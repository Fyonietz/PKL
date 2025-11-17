#include "models.hpp"
#include "mysql.hpp"
#include <exception>
#include <iostream>
#include <middleware.hpp>
#include <phoenix.hpp>
#include <string>

route("/api/admin/kelas/create",kelas_create){
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
  std::cout << post_as_json.dump() << std::endl;
  auto a = Server.method.async([&](){
    try{
      auto db = MySQLPool::getInstance();
      auto cursor = db->get_connection();  
      Kelas.nama = post_as_json.value("nama","");
      auto stmt = db->db_prep(cursor.get(),"INSERT INTO Kelas(nama) VALUES (?)");
      stmt->bind(Kelas.nama)->execute();
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

route("/api/admin/kelas/read",kelas_read){
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
        query = db->db_select(cursor.get(),"SELECT * FROM Kelas");
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

route("/api/admin/kelas/delete",delete_kelas){
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);

  CORS(connection);

  if (!authInfo) {
    Server.Response(connection, 401, "Unauthorized",
                    R"({"error":"Unauthorized"})");
    return 401;
  }

  if (!ASYNC) {
    Server.Response(connection, 500, "Error",
                    R"({"Message":"ASYNC mode failed"})");
    return 500;
  }
 std::string post = Server.Read(connection);
  nlohmann::json post_as_json = nlohmann::json::parse(post,nullptr,false);
  auto a = Server.method.async([&](){
    try{
      auto db = MySQLPool::getInstance();
      auto cursor = db->get_connection();  
      Kelas.id = post_as_json.value("id",0);
      auto stmt = db->db_prep(cursor.get(),"DELETE FROM Kelas WHERE id=?");
      stmt->bind(Kelas.id)->execute();
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
