cmake_minimum_required(VERSION 3.20)
project(factory_of_the_future C)

set(CMAKE_C_STANDARD 99)

add_executable(factory factory.c network/tcp_client.c network/tcp_server.c)

add_executable(dashboard dashboard.c network/tcp_client.c network/tcp_server.c)
