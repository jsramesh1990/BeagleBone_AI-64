# BBB AI-64 SDK Documentation

## Overview

The BBB AI-64 SDK provides a complete cross-compilation environment for developing applications for the BeagleBone Black AI-64 platform. It includes the toolchain, libraries, headers, and development tools needed to build applications for the target device.

## SDK Components

### Toolchain
- **GCC**: 12.2.0
- **Binutils**: 2.40
- **Glibc**: 2.36
- **GDB**: 12.1

### Libraries
- **Qt6**: 6.4.2
- **Boost**: 1.80.0
- **OpenSSL**: 3.0.7
- **JSON-C**: 0.16
- **YAML-CPP**: 0.7.0
- **libserialport**: 0.1.1

### Development Tools
- **CMake**: 3.24.0
- **QMake**: 6.4.2
- **Make**: 4.3
- **PkgConfig**: 0.29.2

## Installation

### 1. Download SDK
```bash
# Download from release
wget https://github.com/yourusername/BeagleBone_AI-64/releases/latest/poky-sdk-installer.sh

# Or from Yocto build
cp build/deploy/sdk/poky-glibc-x86_64-custom-image-core-image-sdk-1.0.sh .

2. Install SDK
bash

# Make executable
chmod +x poky-sdk-installer.sh

# Install (default location: /opt/poky/1.0)
./poky-sdk-installer.sh

# Or specify custom location
./poky-sdk-installer.sh -d /opt/bbb-ai64-sdk

3. Setup Environment
bash

# Source environment
source /opt/poky/1.0/environment-setup-cortexa72-poky-linux

# Or use the provided script
source sdk/environment-setup.sh

Quick Start
Hello World Example
bash

# Source environment
source sdk/environment-setup.sh

# Create source file
cat > hello.c << EOF
#include <stdio.h>
int main() {
    printf("Hello BBB AI-64!\n");
    return 0;
}
EOF

# Compile
$CC -o hello hello.c

# Deploy to target
scp hello root@<board-ip>:/tmp/

Qt Application
bash

# Source environment
source sdk/environment-setup.sh

# Create Qt project
qmake -project
qmake
make

# Deploy
scp app root@<board-ip>:/usr/bin/

Toolchain Details
Cross-Compiler Prefix
text

arm-poky-linux-gnueabi-

Common Commands
bash

# C compiler
$CC -o program source.c

# C++ compiler
$CXX -o program source.cpp

# Linker
$LD -o program object.o

# Archiver
$AR rcs lib.a object.o

# Object dump
$OBJDUMP -d program

# Strip symbols
$STRIP program

Build System Integration
CMake
bash

# Use provided toolchain file
cmake -DCMAKE_TOOLCHAIN_FILE=sdk/cmake/Toolchain.cmake ..

# Or set environment
source sdk/environment-setup.sh
cmake ..

Makefile
makefile

# Use environment variables
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)ld

CFLAGS = -O2 -Wall
LDFLAGS = -lstdc++

all: program

Autotools
bash

# Configure with host
./configure --host=arm-poky-linux-gnueabi

# Or use environment
source sdk/environment-setup.sh
./configure

Development Workflow
1. Setup Environment
bash

# Source SDK environment
source sdk/environment-setup.sh

# Verify toolchain
$CC --version

2. Build Application
bash

# Build with make
make

# Build with cmake
mkdir build && cd build
cmake ..
make

3. Test on Target
bash

# Copy to target
scp app root@<board-ip>:/tmp/

# SSH and run
ssh root@<board-ip>
/tmp/app

4. Debug
bash

# On target, start gdbserver
gdbserver :1234 /tmp/app

# On host, connect
$GDB /tmp/app
(gdb) target remote <board-ip>:1234

Environment Variables
Variable	Description
CC	C compiler command
CXX	C++ compiler command
CPP	C preprocessor command
LD	Linker command
AR	Archiver command
AS	Assembler command
OBJCOPY	Object copy command
STRIP	Strip command
CFLAGS	C compiler flags
CXXFLAGS	C++ compiler flags
LDFLAGS	Linker flags
CPPFLAGS	Preprocessor flags
PKG_CONFIG_PATH	pkg-config search path
CMAKE_TOOLCHAIN_FILE	CMake toolchain file
Troubleshooting
Common Issues
Toolchain Not Found
bash

# Verify SDK installation
ls /opt/poky/1.0/

# Check environment
echo $PATH

Compilation Errors
bash

# Check compiler flags
echo $CFLAGS

# Check include paths
echo $CPPFLAGS

# Check library paths
echo $LDFLAGS

Linker Errors
bash

# Check library path
echo $LD_LIBRARY_PATH

# Use verbose linking
$CC -Wl,--verbose ...

Getting Help
bash

# Compiler help
$CC --help

# Linker help
$LD --help

# CMake help
cmake --help

# pkg-config help
pkg-config --help

Advanced Usage
Custom Toolchain
bash

# Create custom toolchain
export CROSS_COMPILE=arm-poky-linux-gnueabi-
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++

Multiple SDK Versions
bash

# Switch SDK versions
source /opt/poky/1.0/environment-setup-cortexa72-poky-linux
# or
source /opt/poky/1.1/environment-setup-cortexa72-poky-linux

Cross-Debugging
bash

# On target
gdbserver :1234 ./program

# On host
arm-poky-linux-gnueabi-gdb ./program
(gdb) target remote <target-ip>:1234
(gdb) continue

Documentation
Additional Resources

    Yocto Project SDK Manual

    GCC Documentation

    Qt Documentation

    CMake Documentation

Examples

See sdk/examples/ for sample projects:

    Hello World: Basic C program

    Sensor Reader: Read sensor data

    Qt GUI: Qt application example
