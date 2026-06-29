cmake_minimum_required(VERSION 3.16)
project(device-manager VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Include paths
set(DEVICE_MANAGER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(DEVICE_MANAGER_SOURCES
    src/DeviceManager.cpp
    src/Device.cpp
    src/DeviceFactory.cpp
    src/DeviceManagerConfig.cpp
    src/DeviceManagerEvents.cpp
)

# Create library
add_library(device-manager STATIC ${DEVICE_MANAGER_SOURCES})

target_include_directories(device-manager PUBLIC
    ${DEVICE_MANAGER_INCLUDE_DIR}
    ${JSONCPP_INCLUDE_DIRS}
)

target_link_libraries(device-manager
    ${JSONCPP_LIBRARIES}
    pthread
    rt
    dl
)

# Install
install(TARGETS device-manager DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)

# Tests
option(BUILD_DEVICE_MANAGER_TESTS "Build device manager tests" ON)
if(BUILD_DEVICE_MANAGER_TESTS)
    find_package(GTest REQUIRED)
    add_executable(device_manager_test
        tests/DeviceManagerTest.cpp
        tests/DeviceTest.cpp
        tests/DeviceFactoryTest.cpp
    )
    target_link_libraries(device_manager_test
        device-manager
        GTest::GTest
        GTest::Main
    )
    target_include_directories(device_manager_test PRIVATE
        ${DEVICE_MANAGER_INCLUDE_DIR}
    )
    add_test(NAME device_manager_test COMMAND device_manager_test)
endif()

# Example executable
add_executable(device-manager-example
    examples/device_manager_example.cpp
)
target_link_libraries(device-manager-example device-manager)
target_include_directories(device-manager-example PRIVATE
    ${DEVICE_MANAGER_INCLUDE_DIR}
)
