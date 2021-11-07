cmake_minimum_required(VERSION 3.20)
project(factory_of_the_future C)

find_package (Threads)

set(CMAKE_C_STANDARD 99)

add_executable(factory factory.c network/tcp_client.c network/tcp_server.c utils/host_list.c network/ip_address.c)
target_link_libraries (factory ${CMAKE_THREAD_LIBS_INIT})

add_executable(dashboard dashboard.c network/tcp_client.c network/tcp_server.c utils/host_list.c)
target_link_libraries (dashboard ${CMAKE_THREAD_LIBS_INIT})

add_executable(tcp_tester network/tcp_tester.c network/tcp_client.c network/tcp_server.c)
target_link_libraries (tcp_tester ${CMAKE_THREAD_LIBS_INIT})
add_executable(hostname_ip network/ip_address.c)