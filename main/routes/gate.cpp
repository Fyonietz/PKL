#include "models.hpp"
#include "mysql.hpp"
#include <exception>
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

    for (char ch : input) {
        if (ch == '\'') {
            // Escape single quote by doubling it up (SQL standard)
            escaped << "''";
        } else if (ch == '\\') {
            // Escape backslashes
            escaped << "\\\\";
        } else {
            escaped << ch;
        }
    }

    return escaped.str();
}


route("/api/login", api_login) { 
    std::string post = Server.Read(connection);
    CORS(connection);
    if(!ASYNC){
        Server.Response(connection, 500, "Async Error", R"({"Messages":"Error On Async"})");
        return 500;
    }

    // Launch async login task and return immediately
    Server.method.async([post, connection]() {
        try {
            auto post_as_json = nlohmann::json::parse(post, nullptr, false);
            if(post_as_json.is_discarded()){
                Server.Response(connection, 400, "Invalid JSON", R"({"error":"Invalid JSON"})");
                return;
            }

            std::string nama = post_as_json.value("nama", "");
            std::string password = post_as_json.value("password", "");
            auto db = MySQLPool::getInstance();
            auto conn = db->get_connection();

            std::string recipe = nama + password + "akankupakai";
            std::string token = Auth::tokenizer(recipe);

            std::string query = 
                "SELECT r.Nama as role FROM Users u "
                "LEFT JOIN Roles r ON u.roles_id = r.id "
                "WHERE token='" + token + "'";
            nlohmann::json result = db->db_select(conn.get(), query.c_str());
            if(!result.is_array() || result.empty()){
                Server.Response(connection, 404, "User Not Found", 
                                R"({"error":"User Not Found Please Register"})");
                return;
            }

            Auth::Roles user = Auth::strToRole(result[0].value("role", ""));

            static const std::unordered_map<Auth::Roles, std::string> role_redirect = {
                {Auth::Roles::Admin, "/admin"},
                {Auth::Roles::Kaprodi, "/kaprodi"},
                {Auth::Roles::Pembimbing, "/pembimbing"},
                {Auth::Roles::Perusahaan, "/perusahaan"},
                {Auth::Roles::Siswa, "/siswa"}
            };

            std::string redirect_url = Server.env()["IP_CORS"];
            redirect_url += role_redirect.count(user) ? role_redirect.at(user) : "/login";

            nlohmann::json response_json = {
                {"success", true},
                {"redirect", redirect_url},
                {"auth_token",token}
            };
            std::string response_str = response_json.dump();
            mg_printf(connection,
                      "HTTP/1.1 200 OK\r\n"
                      "Set-Cookie: auth_token=%s; Path=/; SameSite=Lax; Max-Age=3600\r\n"
                      "Content-Type: application/json\r\n"
                      "Connection: close\r\n"
                      "\r\n%s",
                      token.c_str(), response_str.c_str());

        } catch(const std::exception &e) {
            std::cerr << "Error On Login: " << e.what() << std::endl;
            Server.Response(connection, 500, "Internal Server Error", R"({"error":"Internal Error"})");
        }
    });

    return 200; // immediately return, server thread is free
}


