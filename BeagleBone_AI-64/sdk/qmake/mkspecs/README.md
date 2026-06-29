arkdown

# QMake Mkspecs for BBB AI-64

## Overview

This directory contains qmake mkspecs for cross-compiling Qt6 applications for the BeagleBone Black AI-64 platform.

## Directory Structure

mkspecs/
└── linux-arm-gnueabi-g++/
├── qmake.conf # Main qmake configuration
└── qplatformdefs.h # Platform definitions
text


## Usage

### 1. Set Up Environment

```bash
# Source the SDK environment
source sdk/environment-setup.sh

# Set QMAKESPEC
export QMAKESPEC=sdk/qmake/mkspecs/linux-arm-gnueabi-g++

# Or use the qmake.conf directly
export QT_QMAKE_EXECUTABLE=/opt/poky/1.0/sysroots/x86_64-pokysdk-linux/usr/bin/qmake

2. Create a Qt Project
bash

# Create a new Qt project
mkdir myapp && cd myapp

# Create a simple main.cpp
cat > main.cpp << 'EOF'
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    return app.exec();
}
EOF

# Create a project file
cat > myapp.pro << 'EOF'
QT += quick
CONFIG += c++17
SOURCES += main.cpp
RESOURCES += qml.qrc
EOF

3. Build the Application
bash

# Generate Makefile
qmake

# Build
make

# Check the binary
file myapp
# Should show: ELF 64-bit LSB executable, ARM aarch64

4. Deploy to Target
bash

# Deploy to board
scp myapp root@<board-ip>:/usr/bin/

# Run on board
ssh root@<board-ip>
export DISPLAY=:0
myapp

Configuration Details
Toolchain

The mkspecs use the Yocto SDK toolchain with the following settings:

    Compiler: arm-poky-linux-gnueabi-gcc/g++

    Sysroot: /opt/poky/1.0/sysroots/cortexa72-poky-linux

    Architecture: ARM Cortex-A72

    FPU: NEON with hard-float

Qt Features

The configuration enables:

    Qt Quick/QML

    OpenGL ES 2.0

    Linux Framebuffer

    EGLFS

    Systemd integration

    D-Bus

    udev

    OpenSSL

Security Features

    Stack protection (-fstack-protector-strong)

    Position Independent Executable (-fPIE)

    Full RELRO (-Wl,-z,relro,-z,now)

    No undefined symbols (-Wl,--no-undefined)

Troubleshooting
Compiler Not Found
bash

# Check SDK installation
ls -la /opt/poky/1.0/

# Verify toolchain exists
ls -la /opt/poky/1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/

Sysroot Not Found
bash

# Check sysroot
ls -la /opt/poky/1.0/sysroots/cortexa72-poky-linux/

# Verify Qt headers
ls -la /opt/poky/1.0/sysroots/cortexa72-poky-linux/usr/include/qt6/

qmake Not Found
bash

# Find qmake
find /opt/poky -name qmake 2>/dev/null

# Or use the SDK's qmake
export QT_QMAKE_EXECUTABLE=/opt/poky/1.0/sysroots/x86_64-pokysdk-linux/usr/bin/qmake

Linking Errors
bash

# Check library path
echo $LD_LIBRARY_PATH

# Add library path if needed
export LD_LIBRARY_PATH=/opt/poky/1.0/sysroots/cortexa72-poky-linux/usr/lib:$LD_LIBRARY_PATH

Examples
Simple QML Application
qml

// main.qml
import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    width: 400
    height: 300
    visible: true
    title: "BBB AI-64"
    
    Text {
        anchors.centerIn: parent
        text: "Hello BBB AI-64!"
        font.pixelSize: 24
    }
}

Project File
makefile

# myapp.pro
QT += quick quickcontrols2
CONFIG += c++17

SOURCES += main.cpp
RESOURCES += qml.qrc

target.path = /usr/bin
INSTALLS += target

Build Command
bash

# Build with qmake
qmake -spec sdk/qmake/mkspecs/linux-arm-gnueabi-g++
make

# Or with environment variable
QMAKESPEC=sdk/qmake/mkspecs/linux-arm-gnueabi-g++ qmake
make

Additional Resources

    Qt Documentation

    Yocto SDK Guide

    API Reference
