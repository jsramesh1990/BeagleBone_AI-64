cmake_minimum_required(VERSION 3.16)

find_package(GTest REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test Core)

add_executable(gui_app_test
    DashboardTest.cpp
    DataManagerTest.cpp
)

target_link_libraries(gui_app_test
    gui-app
    device-manager
    ipc
    logger
    config
    utils
    GTest::GTest
    GTest::Main
    Qt6::Test
)

target_include_directories(gui_app_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/../device-manager/include
    ${CMAKE_SOURCE_DIR}/../common/logger/include
    ${CMAKE_SOURCE_DIR}/../common/ipc/include
    ${CMAKE_SOURCE_DIR}/../common/config/include
    ${CMAKE_SOURCE_DIR}/../common/utils/include
)

add_test(NAME gui_app_test COMMAND gui_app_test)
