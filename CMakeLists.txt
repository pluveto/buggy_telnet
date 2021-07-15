cmake_minimum_required(VERSION 3.0.0)
project(buggy_telnet VERSION 0.1.0)


set(INC_DIR "/app/vcpkg/installed/x64-linux/include")
set(LINK_DIR "/app/vcpkg/installed/x64-linux/lib")
include_directories(${INC_DIR})
link_directories(${LINK_DIR})



add_executable(btel_client client.cpp)
add_executable(btel_server server.cpp)

find_package(spdlog CONFIG REQUIRED)

target_link_libraries(btel_client PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(btel_server PRIVATE spdlog::spdlog spdlog::spdlog_header_only)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})