cmake_minimum_required(VERSION 3.16)

# Find GTest
find_package(GTest REQUIRED)

# Include directories
set(SENSOR_SERVICE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../include)
set(SENSOR_DRIVERS_DIR ${CMAKE_SOURCE_DIR}/../drivers)

# SensorManager Test
add_executable(sensor_manager_test
    SensorManagerTest.cpp
)

target_link_libraries(sensor_manager_test
    sensor-service
    device-manager
    ipc
    logger
    config
    utils
    GTest::GTest
    GTest::Main
    pthread
    rt
    dl
)

target_include_directories(sensor_manager_test PRIVATE
    ${SENSOR_SERVICE_INCLUDE_DIR}
    ${SENSOR_DRIVERS_DIR}
    ${CMAKE_SOURCE_DIR}/../src
    ${CMAKE_SOURCE_DIR}/../../device-manager/include
    ${CMAKE_SOURCE_DIR}/../../common/logger/include
    ${CMAKE_SOURCE_DIR}/../../common/ipc/include
    ${CMAKE_SOURCE_DIR}/../../common/config/include
    ${CMAKE_SOURCE_DIR}/../../common/utils/include
)

target_compile_definitions(sensor_manager_test PRIVATE
    -DSENSOR_SERVICE_TEST
)

add_test(NAME sensor_manager_test COMMAND sensor_manager_test)

# SensorService Test
add_executable(sensor_service_test
    SensorServiceTest.cpp
)

target_link_libraries(sensor_service_test
    sensor-service
    device-manager
    ipc
    logger
    config
    utils
    GTest::GTest
    GTest::Main
    pthread
    rt
    dl
)

target_include_directories(sensor_service_test PRIVATE
    ${SENSOR_SERVICE_INCLUDE_DIR}
    ${SENSOR_DRIVERS_DIR}
    ${CMAKE_SOURCE_DIR}/../src
    ${CMAKE_SOURCE_DIR}/../../device-manager/include
    ${CMAKE_SOURCE_DIR}/../../common/logger/include
    ${CMAKE_SOURCE_DIR}/../../common/ipc/include
    ${CMAKE_SOURCE_DIR}/../../common/config/include
    ${CMAKE_SOURCE_DIR}/../../common/utils/include
)

target_compile_definitions(sensor_service_test PRIVATE
    -DSENSOR_SERVICE_TEST
)

add_test(NAME sensor_service_test COMMAND sensor_service_test)

# Mock Driver for testing
add_library(mock_driver STATIC
    MockSensorDriver.cpp
)

target_include_directories(mock_driver PRIVATE
    ${SENSOR_SERVICE_INCLUDE_DIR}
    ${SENSOR_DRIVERS_DIR}
)

target_link_libraries(mock_driver
    sensor-service
)
