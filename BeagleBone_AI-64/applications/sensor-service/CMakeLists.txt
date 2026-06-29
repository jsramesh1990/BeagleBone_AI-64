cmake_minimum_required(VERSION 3.16)
project(sensor-service VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)
pkg_check_modules(LIBSERIAL REQUIRED libserialport)

# Include paths
set(SENSOR_SERVICE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SENSOR_SERVICE_SOURCES
    src/main.cpp
    src/SensorService.cpp
    src/SensorManager.cpp
    src/SensorEvents.cpp
    drivers/IMUDriver.cpp
    drivers/GPSDriver.cpp
    drivers/TemperatureDriver.cpp
    drivers/PressureDriver.cpp
    drivers/HumidityDriver.cpp
)

set(SENSOR_SERVICE_HEADERS
    include/sensor-service/SensorService.h
    include/sensor-service/SensorManager.h
    include/sensor-service/SensorDriver.h
    include/sensor-service/SensorData.h
    include/sensor-service/SensorConfig.h
    include/sensor-service/SensorEvents.h
    drivers/IMUDriver.h
    drivers/GPSDriver.h
    drivers/TemperatureDriver.h
    drivers/PressureDriver.h
    drivers/HumidityDriver.h
)

# Create library
add_library(sensor-service STATIC ${SENSOR_SERVICE_SOURCES})

target_include_directories(sensor-service PUBLIC
    ${SENSOR_SERVICE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
    ${JSONCPP_INCLUDE_DIRS}
    ${LIBSERIAL_INCLUDE_DIRS}
)

target_compile_definitions(sensor-service PRIVATE
    -DSENSOR_SERVICE_USE_I2C
    -DSENSOR_SERVICE_USE_SPI
    -DSENSOR_SERVICE_USE_SERIAL
)

target_link_libraries(sensor-service
    ${JSONCPP_LIBRARIES}
    ${LIBSERIAL_LIBRARIES}
    pthread
    rt
    dl
    device-manager
    ipc
    logger
    config
    utils
)

# Install
install(TARGETS sensor-service DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY drivers/ DESTINATION include/sensor-service/drivers)

# Executable
add_executable(sensor-service-daemon src/daemon.cpp)
target_link_libraries(sensor-service-daemon sensor-service)
target_include_directories(sensor-service-daemon PRIVATE
    ${SENSOR_SERVICE_INCLUDE_DIR}
)

# Tests
option(BUILD_SENSOR_TESTS "Build sensor tests" ON)
if(BUILD_SENSOR_TESTS)
    add_subdirectory(tests)
endif()
