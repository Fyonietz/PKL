#ifndef MIDDLEWARE
#define MIDDLEWARE
#include <iomanip>
#include <openssl/evp.h>
#include <vector>
struct Auth{
  enum class Roles{
    None = 0,
    Admin = 1<<0,
    Siswa = 1<<1,
    Pembimbing=1<<2,
    Kaprodi=1<<3,
    Perusahaan=1<<4,
    Unknown=1<<5
  };
  static Roles strToRole(const std::string &user);
  static std::string roleToStr(Roles roles);
  static std::string tokenizer(const std::string &input);

  // Helper to convert bitmask to vector of individual roles
  static std::vector<Roles> bitmaskToRoles(Roles roleMask);

  // Check if a role is contained in a bitmask
  static bool hasRole(Roles userRole, Roles requiredRoleMask);
};

// Free functions for bitwise operations (must be non-member)
inline Auth::Roles operator|(Auth::Roles lhs, Auth::Roles rhs) {
  return static_cast<Auth::Roles>(
      static_cast<std::underlying_type_t<Auth::Roles>>(lhs) |
      static_cast<std::underlying_type_t<Auth::Roles>>(rhs));
}

inline Auth::Roles operator&(Auth::Roles lhs, Auth::Roles rhs) {
  return static_cast<Auth::Roles>(
      static_cast<std::underlying_type_t<Auth::Roles>>(lhs) &
      static_cast<std::underlying_type_t<Auth::Roles>>(rhs));
}



#endif // !MIDDLEWARE

