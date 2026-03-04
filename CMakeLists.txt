cmake_minimum_required(VERSION 3.20)
project(lisboa-pathfinder)

set(CMAKE_CXX_STANDARD 17)

find_package(protobuf CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)


add_executable(lisboa 
    src/proto/fileformat.pb.cc
    src/proto/osmformat.pb.cc
    src/io/nominatin.cpp
    src/routing/astar.cpp
    src/io/parser.cpp
    src/ui/utils.cpp
    src/main.cpp
)

target_include_directories(lisboa PRIVATE 
    src
    src/core
    src/routing
    src/io
    src/ui
)


target_link_libraries(lisboa PRIVATE protobuf::libprotobuf ZLIB::ZLIB CURL::libcurl)