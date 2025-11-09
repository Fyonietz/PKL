#include "models.hpp"
#include "mysql.hpp"
#include <exception>
#include <phoenix.hpp>
#include <middleware.hpp>
#include <iostream>

route("/api/admin/user/create",create_user){
  std::string post = Server.Read(connection);
  CORS(connection);
  try{
    auto db = MySQLPool::getInstance();
    auto conn = db->get_connection();
    nlohmann::json post_as_json = nlohmann::json::parse(post);
    //Req User/Admin/Pembimbing
    Users.nama = post_as_json["nama"];
    Users.password = post_as_json["password"];
    Users.rolesId = post_as_json["role"].get<int>();

    //Req Siswa/Kaprodi
    Users.kelasId = post_as_json["kelas"].empty() ? 0 :post_as_json["kelas"].get<int>();
    Users.jurusanId = post_as_json["jurusan"].empty() ? 0 :post_as_json["jurusan"].get<int>();
    Users.namaPembimbing = post_as_json["pembimbing"].empty() ? "" : post_as_json["pembimbing"];
    Users.tempatPerusahaanId = post_as_json["pembimbing"].empty() ? 0 : post_as_json["pembimbing"].get<int>();
 

    //Req Pembimbing
    std::string recipe= Users.nama + Users.password + "akankupakai";
    std::string token = Auth::tokenizer(std::move(recipe));
   Server.Response(connection, 200, "Ok", R"({"Message":")" + token + R"("})");

    return 200;
;
  }
  catch(std::exception &e){
    std::cerr << e.what() << std::endl;
    return 500;
  }
}
