cmake_minimum_required(VERSION 3.13)
project(factory_of_the_future C)

find_package (Threads)

set(CMAKE_C_STANDARD 99)

find_library(WIRINGPI_LIB wiringPi)
find_package(GSL)
IF(WIRINGPI_LIB)
    add_executable(factory factory.c network/tcp_client.c network/tcp_server.c network/connection.c utils/host_list.c utils/sensor_history.c peripherals/bme280.c peripherals/peripherals_control.c utils/sensor_data_utils.c)
    target_link_libraries(factory ${CMAKE_THREAD_LIBS_INIT} ${WIRINGPI_LIB})
ELSE()
    add_executable(factory factory.c network/tcp_client.c network/tcp_server.c network/connection.c utils/host_list.c utils/sensor_history.c peripherals/peripherals_test.c utils/sensor_data_utils.c)
    target_link_libraries (factory ${CMAKE_THREAD_LIBS_INIT})
ENDIF()

add_executable(dashboard dashboard.c network/tcp_client.c network/tcp_server.c utils/host_list.c network/connection.c utils/sensor_data_utils.c dashlib/plot.c dashlib/database.c dashlib/factory_comm.c dashlib/commands.c dashlib/listing.c dashlib/factory_data.c utils/sensor_history.c)
target_link_libraries (dashboard ${CMAKE_THREAD_LIBS_INIT})

IF(GSL_FOUND)
    add_executable(ml ml_module.c network/tcp_client.c network/tcp_server.c utils/host_list.c network/connection.c utils/sensor_data_utils.c utils/sensor_history.c mlearn/data_files.c mlearn/csv.c mlearn/predict_temp.c mlearn/fit.c)
    target_link_libraries (ml ${CMAKE_THREAD_LIBS_INIT} GSL::gsl GSL::gslcblas)
ENDIF()