#include <exception>
#include <middleware.hpp>
#include <optional>
#include <phoenix.hpp>
#include <iostream>

std::optional<nlohmann::json> CheckAuthToken(struct mg_connection *conn, Auth::Roles requiredRoles) {
    const char *cookie_header = mg_get_header(conn, "Cookie");
    if (!cookie_header)
        return std::nullopt;

    std::string cookies(cookie_header);
    std::string tokenKey = "auth_token=";
    size_t tokenPos = cookies.find(tokenKey);
    if (tokenPos == std::string::npos)
        return std::nullopt;

    size_t start = tokenPos + tokenKey.length();
    size_t end = cookies.find(";", start);
    std::string token = cookies.substr(
        start, (end == std::string::npos) ? std::string::npos : end - start);

    try {
        auto db = MySQLPool::getInstance();
        auto db_conn = db->get_connection();

        char escaped_token[token.length() * 2 + 1];
        mysql_real_escape_string(db_conn.get(), escaped_token, token.c_str(), token.length());
        
        std::string query_builder = "SELECT r.Nama as role FROM Users u "
                                    "LEFT JOIN Roles r ON u.Roles = r.id "
                                    "WHERE u.token = '" + std::string(escaped_token) + "'";

        nlohmann::json result = db->db_select(db_conn.get(), query_builder.c_str());
        if (!result.is_array() || result.empty())
            return std::nullopt;

        std::string roleStr = result[0]["role"];
        Auth::Roles userRole = Auth::strToRole(roleStr);

        if (!Auth::hasRole(userRole, requiredRoles))
            return std::nullopt;

        return result[0];

    } catch (const std::exception &e) {
        std::cerr << "Error On AUTH: " << e.what() << std::endl;
        return std::nullopt;
    }
}
std::string GetAuthToken(struct mg_connection *connection) {
  const char *cookie_header = mg_get_header(connection, "Cookie");
  if (!cookie_header)
    return {};

  std::string_view cookies(cookie_header);
  constexpr std::string_view tokenKey = "auth_token=";

  size_t tokenPos = cookies.find(tokenKey);
  if (tokenPos == std::string_view::npos)
    return {};

  size_t start = tokenPos + tokenKey.size();
  size_t end = cookies.find(';', start);

  std::string token(cookies.substr(start, end - start));
  return std::move(token);
}
