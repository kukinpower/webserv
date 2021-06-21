cmake_minimum_required(VERSION 3.17.3)
project(webserv)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_FLAGS "-g")

include_directories(config_parser)
include_directories(webserver)
include_directories(logger)
include_directories(client)
include_directories(exception)
include_directories(request)
include_directories(response)
include_directories(server)
include_directories(cgi_handler)

add_executable(webserv
        main.cpp)

set_target_properties(webserv PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ../
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ../)
