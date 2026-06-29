#!/bin/bash
# Prepare SDK for development

set -e

# ============================================================================
# Colors
# ============================================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# ============================================================================
# Configuration
# ============================================================================
SDK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.."
TOOLCHAIN_DIR="${SDK_DIR}/toolchain"
SYSROOT="${TOOLCHAIN_DIR}/sysroot"

# ============================================================================
# Main
# ============================================================================
print_info "Preparing BBB AI-64 SDK..."

# Create directories
mkdir -p "${TOOLCHAIN_DIR}/bin"
mkdir -p "${TOOLCHAIN_DIR}/include"
mkdir -p "${TOOLCHAIN_DIR}/lib"
mkdir -p "${SYSROOT}/usr/include"
mkdir -p "${SYSROOT}/usr/lib"
mkdir -p "${SYSROOT}/usr/share"

# Copy toolchain
print_info "Copying toolchain..."
# In production, this would copy from Yocto SDK
# For now, create symlinks

# Create environment script
print_info "Creating environment setup script..."
cat > "${SDK_DIR}/environment-setup.sh" << 'EOF'
# BBB AI-64 SDK Environment Setup Script
export SDK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export TOOLCHAIN_DIR="${SDK_DIR}/toolchain"
export SYSROOT="${TOOLCHAIN_DIR}/sysroot"
export CROSS_COMPILE="arm-poky-linux-gnueabi-"
export PATH="${TOOLCHAIN_DIR}/bin:${PATH}"
export CC="${CROSS_COMPILE}gcc"
export CXX="${CROSS_COMPILE}g++"
export AR="${CROSS_COMPILE}ar"
export LD="${CROSS_COMPILE}ld"
export STRIP="${CROSS_COMPILE}strip"
export CFLAGS="-O2 -pipe -g"
export CXXFLAGS="${CFLAGS}"
export LDFLAGS="-L${SYSROOT}/usr/lib -Wl,-rpath-link,${SYSROOT}/usr/lib"
export PKG_CONFIG_SYSROOT_DIR="${SYSROOT}"
export PKG_CONFIG_PATH="${SYSROOT}/usr/lib/pkgconfig:${SYSROOT}/usr/share/pkgconfig"
export CMAKE_TOOLCHAIN_FILE="${SDK_DIR}/cmake/Toolchain.cmake"
echo "BBB AI-64 SDK Environment Loaded"
echo "CC: $CC"
echo "CXX: $CXX"
EOF

# Create CMake toolchain file
print_info "Creating CMake toolchain file..."
cat > "${SDK_DIR}/cmake/Toolchain.cmake" << 'EOF'
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CROSS_COMPILE arm-poky-linux-gnueabi-)
set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/sysroot)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF

# Create qmake configuration
print_info "Creating qmake configuration..."
mkdir -p "${SDK_DIR}/qmake/mkspecs/linux-arm-gnueabi-g++"
cat > "${SDK_DIR}/qmake/mkspecs/linux-arm-gnueabi-g++/qmake.conf" << 'EOF'
MAKEFILE_GENERATOR = UNIX
CONFIG += incremental
QMAKE_INCREMENTAL_STYLE = sublib
include(../common/linux.conf)
include(../common/gcc-base-unix.conf)
include(../common/g++-unix.conf)
QMAKE_CC = arm-poky-linux-gnueabi-gcc
QMAKE_CXX = arm-poky-linux-gnueabi-g++
QMAKE_LINK = arm-poky-linux-gnueabi-g++
QMAKE_LINK_SHLIB = arm-poky-linux-gnueabi-g++
QMAKE_AR = arm-poky-linux-gnueabi-ar cqs
QMAKE_OBJCOPY = arm-poky-linux-gnueabi-objcopy
QMAKE_STRIP = arm-poky-linux-gnueabi-strip
QMAKE_CFLAGS += -O2 -pipe
QMAKE_CXXFLAGS += -O2 -pipe
QMAKE_LFLAGS += -Wl,-rpath-link,/opt/poky/1.0/sysroots/cortexa72-poky-linux/usr/lib
EOF

# Create example project
print_info "Creating example projects..."
mkdir -p "${SDK_DIR}/examples/hello-world"
cat > "${SDK_DIR}/examples/hello-world/hello.c" << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello BBB AI-64!\n");
    printf("SDK Version: 1.0.0\n");
    return 0;
}
EOF

mkdir -p "${SDK_DIR}/examples/sensor-reader"
cat > "${SDK_DIR}/examples/sensor-reader/sensor.c" << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int main() {
    printf("Sensor Reader Example\n");
    printf("Opening I2C device...\n");
    
    int fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        perror("Failed to open I2C device");
        return 1;
    }
    
    printf("I2C device opened successfully\n");
    close(fd);
    return 0;
}
EOF

# Create build script
print_info "Creating build script..."
cat > "${SDK_DIR}/scripts/build-app.sh" << 'EOF'
#!/bin/bash
# Build application script

if [ -z "$1" ]; then
    echo "Usage: $0 <source-file> [output-name]"
    exit 1
fi

source "${SDK_DIR}/environment-setup.sh"

SOURCE="$1"
OUTPUT="${2:-${SOURCE%.*}}"

echo "Building $SOURCE -> $OUTPUT"
$CC $CFLAGS -o "$OUTPUT" "$SOURCE" $LDFLAGS

if [ $? -eq 0 ]; then
    echo "Build successful: $OUTPUT"
    file "$OUTPUT"
else
    echo "Build failed"
    exit 1
fi
EOF
chmod +x "${SDK_DIR}/scripts/build-app.sh"

# Create deploy script
print_info "Creating deploy script..."
cat > "${SDK_DIR}/scripts/deploy-target.sh" << 'EOF'
#!/bin/bash
# Deploy to target script

if [ -z "$1" ]; then
    echo "Usage: $0 <target-ip> [file]"
    exit 1
fi

TARGET_IP="$1"
FILE="${2:-./app}"

echo "Deploying $FILE to $TARGET_IP..."
scp "$FILE" "root@${TARGET_IP}:/tmp/"

if [ $? -eq 0 ]; then
    echo "Deploy successful"
    echo "To run: ssh root@${TARGET_IP} /tmp/$(basename $FILE)"
else
    echo "Deploy failed"
    exit 1
fi
EOF
chmod +x "${SDK_DIR}/scripts/deploy-target.sh"

print_success "SDK prepared successfully!"
echo ""
print_info "SDK Location: $SDK_DIR"
print_info "To use the SDK:"
echo "  source $SDK_DIR/environment-setup.sh"
echo ""
print_info "Examples:"
echo "  cd $SDK_DIR/examples/hello-world"
echo "  $SDK_DIR/scripts/build-app.sh hello.c"
echo "  $SDK_DIR/scripts/deploy-target.sh <board-ip> hello"
echo ""

# ============================================================================
# End of prepare-sdk.sh
# ============================================================================
