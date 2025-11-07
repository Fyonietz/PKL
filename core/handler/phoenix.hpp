#include <route_register.hpp>
#include <mysql.hpp>
#include <models.hpp>
#include <middleware.hpp>
#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif
struct mg_connection;
EXPORT int default_handler(struct mg_connection *connection,
                           void * /*callbackdata*/);
EXPORT int home(struct mg_connection *connection, void *callback);

// Even simpler - no EXPORT in macro
#define route(PATH, NAME)                                                      \
  int NAME(struct mg_connection *connection, void *cb);                        \
  namespace {                                                                  \
  struct NAME##_Reg {                                                          \
    NAME##_Reg() { add_route(PATH, NAME); }                                    \
  } NAME##_instance;                                                           \
  }                                                                            \
  int NAME(struct mg_connection *connection, void *cb)

#define ASYNC (engine_running.load() && !shutdown_requested.load())
bool Method(struct mg_connection *connection,const char* define_method);
bool CORS(struct mg_connection *connection);
std::string Escape(const std::string& input);
std::optional<nlohmann::json> CheckAuthToken(
    struct mg_connection *conn,
    Auth::Roles requiredRoles = Auth::Roles::None);

std::string GetAuthToken(struct mg_connection *connection);
