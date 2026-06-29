cmake_minimum_required(VERSION 3.16)
project(gui-app VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 
             QuickWidgets Qml Network WebSockets SerialPort)

# Include paths
set(GUI_APP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(GUI_APP_SOURCES
    src/main.cpp
    src/Dashboard.cpp
    src/DataManager.cpp
    src/SensorClient.cpp
    src/DeviceController.cpp
    src/ChartModel.cpp
    src/SettingsManager.cpp
    src/ThemeManager.cpp
)

set(GUI_APP_HEADERS
    include/gui-app/Dashboard.h
    include/gui-app/DataManager.h
    include/gui-app/SensorClient.h
    include/gui-app/DeviceController.h
    include/gui-app/ChartModel.h
    include/gui-app/SettingsManager.h
    include/gui-app/ThemeManager.h
)

# Create executable
add_executable(gui-app
    ${GUI_APP_SOURCES}
    ${GUI_APP_HEADERS}
    resources/qml.qrc
)

# Include directories
target_include_directories(gui-app PRIVATE
    ${GUI_APP_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(gui-app
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickWidgets
    Qt6::Qml
    Qt6::Network
    Qt6::WebSockets
    Qt6::SerialPort
    device-manager
    ipc
    logger
    config
    utils
)

# Set QML import path
target_compile_definitions(gui-app PRIVATE
    QT_QML_IMPORT_PATH="${CMAKE_SOURCE_DIR}/qml"
)

# Install
install(TARGETS gui-app DESTINATION bin)
install(DIRECTORY qml/ DESTINATION share/gui-app/qml)
install(DIRECTORY resources/ DESTINATION share/gui-app/resources)

# Tests
option(BUILD_GUI_TESTS "Build GUI tests" ON)
if(BUILD_GUI_TESTS)
    add_subdirectory(tests)
endif()
