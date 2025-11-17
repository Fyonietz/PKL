#include "models.hpp"
#include "mysql.hpp"
#include <ctype.h>
#include <exception>
#include <iostream>
#include <middleware.hpp>
#include <phoenix.hpp>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
struct upload_ctx {
  char path[256];
  char token[6]; // 5-digit token + null terminator
};

// ----------------- Form callbacks -----------------
static int field_found_cb(const char *key, const char *filename, char *path,
                          size_t pathlen, void *user_data) {
  struct upload_ctx *ctx = (struct upload_ctx *)user_data;

  if (!filename || filename[0] == '\0') {
    // No filename -> skip
    return MG_FORM_FIELD_STORAGE_SKIP;
  }

  // Only accept PNG
  const char *ext = strrchr(filename, '.');
  if (!ext || strcasecmp(ext, ".png") != 0) {
    return MG_FORM_FIELD_STORAGE_SKIP;
  }

  // Ensure folder exists
  mkdir("main/uploads", 0755);

  // Build path using backend token
  snprintf(ctx->path, sizeof(ctx->path), "main/public/uploads/%s.png", ctx->token);
  snprintf(path, pathlen, "%s", ctx->path);

  return MG_FORM_FIELD_STORAGE_STORE;
}

static int field_store_cb(const char *path, long long file_size,
                          void *user_data) {
  return MG_FORM_FIELD_HANDLE_NEXT;
}

static int field_get_cb(const char *key, const char *value, size_t valuelen,
                        void *user_data) {
  return MG_FORM_FIELD_HANDLE_NEXT;
}
// Structure to hold company form data
struct PerusahaanFormData {
  std::string name;
  std::string bio;
  std::string about;
  std::string address;
  std::string phone;
  std::string email;
  std::string image_url;
  std::vector<int> jurusan_ids;
  std::vector<std::string> benefits;
  std::vector<std::string> syarat;
  int kuota;
  std::string kuota_keterangan;
};

// Parse JSON form data
PerusahaanFormData parse_company_form(const json &body) {
  PerusahaanFormData data;

  data.name = body.value("name", "");
  data.bio = body.value("bio", "");
  data.about = body.value("about", "");
  data.address = body.value("address", "");
  data.phone = body.value("phone", "");
  data.email = body.value("email", "");
  data.image_url = body.value("image_url", "");

  // Parse jurusan_ids array
  if (body.contains("jurusan_ids") && body["jurusan_ids"].is_array()) {
    for (const auto &id : body["jurusan_ids"]) {
      data.jurusan_ids.push_back(id.get<int>());
    }
  }

  // Parse benefits array
  if (body.contains("benefits") && body["benefits"].is_array()) {
    for (const auto &benefit : body["benefits"]) {
      data.benefits.push_back(benefit.get<std::string>());
    }
  }

  // Parse syarat array
  if (body.contains("syarat") && body["syarat"].is_array()) {
    for (const auto &s : body["syarat"]) {
      data.syarat.push_back(s.get<std::string>());
    }
  }

  data.kuota = body.value("kuota", 0);
  data.kuota_keterangan = body.value("kuota_keterangan", "");

  return data;
}

json insert_company(MYSQL *conn, const PerusahaanFormData &data) {
  MySQLPool *pool = MySQLPool::getInstance();
  json response;

  try {
    // Start transaction
    pool->db_query(conn, "START TRANSACTION");

    // 1. Insert main company data
    std::string insert_company_sql =
        "INSERT INTO Perusahaan (nama, bio, deskripsi, alamat, phone, email, "
        "image_url, benefit, jurusan_id) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, '', ?)";

    PreparedStatement *stmt = pool->db_prep(conn, insert_company_sql);

    // Use the first jurusan_id as default for backward compatibility
    int default_jurusan = data.jurusan_ids.empty() ? 8 : data.jurusan_ids[0];

    stmt->bind(data.name)
        ->bind(data.bio)
        ->bind(data.about)
        ->bind(data.address)
        ->bind(data.phone)
        ->bind(data.email)
        ->bind(data.image_url)
        ->bind(default_jurusan);

    if (!stmt->execute()) {
      delete stmt;
      pool->db_query(conn, "ROLLBACK");
      response["success"] = false;
      response["error"] = "Failed to insert company";
      return response;
    }

    unsigned long long perusahaan_id = stmt->insert_id();
    delete stmt;

    // 2. Insert jurusan relationships
    if (!data.jurusan_ids.empty()) {
      std::string insert_jurusan_sql =
          "INSERT INTO Perusahaan_Jurusan (perusahaan_id, jurusan_id) VALUES "
          "(?, ?)";

      for (int jurusan_id : data.jurusan_ids) {
        PreparedStatement *jur_stmt = pool->db_prep(conn, insert_jurusan_sql);
        jur_stmt->bind((long long)perusahaan_id)->bind(jurusan_id);

        if (!jur_stmt->execute()) {
          delete jur_stmt;
          pool->db_query(conn, "ROLLBACK");
          response["success"] = false;
          response["error"] = "Failed to insert jurusan";
          return response;
        }
        delete jur_stmt;
      }
    }

    // 3. Insert benefits
    if (!data.benefits.empty()) {
      std::string insert_benefit_sql =
          "INSERT INTO Perusahaan_Benefit (perusahaan_id, benefit, urutan) "
          "VALUES (?, ?, ?)";

      for (size_t i = 0; i < data.benefits.size(); i++) {
        PreparedStatement *ben_stmt = pool->db_prep(conn, insert_benefit_sql);
        ben_stmt->bind((long long)perusahaan_id)
            ->bind(data.benefits[i])
            ->bind((int)i + 1);

        if (!ben_stmt->execute()) {
          delete ben_stmt;
          pool->db_query(conn, "ROLLBACK");
          response["success"] = false;
          response["error"] = "Failed to insert benefit";
          return response;
        }
        delete ben_stmt;
      }
    }

    // 4. Insert kuota
    if (data.kuota > 0) {
      std::string insert_kuota_sql =
          "INSERT INTO Perusahaan_Kuota (perusahaan_id, jumlah, keterangan) "
          "VALUES (?, ?, ?)";

      PreparedStatement *kuota_stmt = pool->db_prep(conn, insert_kuota_sql);
      kuota_stmt->bind((long long)perusahaan_id)
          ->bind(data.kuota)
          ->bind(data.kuota_keterangan);

      if (!kuota_stmt->execute()) {
        delete kuota_stmt;
        pool->db_query(conn, "ROLLBACK");
        response["success"] = false;
        response["error"] = "Failed to insert kuota";
        return response;
      }
      delete kuota_stmt;
    }

    // 5. Insert syarat
    if (!data.syarat.empty()) {
      std::string insert_syarat_sql =
          "INSERT INTO Perusahaan_Syarat (perusahaan_id, syarat, urutan) "
          "VALUES (?, ?, ?)";

      for (size_t i = 0; i < data.syarat.size(); i++) {
        PreparedStatement *syarat_stmt = pool->db_prep(conn, insert_syarat_sql);
        syarat_stmt->bind((long long)perusahaan_id)
            ->bind(data.syarat[i])
            ->bind((int)i + 1);

        if (!syarat_stmt->execute()) {
          delete syarat_stmt;
          pool->db_query(conn, "ROLLBACK");
          response["success"] = false;
          response["error"] = "Failed to insert syarat";
          return response;
        }
        delete syarat_stmt;
      }
    }

    // Commit transaction
    pool->db_query(conn, "COMMIT");

    response["success"] = true;
    response["id"] = perusahaan_id;
    response["message"] = "Company added successfully";

  } catch (const std::exception &e) {
    pool->db_query(conn, "ROLLBACK");
    response["success"] = false;
    response["error"] = std::string("Exception: ") + e.what();
  }

  return response;
}

// GET company by ID with all related data
json get_company_by_id(MYSQL *conn, int company_id) {
  MySQLPool *pool = MySQLPool::getInstance();
  json result;

  // 1. Get basic company info
  std::string basic_sql = "SELECT id, nama, bio, deskripsi, alamat, phone, "
                          "email, image_url, created_at "
                          "FROM Perusahaan WHERE id = ?";

  json company_data = pool->db_select_prep(
      conn, basic_sql,
      [company_id](PreparedStatement *stmt) { stmt->bind(company_id); });

  if (company_data.empty()) {
    return nullptr;
  }

  result = company_data[0];

  // 2. Get jurusan
  std::string jurusan_sql =
      "SELECT j.id, j.nama, j.kode FROM Perusahaan_Jurusan pj "
      "JOIN Jurusan j ON pj.jurusan_id = j.id "
      "WHERE pj.perusahaan_id = ? ORDER BY j.nama";

  result["jurusan"] = pool->db_select_prep(
      conn, jurusan_sql,
      [company_id](PreparedStatement *stmt) { stmt->bind(company_id); });

  // 3. Get benefits
  std::string benefit_sql = "SELECT benefit FROM Perusahaan_Benefit "
                            "WHERE perusahaan_id = ? ORDER BY urutan";

  result["benefits"] = pool->db_select_prep(
      conn, benefit_sql,
      [company_id](PreparedStatement *stmt) { stmt->bind(company_id); });

  // 4. Get syarat
  std::string syarat_sql = "SELECT syarat FROM Perusahaan_Syarat "
                           "WHERE perusahaan_id = ? ORDER BY urutan";

  result["syarat"] = pool->db_select_prep(
      conn, syarat_sql,
      [company_id](PreparedStatement *stmt) { stmt->bind(company_id); });

  // 5. Get kuota
  std::string kuota_sql = "SELECT jumlah, keterangan FROM Perusahaan_Kuota "
                          "WHERE perusahaan_id = ? LIMIT 1";

  json kuota_data = pool->db_select_prep(
      conn, kuota_sql,
      [company_id](PreparedStatement *stmt) { stmt->bind(company_id); });

  if (!kuota_data.empty()) {
    result["kuota"] = kuota_data[0];
  }

  return result;
}

// ----------------- Route handler -----------------
route("/api/admin/perusahaan/create", perusahaan_create) {
  auto authInfo = CheckAuthToken(connection, Auth::Roles::Admin);
  CORS(connection);

  if (!authInfo) {
    Server.Response(connection, 401, "Unauthorized",
                    R"({"error":"Unauthorized"})");
    return 401;
  }

  if (!ASYNC) {
    Server.Response(connection, 500, "Error",
                    R"({"Message":"ASYNC mode Error"})");
    return 500;
  }
  
  std::string post = Server.Read(connection);
  std::cout << "Raw POST data: " << post << std::endl;
  
  nlohmann::json post_as_json;
  try {
    post_as_json = nlohmann::json::parse(post);
    std::cout << "Parsed JSON: " << post_as_json << std::endl;
  } catch (const nlohmann::json::exception& e) {
    std::cerr << "JSON parse error: " << e.what() << std::endl;
    Server.Response(connection, 400, "Bad Request", 
                    R"({"error":"Invalid JSON"})");
    return 400;
  }

  Server.method.async([&]() {
    try {
      // Validate required fields
      if (!post_as_json.contains("name") || !post_as_json["name"].is_string()) {
        Server.Response(connection, 400, "Bad Request", 
                        R"({"error":"Missing or invalid name field"})");
        return 400;
      }

      std::string company_name = post_as_json["name"];
      
      struct upload_ctx ctx;
      memset(&ctx, 0, sizeof(ctx));

      // Generate 5-digit token from backend logic
      std::string recipe = company_name + "batak";
      std::string token = Auth::tokenizer(recipe);
      std::string token_short = token.substr(0, 5);
      snprintf(ctx.token, sizeof(ctx.token), "%s", token_short.c_str());

      // Setup form handler
      struct mg_form_data_handler fdh = {.field_found = field_found_cb,
                                         .field_get = field_get_cb,
                                         .field_store = field_store_cb,
                                         .user_data = &ctx};
      mg_handle_form_request(connection, &fdh);
      
      auto db = MySQLPool::getInstance();
      auto cursor = db->get_connection();
      
      PerusahaanFormData data = parse_company_form(post_as_json);
data.image_url = "main/public/uploads/"+token_short+".png";
      
      auto response = insert_company(cursor.get(), data);
      std::cout << "Database response: " << response << std::endl;

      if (response["success"] == true) {
        std::string json_response = "{\"success\":true,\"id\":" + 
                                   std::to_string(response["id"].get<int>()) + 
                                   ",\"message\":\"Company created successfully\"}";
        Server.Response(connection, 200, "OK", json_response.c_str());
      } else {
        std::string error_msg = "{\"success\":false,\"error\":\"" + 
                               response["error"].get<std::string>() + "\"}";
        Server.Response(connection, 500, "Error", error_msg.c_str());
      }
      return 200;
      
    } catch (std::exception &e) {
      std::cerr << "Exception in async handler: " << e.what() << std::endl;
      Server.Response(connection, 500, "Internal Server Error", 
                      R"({"error":"Failed to process request"})");
      return 500;
    }
  });
  return 200;
}
