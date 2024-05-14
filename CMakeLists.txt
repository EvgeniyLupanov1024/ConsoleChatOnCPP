cmake_minimum_required(VERSION 3.5)

project(console_chat_on_cpp)

add_compile_options(
    -Wall
)

find_package(Threads REQUIRED)
add_library(logger src/logger.hpp src/logger.cpp)
add_library(fifo_channel src/fifo_channel.hpp src/fifo_channel.cpp)

add_executable(server src/server.cpp src/consts.hpp)
add_executable(client src/client.cpp src/consts.hpp)
add_executable(client_message_screen src/client_message_screen.cpp src/consts.hpp)

target_link_libraries(server PRIVATE logger)
target_link_libraries(client PRIVATE fifo_channel Threads::Threads)
target_link_libraries(client_message_screen PRIVATE fifo_channel)
