#include "civetweb.h"
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

// ----------------- Structures -----------------
struct upload_ctx {
  char path[512];
  char token[6]; // 5-digit token + null terminator
};

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

// ----------------- Form Upload Callbacks -----------------
static int field_found_cb(const char *key, const char *filename, char *path,
                          size_t pathlen, void *user_data) {
  struct upload_ctx *ctx = (struct upload_ctx *)user_data;

  if (!filename || filename[0] == '\0') {
    return MG_FORM_FIELD_STORAGE_SKIP;
  }

  // Only accept PNG
  const char *ext = strrchr(filename, '.');
  if (!ext || strcasecmp(ext, ".png") != 0) {
    std::cerr << "Rejected file: " << filename << " (not PNG)" << std::endl;
    return MG_FORM_FIELD_STORAGE_SKIP;
  }

  // Ensure folder exists
  mkdir("main", 0755);
  mkdir("main/public", 0755);
  mkdir("main/public/uploads", 0755);

  // Build path using token
  snprintf(ctx->path, sizeof(ctx->path), "main/public/uploads/%s.png", ctx->token);
  snprintf(path, pathlen, "%s", ctx->path);

  std::cout << "Saving file to: " << ctx->path << std::endl;

  return MG_FORM_FIELD_STORAGE_STORE;
}

static int field_store_cb(const char *path, long long file_size,
                          void *user_data) {
  std::cout << "File stored: " << path << " (size: " << file_size << " bytes)" << std::endl;
  return MG_FORM_FIELD_HANDLE_NEXT;
}

static int field_get_cb(const char *key, const char *value, size_t valuelen,
                        void *user_data) {
  return MG_FORM_FIELD_HANDLE_NEXT;
}

// ----------------- Helper Functions -----------------

// Generate image URL from company name
std::string get_company_image_url(const std::string& company_name) {
  std::string recipe = company_name + "batak";
  std::string token = Auth::tokenizer(recipe);
  std::string token_short = token.substr(0, 5);
  return "/uploads/" + token_short + ".png";
}

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

// Insert company with all related data
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
  
  // If image_url is empty, generate it from company name
  if (!result.contains("image_url") || result["image_url"].get<std::string>().empty()) {
    std::string company_name = result["nama"].get<std::string>();
    result["image_url"] = get_company_image_url(company_name);
  }

  // 2. Get jurusan
  std::string jurusan_sql =
      "SELECT j.id, j.nama, j.kode FROM Perusahaan_Jurusan pj "
      "JOIN Jurusan j ON pj.jurusan_id = j.id "
      "WHERE pj.perusahaan_id = ? ORDER BY j.nama";

  json jurusan_list = pool->db_select_prep(
      conn, jurusan_sql,
      [company_id](PreparedStatement *stmt) { stmt->bind(company_id); });

  result["jurusan"] = json::array();
  for (const auto& j : jurusan_list) {
    result["jurusan"].push_back(j["nama"].get<std::string>());
  }

  // 3. Get benefits
  std::string benefit_sql = "SELECT benefit FROM Perusahaan_Benefit "
                            "WHERE perusahaan_id = ? ORDER BY urutan";

  json benefit_list = pool->db_select_prep(
      conn, benefit_sql,
      [company_id](PreparedStatement *stmt) { stmt->bind(company_id); });

  result["benefit"] = json::array();
  for (const auto& b : benefit_list) {
    result["benefit"].push_back(b["benefit"].get<std::string>());
  }

  // 4. Get syarat
  std::string syarat_sql = "SELECT syarat FROM Perusahaan_Syarat "
                           "WHERE perusahaan_id = ? ORDER BY urutan";

  json syarat_list = pool->db_select_prep(
      conn, syarat_sql,
      [company_id](PreparedStatement *stmt) { stmt->bind(company_id); });

  result["syarat"] = json::array();
  for (const auto& s : syarat_list) {
    result["syarat"].push_back(s["syarat"].get<std::string>());
  }

  // 5. Get kuota
  std::string kuota_sql = "SELECT jumlah, keterangan FROM Perusahaan_Kuota "
                          "WHERE perusahaan_id = ? LIMIT 1";

  json kuota_data = pool->db_select_prep(
      conn, kuota_sql,
      [company_id](PreparedStatement *stmt) { stmt->bind(company_id); });

  if (!kuota_data.empty()) {
    result["kuota"] = std::stoi(kuota_data[0]["jumlah"].get<std::string>());
  } else {
    result["kuota"] = 0;
  }

  // Add default fields
  result["bidang"] = "IT & Software";
  result["gambar"] = result["image_url"];
  result["alamat"] = result.value("alamat", "");

  return result;
}

// ----------------- Route Handlers -----------------

// Create company (JSON only)
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
      
      // Generate token from company name
      std::string recipe = company_name + "batak";
      std::string token = Auth::tokenizer(recipe);
      std::string token_short = token.substr(0, 5);
      
      std::cout << "Company: " << company_name << " -> Token: " << token_short << std::endl;

      auto db = MySQLPool::getInstance();
      auto cursor = db->get_connection();
      
      PerusahaanFormData data = parse_company_form(post_as_json);
      
      // Set image URL (web accessible path)
      data.image_url = "/uploads/" + token_short + ".png";
      
      std::cout << "Image URL: " << data.image_url << std::endl;
      
      auto response = insert_company(cursor.get(), data);
      std::cout << "Database response: " << response << std::endl;

      if (response["success"] == true) {
        std::string json_response = "{\"success\":true,\"id\":" + 
                                   std::to_string(response["id"].get<unsigned long long>()) + 
                                   ",\"token\":\"" + token_short + "\"" +
                                   ",\"image_url\":\"" + data.image_url + "\"" +
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

// Upload image for company
route("/api/admin/perusahaan/upload-image", upload_company_image) {
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

  Server.method.async([&]() {
    try {
      // Get company_name from query string
      const struct mg_request_info *ri = mg_get_request_info(connection);
      char company_name_buf[256] = {0};
      
      mg_get_var(ri->query_string, strlen(ri->query_string ? ri->query_string : ""),
                 "company_name", company_name_buf, sizeof(company_name_buf));
      
      if (company_name_buf[0] == '\0') {
        Server.Response(connection, 400, "Bad Request",
                        R"({"error":"Missing company_name parameter"})");
        return 400;
      }

      std::string company_name(company_name_buf);
      
      // Generate token from company name (same as creation)
      std::string recipe = company_name + "batak";
      std::string token = Auth::tokenizer(recipe);
      std::string token_short = token.substr(0, 5);

      struct upload_ctx ctx;
      memset(&ctx, 0, sizeof(ctx));
      snprintf(ctx.token, sizeof(ctx.token), "%s", token_short.c_str());

      std::cout << "Uploading image for: " << company_name 
                << " -> Token: " << token_short << std::endl;

      // Setup form handler
      struct mg_form_data_handler fdh = {
          .field_found = field_found_cb,
          .field_get = field_get_cb,
          .field_store = field_store_cb,
          .user_data = &ctx
      };

      int result = mg_handle_form_request(connection, &fdh);

      if (result < 0) {
        Server.Response(connection, 400, "Bad Request",
                        R"({"error":"File upload failed"})");
        return 400;
      }

      // Verify file was created
      struct stat st;
      if (stat(ctx.path, &st) != 0) {
        std::cerr << "File not found after upload: " << ctx.path << std::endl;
        Server.Response(connection, 500, "Internal Server Error",
                        R"({"error":"File upload verification failed"})");
        return 500;
      }

      std::cout << "File uploaded successfully: " << ctx.path 
                << " (size: " << st.st_size << " bytes)" << std::endl;

      // Return the web-accessible URL
      std::string image_url = "/uploads/" + token_short + ".png";
      std::string json_response = "{\"success\":true,\"token\":\"" + token_short +
                                 "\",\"image_url\":\"" + image_url + 
                                 "\",\"file_path\":\"" + std::string(ctx.path) + "\"}";

      Server.Response(connection, 200, "OK", json_response.c_str());
      return 200;

    } catch (std::exception &e) {
      std::cerr << "Upload exception: " << e.what() << std::endl;
      Server.Response(connection, 500, "Internal Server Error",
                      R"({"error":"Upload failed"})");
      return 500;
    }
  });
  return 200;
}

// Get company by ID
route("/api/perusahaan/", get_company_detail) { 
  CORS(connection);
  const struct mg_request_info *req_info = mg_get_request_info(connection);
  std::string uri = req_info->request_uri;
  // CORS(connection);
  std::string get_id = uri.substr(16);
  std::cout <<"Located: " << get_id << std::endl;
  if (!ASYNC) {
    Server.Response(connection, 500, "Error",
                    R"({"Message":"ASYNC mode Error"})");
    return 500;
  }

  Server.method.async([&]() {
    try {
      auto db = MySQLPool::getInstance();
      auto cursor = db->get_connection();

      // Get company ID from URL params

      json company = get_company_by_id(cursor.get(), std::stoi(get_id));

      if (company.is_null()) {
        Server.Response(connection, 404, "Not Found",
                        R"({"error":"Company not found"})");
        return 404;
      }

      std::string json_response = company.dump();
      Server.Response(connection, 200, "OK", json_response.c_str());
      return 200;

    } catch (std::exception &e) {
      std::cerr << "Exception: " << e.what() << std::endl;
      Server.Response(connection, 500, "Internal Server Error",
                      R"({"error":"Failed to fetch company"})");
      return 500;
    }
  });
  return 200;
}

// Get all companies
route("/api/companies", get_companies_list) {
  CORS(connection);

  if (!ASYNC) {
    Server.Response(connection, 500, "Error",
                    R"({"Message":"ASYNC mode Error"})");
    return 500;
  }

  Server.method.async([&]() {
    try {
      auto db = MySQLPool::getInstance();
      auto cursor = db->get_connection();

      std::string sql = 
          "SELECT p.id, p.nama, p.bio, p.alamat, p.phone, p.email, p.image_url, "
          "GROUP_CONCAT(DISTINCT j.nama SEPARATOR ', ') as jurusan_list "
          "FROM Perusahaan p "
          "LEFT JOIN Perusahaan_Jurusan pj ON p.id = pj.perusahaan_id "
          "LEFT JOIN Jurusan j ON pj.jurusan_id = j.id "
          "GROUP BY p.id ORDER BY p.created_at DESC";

      json companies = db->db_select(cursor.get(), sql.c_str());
      
      // Ensure each company has an image_url
      for (auto& company : companies) {
        if (!company.contains("image_url") || company["image_url"].get<std::string>().empty()) {
          std::string company_name = company["nama"].get<std::string>();
          company["image_url"] = get_company_image_url(company_name);
        }
      }

      std::string json_response = companies.dump();
      Server.Response(connection, 200, "OK", json_response.c_str());
      return 200;

    } catch (std::exception &e) {
      std::cerr << "Exception: " << e.what() << std::endl;
      Server.Response(connection, 500, "Internal Server Error",
                      R"({"error":"Failed to fetch companies"})");
      return 500;
    }
  });
  return 200;
}


