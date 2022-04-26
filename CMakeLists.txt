cmake_minimum_required(VERSION 3.21)
project(Mysql)

set(CMAKE_CXX_STANDARD 14)

set(FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR /usr/local/mysql-connector-c++-8.0.28)
include_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/include)
link_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/lib64)

add_executable(Mysql main.cpp)

target_link_libraries(Mysql mysqlcppconn)
