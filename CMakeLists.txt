cmake_minimum_required(VERSION 3.5)

project(console_chat_on_cpp)

add_compile_options(
    -Wall
)

add_executable(server src/server.cpp)
add_executable(client src/client.cpp)
add_executable(client_message_screen src/client_message_screen.cpp)

add_library(logger src/logger.hpp src/logger.cpp)
target_link_libraries(server PRIVATE logger)

find_package(Threads REQUIRED)
target_link_libraries(client PRIVATE Threads::Threads)
