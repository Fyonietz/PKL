#include <cstring>
#include <phoenix.hpp>
route("/api", api) {
  Server.Response(connection, 200, "OK",
                  R"({"Messages":"Hello From Phoenix"})");
  return 200;
};

route("/api/env", api_list) {
  std::string messages = R"({"Messages": "Hello From Env", "Env": ")" +
                         Server.env()["env"] + R"("})";
  if (ASYNC) {
    auto result = Server.method.async(
        [&]() { Server.Response(connection, 200, "OK", messages.c_str()); });

    result.get();
  }

  return 200;
};

route("/api/cors", cors) {
  const struct mg_request_info *request_info = mg_get_request_info(connection);
  const char *uri = request_info->request_uri;
  const char *method = request_info->request_method;

  mg_printf(connection, "HTTP/1.1 200 OK\r\n");
  mg_printf(connection, "Content-Type: text/plain\r\n");

  Server.CORS(connection, Server.env()["IP_CORS"]);
  if (!strcmp(method, "OPTIONS")) {
    return Server.CORS_OPTIONS(connection);
  }
  Server.Response(connection, 200, "Ok", R"({"Message":"Success"})");
  return 200;
}
