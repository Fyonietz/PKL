#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#include <string>

// Declare the structs (no need to define them here, just declare them)
struct modelRoles {
  int id;
  std::string nama;
};

struct modelKelas {
  int id;
  std::string nama;
};

struct modelJurusan {
  int id;
  std::string nama;
};

struct modelPerusahaan {
  int id;
  std::string nama;
  std::string alamat;
  std::string benefit;
  int jurusanId;
};

struct modelUsers {
  int id;
  std::string nama;
  std::string password;
  std::string token;
  int rolesId;
  int kelasId;
  int jurusanId;
  std::string namaPembimbing;
  int tempatPerusahaanId;
  std::string siswaDibimbing;
  int jurusanKaprodiId;
  int jurusanPerusahaanId;
};

// Declare the global variables as extern
extern modelRoles Roles;
extern modelKelas Kelas;
extern modelJurusan Jurusan;
extern modelPerusahaan Perusahaan;
extern modelUsers Users;

#endif // GLOBAL_VARS_H
