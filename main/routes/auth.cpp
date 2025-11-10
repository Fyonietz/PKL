#include <exception>
#include <middleware.hpp>
#include <optional>
#include <phoenix.hpp>
#include <iostream>

std::optional<nlohmann::json> CheckAuthToken(struct mg_connection *conn, Auth::Roles requiredRoles) {
    const char *auth_header = mg_get_header(conn, "Authorization");
    
    if (!auth_header) {
        std::cout << "No Authorization header found" << std::endl;
        return std::nullopt;
    }

    std::string auth_str = auth_header;
    std::string bearer_prefix = "Bearer ";
    
    if (auth_str.find(bearer_prefix) != 0) {
        std::cout << "Invalid Authorization format" << std::endl;
        return std::nullopt;
    }

    std::string token = auth_str.substr(bearer_prefix.length());

    try {
        auto db = MySQLPool::getInstance();
        auto db_conn = db->get_connection();

        char escaped_token[token.length() * 2 + 1];
        mysql_real_escape_string(db_conn.get(), escaped_token, token.c_str(), token.length());
        
        std::string query_builder = "SELECT r.Nama as role FROM Users u "
                                    "LEFT JOIN Roles r ON u.Roles = r.id "
                                    "WHERE u.token = '" + std::string(escaped_token) + "'";

        
        nlohmann::json result = db->db_select(db_conn.get(), query_builder.c_str());
        
        if (!result.is_array() || result.empty()) {
            std::cout << "No user found with this token" << std::endl;
            return std::nullopt;
        }

        std::string roleStr = result[0]["role"];
        
        Auth::Roles userRole = Auth::strToRole(roleStr);

        if (!Auth::hasRole(userRole, requiredRoles)) {
            std::cout << "User role doesn't have required permissions" << std::endl;
            return std::nullopt;
        }

        return result[0];

    } catch (const std::exception &e) {
        std::cerr << "Error On AUTH: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::string GetAuthToken(struct mg_connection *connection) {
  const char *auth_header = mg_get_header(connection, "Authorization");
  if (!auth_header)
    return {};

  std::string auth_str(auth_header);
  const std::string bearer_prefix = "Bearer";

  // Find where "Bearer" starts (case-insensitive)
  size_t bearer_pos = auth_str.find(bearer_prefix);
  if (bearer_pos == std::string::npos)
    return {};

  // Skip "Bearer" and any whitespace
  size_t token_start = bearer_pos + bearer_prefix.length();
  
  // Skip whitespace after "Bearer"
  while (token_start < auth_str.length() && std::isspace(auth_str[token_start])) {
    token_start++;
  }

  if (token_start >= auth_str.length())
    return {};

  // Extract the token
  std::string token = auth_str.substr(token_start);
  
  // Trim any trailing whitespace
  size_t end = token.find_last_not_of(" \t\r\n");
  if (end != std::string::npos) {
    token = token.substr(0, end + 1);
  }

  return token;
}
