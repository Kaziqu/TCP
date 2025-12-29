cmake_minimum_required(VERSION 3.10)
project(cppgrind)

set(CMAKE_CXX_STANDARD 17)

find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Protobuf REQUIRED)
add_executable(test test.cpp packet.pb.cc)
target_include_directories(test PRIVATE 
                      ${CMAKE_CURRENT_SOURCE_DIR}
                      ${Protobuf_INCLUDE_DIRS}
                      /opt/homebrew/lib)

if(Protobuf_LIBRARY_DIRS)
    target_link_directories(test PRIVATE ${Protobuf_LIBRARY_DIRS})
else()
    target_link_directories(test PRIVATE /opt/homebrew/lib)
endif()

target_link_libraries(test PRIVATE 
    protobuf::libprotobuf
    ${Protobuf_LIBRARIES}
    )