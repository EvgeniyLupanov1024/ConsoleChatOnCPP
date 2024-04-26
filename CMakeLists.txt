cmake_minimum_required(VERSION 3.5)

project(console_chat_on_cpp)

add_compile_options(
    -Wall
)

add_executable(server src/server.cpp)
add_executable(client src/client.cpp)