cmake_minimum_required(VERSION 3.16)
project(factory_of_the_future C)

find_package (Threads)
find_library(WIRINGPI_LIB wiringPi)

set(CMAKE_C_STANDARD 99)

add_executable(factory factory.c network/tcp_client.c network/tcp_server.c network/connection.c utils/host_list.c  network/connection.c utils/sensor_history.c peripherals/bme280.c peripherals/peripherals_control.c)
target_link_libraries (factory ${CMAKE_THREAD_LIBS_INIT})
IF(${WIRINGPI_LIB})
    target_link_libraries(factory ${WIRINGPI_LIB})
ENDIF()

add_executable(dashboard dashboard.c network/tcp_client.c network/tcp_server.c utils/host_list.c network/connection.c)
target_link_libraries (dashboard ${CMAKE_THREAD_LIBS_INIT})