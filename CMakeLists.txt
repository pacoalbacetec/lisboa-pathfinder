cmake_minimum_required(VERSION 3.20)
project(lisboa-pathfinder)

set(CMAKE_CXX_STANDARD 17)

find_package(protobuf CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(lisboa 
    src/main.cpp
    src/proto/fileformat.pb.cc
    src/proto/osmformat.pb.cc
)

target_include_directories(lisboa PRIVATE src)

target_link_libraries(lisboa PRIVATE protobuf::libprotobuf ZLIB::ZLIB)