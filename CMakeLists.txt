cmake_minimum_required(VERSION 3.19)
project(webserv)

set(CMAKE_CXX_STANDARD 98)

include_directories(config_parser)

add_executable(webserv
        main.cpp
        config_parser/ConfigReader.cpp
        config_parser/ConfigReader.h)
