cmake_minimum_required(VERSION 3.21)
project(Mysql)

set(CMAKE_CXX_STANDARD 14)

set(FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR /opt/homebrew/Caskroom/miniforge/base/envs/MYSQLCPP)
include_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/include)
link_directories(${FULL_PATH_TO_MYSQL_CONNECTOR_CPP_DIR}/lib)

add_executable(Mysql main.cpp)

target_link_libraries(Mysql mysqlcppconn)
