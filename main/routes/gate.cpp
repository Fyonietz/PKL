#include "mysql.hpp"
#include <middleware.hpp>
#include <phoenix.hpp>
#include <iostream>

bool Method(struct mg_connection *connection,const char* define_method){
  const struct mg_request_info *request_info = mg_get_request_info(connection);
  const char *method = request_info->request_method;

    if (strcmp(method, define_method) != 0) {
        Server.Response(connection, 405, "Method Not Allowed", 
                        R"({"error": "Method Not Allowed"})");
        return false;
    }
  return true;
}

bool CORS(struct mg_connection *connection){
  const struct mg_request_info *request_info = mg_get_request_info(connection);
  const char *method = request_info->request_method;
 mg_printf(connection, "HTTP/1.1 200 OK\r\n");
  mg_printf(connection, "Content-Type: application/json\r\n");
    Server.CORS(connection, Server.env()["IP_CORS"]);
  if (!strcmp(method, "OPTIONS")) {
 Server.CORS_OPTIONS(connection);
    return true;
  }
  return true;
}

std::string Escape(const std::string& input) {
    std::ostringstream escaped;
    // Start with a leading quote
    escaped << '"';
    
    for (char ch : input) {
        if (ch == '"') {
            // Escape any existing double quotes
            escaped << '\\' << '"';
        } else {
            escaped << ch;
        }
    }

    // End with a trailing quote
    escaped << '"';
    return escaped.str();
}
route("/api/login", api_login) { 
  auto db = MySQLPool::getInstance();
  auto conn = db->get_connection();

   if (conn == nullptr) {
        std::cerr << "Failed to connect to the database!" << std::endl;
        return 500;
    }
  
  Server.Response(connection,200,"Ok",R"({"Messages":"Database Connected"})");
   return 200;
};


