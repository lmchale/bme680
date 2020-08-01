cmake_minimum_required(VERSION 3.6.0)

project(bme680_logger)
set(CMAKE_CXX_STANDARD 17)

add_executable(bme680-logger main.cpp)
target_link_libraries(bme680-logger spi-bme680 stdc++fs)
target_include_directories(bme680-logger PRIVATE "BME680_driver.git")

add_subdirectory(BME680_driver.git)

