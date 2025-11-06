CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
project(PKL-Phoenix)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Collect source files
file(GLOB_RECURSE MAIN "core/core.cpp")
file(GLOB_RECURSE PHOENIX "core/libs/phoenix/*.cpp")
file(GLOB_RECURSE SERVER "core/libs/Civet/civetweb.c")
file(GLOB_RECURSE SERVER_INL "core/libs/Civet/*.inl")
list(APPEND PHOENIX "core/phoenix/engine.cpp")
add_subdirectory(core/handler)

# Find OpenSSL package
find_package(OpenSSL REQUIRED)

# Manually set the paths for MariaDB/MySQL include and lib directories
set(MYSQL_INCLUDE_DIR "/usr/include/mysql")  # Path to MariaDB headers
set(MYSQL_LIBRARIES "/usr/lib/libmysqlclient.so")  # Path to MariaDB client library

# Create executable from the sources
add_executable(${CMAKE_PROJECT_NAME} ${MAIN} ${PHOENIX} ${SERVER} ${SERVER_INL})

# Include directories
target_include_directories(${CMAKE_PROJECT_NAME}
  PUBLIC ${CMAKE_SOURCE_DIR}/core/handler/
  PUBLIC ${CMAKE_SOURCE_DIR}/core/libs/
  PUBLIC ${CMAKE_SOURCE_DIR}/core/phoenix/
  PRIVATE ${OPENSSL_INCLUDE_DIR}  # OpenSSL includes
  PRIVATE ${MYSQL_INCLUDE_DIR}    # MariaDB includes
)

# Link libraries (MariaDB/MySQL, OpenSSL)
target_link_libraries(${CMAKE_PROJECT_NAME}
  PRIVATE OpenSSL::SSL
  PRIVATE OpenSSL::Crypto
  PRIVATE ${MYSQL_LIBRARIES}  # Link MariaDB MySQL client library
)

# Conditional compilation based on platform
if (WIN32)
    add_definitions(-DNO_SSL)
else()
    add_definitions(-DNO_SSL -DLINUX)
    # Export symbols for dynamic shared libraries
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE "-Wl,--export-dynamic")
endif()



