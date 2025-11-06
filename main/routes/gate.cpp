#include "mysql.hpp"
#include <middleware.hpp>
#include <phoenix.hpp>
#include <iostream>
route("/api/login", api_login) { 
  MySQL* db = MySQL::getInstance();
  MYSQL* conn = db->getConnection();

   if (conn == nullptr) {
        std::cerr << "Failed to connect to the database!" << std::endl;
        return 500;
    }
  
  Server.Response(connection,200,"Ok",R"({"Messages":"Database Connected"})");
   return 200;
};

route("/api/delete",api_delete){
  std::cout<< "Hello From Delete" << std::endl;
  return 200;
}
