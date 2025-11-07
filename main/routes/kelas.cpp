#include "middleware.hpp"
#include <exception>
#include <phoenix.hpp>
#include <iostream>
//
// route("/api/admin/kelas/create",kelas_create){
//   if(!Method(connection,"POST")){
//     return 405;
//   }
//   try{
//     // auto authInfo = CheckAuthToken()
//   }catch(std::exception& e){
//     std::cerr << e.what() << std::endl;
//     return 500;
//   }
// }
