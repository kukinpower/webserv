cmake_minimum_required(VERSION 3.19)
project(webserv)

set(CMAKE_CXX_STANDARD 98)

include_directories(config_parser)
include_directories(webserver)
include_directories(logger)
include_directories(client)

add_executable(webserv
        main.cpp)
