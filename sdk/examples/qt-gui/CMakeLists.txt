# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(dashboard VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Sensors)

# Add executable
qt_add_executable(dashboard
    main.cpp
    qml.qrc
)

# Link libraries
target_link_libraries(dashboard
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Sensors
)

# Install
install(TARGETS dashboard DESTINATION bin)
