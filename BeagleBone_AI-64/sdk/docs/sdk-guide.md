# BBB AI-64 SDK Guide

## Introduction

The BBB AI-64 SDK provides a complete development environment for creating applications for the BeagleBone Black AI-64 platform. This guide covers installation, configuration, and usage of the SDK.

## Installation

### 1. Download SDK

Download the SDK installer from the releases page or from your Yocto build:

```bash
# From release
wget https://github.com/yourusername/BeagleBone_AI-64/releases/latest/poky-sdk-installer.sh

# From build
cp build/deploy/sdk/poky-glibc-x86_64-custom-image-core-image-sdk-1.0.sh .

2. Install SDK
bash

# Make executable
chmod +x poky-sdk-installer.sh

# Install to default location
./poky-sdk-installer.sh

# Or install to custom location
./poky-sdk-installer.sh -d /opt/bbb-ai64-sdk

3. Verify Installation
bash

# Check SDK directory
ls -la /opt/poky/1.0/

# Check toolchain
/opt/poky/1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc --version

Environment Setup
1. Source Environment
bash

# Source the environment setup script
source /opt/poky/1.0/environment-setup-cortexa72-poky-linux

# Or use the SDK helper script
source sdk/environment-setup.sh

2. Verify Environment
bash

# Check compiler
$CC --version

# Check sysroot
echo $SYSROOT

# Check flags
echo $CFLAGS
echo $LDFLAGS

Building Applications
1. Simple C Program
bash

# Create source file
cat > hello.c << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello BBB AI-64!\n");
    return 0;
}
EOF

# Compile
$CC -o hello hello.c

# Check binary
file hello
# Output: hello: ELF 64-bit LSB executable, ARM aarch64, version 1 (SYSV), dynamically linked...

2. C++ Program
bash

# Create source file
cat > hello.cpp << 'EOF'
#include <iostream>
using namespace std;
int main() {
    cout << "Hello BBB AI-64!" << endl;
    return 0;
}
EOF

# Compile
$CXX -o hello hello.cpp

3. Qt Application
bash

# Create project
qmake -project
qmake
make

# Or use CMake
mkdir build && cd build
cmake ..
make

Deploying Applications
1. Deploy to Target
bash

# Copy to target
scp hello root@<board-ip>:/tmp/

# Or use the deploy script
sdk/scripts/deploy-target.sh <board-ip> hello

2. Run on Target
bash

# SSH to target
ssh root@<board-ip>

# Run application
/tmp/hello

Debugging
1. On Target Debug
bash

# On target, start gdbserver
gdbserver :1234 /tmp/hello

# On host, connect
arm-poky-linux-gnueabi-gdb /tmp/hello
(gdb) target remote <board-ip>:1234
(gdb) continue

2. Remote Debugging
bash

# On host, start gdb with remote target
$GDB
(gdb) file hello
(gdb) target remote <board-ip>:1234
(gdb) break main
(gdb) continue
(gdb) info registers
(gdb) print variable
(gdb) continue

Cross-Compilation Tips
1. Makefile
makefile

CC = arm-poky-linux-gnueabi-gcc
CXX = arm-poky-linux-gnueabi-g++
AR = arm-poky-linux-gnueabi-ar
STRIP = arm-poky-linux-gnueabi-strip

CFLAGS = -O2 -Wall -I$(SYSROOT)/usr/include
LDFLAGS = -L$(SYSROOT)/usr/lib

all: program

program: source.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f program

install: program
	cp program /usr/bin/

2. CMake
cmake

# Use toolchain file
cmake -DCMAKE_TOOLCHAIN_FILE=sdk/cmake/Toolchain.cmake ..

# Or set variables
cmake -DCMAKE_C_COMPILER=arm-poky-linux-gnueabi-gcc \
      -DCMAKE_CXX_COMPILER=arm-poky-linux-gnueabi-g++ \
      -DCMAKE_SYSROOT=/opt/poky/1.0/sysroots/cortexa72-poky-linux \
      ..

3. Autotools
bash

# Configure for cross-compilation
./configure --host=arm-poky-linux-gnueabi \
            --prefix=/usr \
            --with-sysroot=/opt/poky/1.0/sysroots/cortexa72-poky-linux

# Build
make
make install DESTDIR=/tmp/install

Library Management
1. Finding Libraries
bash

# Use pkg-config
pkg-config --cflags --libs libname

# Check library path
echo $LD_LIBRARY_PATH

# Find library
find $SYSROOT -name "lib*.so"

2. Building Libraries
bash

# Build shared library
$CC -shared -o libmylib.so source.c

# Build static library
$AR rcs libmylib.a source.o

# Install library
cp libmylib.so $SYSROOT/usr/lib/

Troubleshooting
Common Issues
Compiler Not Found
bash

# Check PATH
echo $PATH

# Check SDK installation
ls -la /opt/poky/1.0/

# Re-source environment
source /opt/poky/1.0/environment-setup-cortexa72-poky-linux

Linker Errors
bash

# Check library path
echo $LD_LIBRARY_PATH

# Add library path
export LD_LIBRARY_PATH=$SYSROOT/usr/lib:$LD_LIBRARY_PATH

# Use verbose linking
$CC -Wl,--verbose ...

Incorrect Sysroot
bash

# Check sysroot
echo $SYSROOT

# Verify sysroot contents
ls -la $SYSROOT/usr/include
ls -la $SYSROOT/usr/lib

Performance Optimization
1. Compiler Optimization
bash

# Level 2 optimization
CFLAGS="-O2 -pipe"

# Level 3 optimization
CFLAGS="-O3 -pipe"

# Size optimization
CFLAGS="-Os -pipe"

2. Architecture-Specific Optimizations
bash

# Cortex-A72 specific
CFLAGS="-mcpu=cortex-a72 -mtune=cortex-a72"

# NEON optimization
CFLAGS="-mfpu=neon"

# Hard-float
CFLAGS="-mfloat-abi=hard"

3. Link-Time Optimization
bash

# Enable LTO
CFLAGS="-flto"
LDFLAGS="-flto"

Security
1. Hardening Options
bash

# Stack protection
CFLAGS="-fstack-protector-strong"

# Position Independent Executable
CFLAGS="-fPIE"
LDFLAGS="-pie"

# Full RELRO
LDFLAGS="-Wl,-z,relro,-z,now"

2. Signing Binaries
bash

# Sign with GPG
gpg --detach-sign program

# Verify signature
gpg --verify program.sig program

Additional Resources
Documentation

    Yocto Project SDK Manual

    GCC Documentation

    Qt Documentation

    CMake Documentation

Tools

    GDB: Debugger

    Valgrind: Memory debugger

    Perf: Performance analyzer

    Strace: System call tracer

Examples
Hello World

examples/hello-world/hello.c
Sensor Reader

examples/sensor-reader/sensor-reader.c
Qt GUI

examples/qt-gui/main.qml
Support

    Community: https://beagleboard.org/community

    Issues: https://github.com/yourusername/BeagleBone_AI-64/issues

    Documentation: docs/ directory

text


This complete SDK structure provides:

1. **README.md**: Comprehensive SDK documentation
2. **environment-setup.sh**: Environment setup script
3. **Toolchain**: Cross-compilation toolchain
4. **CMake Toolchain**: CMake cross-compilation configuration
5. **Examples**: Hello World, Sensor Reader, Qt GUI
6. **Scripts**: Build, deploy, debug scripts
7. **Documentation**: Complete SDK guide

The SDK enables:
- Cross-compilation for ARM64
- Qt6 application development
- Sensor programming
- Remote debugging
- Performance optimization
- Security hardening



