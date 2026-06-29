#!/bin/bash
# Prepare BBB AI-64 SDK
# This script prepares the SDK environment for development

set -e

# ============================================================================
# Colors
# ============================================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
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

print_header() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}========================================${NC}"
}

# ============================================================================
# Configuration
# ============================================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDK_DIR="$(dirname "$SCRIPT_DIR")"
TOOLCHAIN_DIR="$SDK_DIR/toolchain"
SYSROOT="$TOOLCHAIN_DIR/sysroot"

# Default SDK location (Yocto SDK)
YOCTO_SDK="${1:-/opt/poky/1.0}"
CROSS_COMPILE="${2:-arm-poky-linux-gnueabi-}"
TARGET_TRIPLE="${3:-cortexa72-poky-linux}"

# ============================================================================
# Check Yocto SDK
# ============================================================================
check_yocto_sdk() {
    print_info "Checking Yocto SDK at: $YOCTO_SDK"
    
    if [ ! -d "$YOCTO_SDK" ]; then
        print_error "Yocto SDK not found at: $YOCTO_SDK"
        echo ""
        print_info "Please install the Yocto SDK first:"
        echo "  ./scripts/create-sdk.sh"
        echo "  # or download from releases"
        exit 1
    fi
    
    # Check for toolchain
    local toolchain_dir="$YOCTO_SDK/sysroots/x86_64-pokysdk-linux/usr/bin/${CROSS_COMPILE%%-}"
    if [ ! -d "$toolchain_dir" ]; then
        print_error "Toolchain not found in SDK"
        exit 1
    fi
    
    # Check for sysroot
    local sysroot_dir="$YOCTO_SDK/sysroots/$TARGET_TRIPLE"
    if [ ! -d "$sysroot_dir" ]; then
        print_error "Sysroot not found in SDK"
        exit 1
    fi
    
    print_success "Yocto SDK found"
    print_info "  Toolchain: $toolchain_dir"
    print_info "  Sysroot: $sysroot_dir"
}

# ============================================================================
# Setup Toolchain
# ============================================================================
setup_toolchain() {
    print_info "Setting up toolchain..."
    
    # Create directories
    mkdir -p "$TOOLCHAIN_DIR/bin"
    mkdir -p "$TOOLCHAIN_DIR/include"
    mkdir -p "$TOOLCHAIN_DIR/lib"
    mkdir -p "$SYSROOT/usr/include"
    mkdir -p "$SYSROOT/usr/lib"
    mkdir -p "$SYSROOT/usr/share"
    
    # Copy toolchain
    local toolchain_src="$YOCTO_SDK/sysroots/x86_64-pokysdk-linux/usr/bin/${CROSS_COMPILE%%-}"
    if [ -d "$toolchain_src" ]; then
        print_info "Copying toolchain from: $toolchain_src"
        cp -r "$toolchain_src"/* "$TOOLCHAIN_DIR/bin/"
        chmod +x "$TOOLCHAIN_DIR/bin"/*
    fi
    
    # Copy sysroot
    local sysroot_src="$YOCTO_SDK/sysroots/$TARGET_TRIPLE"
    if [ -d "$sysroot_src" ]; then
        print_info "Copying sysroot from: $sysroot_src"
        cp -r "$sysroot_src/usr/include" "$SYSROOT/usr/"
        cp -r "$sysroot_src/usr/lib" "$SYSROOT/usr/"
        cp -r "$sysroot_src/usr/share" "$SYSROOT/usr/"
    fi
    
    print_success "Toolchain setup complete"
}

# ============================================================================
# Create Environment Script
# ============================================================================
create_environment_script() {
    print_info "Creating environment setup script..."
    
    cat > "$SDK_DIR/environment-setup.sh" << 'EOF'
#!/bin/bash
# BBB AI-64 SDK Environment Setup Script

# ============================================================================
# Configuration
# ============================================================================
SDK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLCHAIN_DIR="$SDK_DIR/toolchain"
SYSROOT="$TOOLCHAIN_DIR/sysroot"
CROSS_COMPILE="arm-poky-linux-gnueabi-"

# ============================================================================
# Toolchain Paths
# ============================================================================
export PATH="$TOOLCHAIN_DIR/bin:$PATH"

# ============================================================================
# Compiler Settings
# ============================================================================
export CC="${CROSS_COMPILE}gcc"
export CXX="${CROSS_COMPILE}g++"
export CPP="${CROSS_COMPILE}cpp"
export LD="${CROSS_COMPILE}ld"
export AR="${CROSS_COMPILE}ar"
export AS="${CROSS_COMPILE}as"
export RANLIB="${CROSS_COMPILE}ranlib"
export STRIP="${CROSS_COMPILE}strip"
export OBJCOPY="${CROSS_COMPILE}objcopy"
export OBJDUMP="${CROSS_COMPILE}objdump"
export GDB="${CROSS_COMPILE}gdb"

# ============================================================================
# Compiler Flags
# ============================================================================
export CFLAGS="-O2 -pipe -g -feliminate-unused-debug-types"
export CXXFLAGS="${CFLAGS}"
export CPPFLAGS="-I${SYSROOT}/usr/include"
export LDFLAGS="-L${SYSROOT}/usr/lib -Wl,-rpath-link,${SYSROOT}/usr/lib"

# ============================================================================
# pkg-config
# ============================================================================
export PKG_CONFIG_SYSROOT_DIR="${SYSROOT}"
export PKG_CONFIG_PATH="${SYSROOT}/usr/lib/pkgconfig:${SYSROOT}/usr/share/pkgconfig"

# ============================================================================
# Qt6 Settings
# ============================================================================
export QMAKESPEC="$SDK_DIR/qmake/mkspecs/linux-arm-gnueabi-g++"
export QT_QMAKE_EXECUTABLE="$TOOLCHAIN_DIR/bin/qmake"

# ============================================================================
# CMake
# ============================================================================
export CMAKE_TOOLCHAIN_FILE="$SDK_DIR/cmake/Toolchain.cmake"

# ============================================================================
# Python
# ============================================================================
export PYTHONPATH="${SYSROOT}/usr/lib/python3.10/site-packages:${PYTHONPATH}"

# ============================================================================
# Helper Functions
# ============================================================================
bbbsdk-build() {
    echo "Building application..."
    make "$@"
}

bbbsdk-clean() {
    echo "Cleaning build..."
    make clean
}

bbbsdk-deploy() {
    if [ -z "$1" ]; then
        echo "Usage: bbbsdk-deploy <target-ip> [file]"
        return 1
    fi
    if [ -n "$2" ]; then
        scp "$2" root@$1:/tmp/
    else
        scp ${PWD##*/} root@$1:/tmp/
    fi
}

bbbsdk-debug() {
    if [ -z "$1" ]; then
        echo "Usage: bbbsdk-debug <target-ip> [program]"
        return 1
    fi
    if [ -n "$2" ]; then
        ssh root@$1 "gdbserver :1234 $2"
    else
        ssh root@$1 "gdbserver :1234 /tmp/${PWD##*/}"
    fi
}

export -f bbbsdk-build
export -f bbbsdk-clean
export -f bbbsdk-deploy
export -f bbbsdk-debug

# ============================================================================
# Environment Verification
# ============================================================================
echo "BBB AI-64 SDK Environment Loaded"
echo ""
echo "SDK Directory: $SDK_DIR"
echo "Toolchain: $TOOLCHAIN_DIR"
echo "Sysroot: $SYSROOT"
echo ""
echo "Compiler: $CC"
echo "C++ Compiler: $CXX"
echo ""
echo "Helper functions:"
echo "  bbbsdk-build  - Build application"
echo "  bbbsdk-clean  - Clean build"
echo "  bbbsdk-deploy - Deploy to target"
echo "  bbbsdk-debug  - Debug on target"
echo ""
EOF
    
    chmod +x "$SDK_DIR/environment-setup.sh"
    print_success "Environment script created: $SDK_DIR/environment-setup.sh"
}

# ============================================================================
# Create Example Projects
# ============================================================================
create_examples() {
    print_info "Creating example projects..."
    
    # Hello World example
    mkdir -p "$SDK_DIR/examples/hello-world"
    cat > "$SDK_DIR/examples/hello-world/hello.c" << 'EOF'
#include <stdio.h>
int main() {
    printf("Hello BBB AI-64!\n");
    return 0;
}
EOF
    
    cat > "$SDK_DIR/examples/hello-world/Makefile" << 'EOF'
CC = arm-poky-linux-gnueabi-gcc
CFLAGS = -O2 -Wall -g
TARGET = hello
SOURCES = hello.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
EOF
    
    # Sensor Reader example
    mkdir -p "$SDK_DIR/examples/sensor-reader"
    cat > "$SDK_DIR/examples/sensor-reader/sensor-reader.c" << 'EOF'
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

int main() {
    printf("Sensor Reader Example\n");
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
    
    cat > "$SDK_DIR/examples/sensor-reader/Makefile" << 'EOF'
CC = arm-poky-linux-gnueabi-gcc
CFLAGS = -O2 -Wall -g
TARGET = sensor-reader
SOURCES = sensor-reader.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
EOF
    
    print_success "Example projects created in $SDK_DIR/examples/"
}

# ============================================================================
# Create Build Scripts
# ============================================================================
create_build_scripts() {
    print_info "Creating build scripts..."
    
    # build-app.sh
    cat > "$SCRIPT_DIR/build-app.sh" << 'EOF'
#!/bin/bash
# Build application script
source "$SDK_DIR/environment-setup.sh"
echo "Building application..."
make "$@"
EOF
    chmod +x "$SCRIPT_DIR/build-app.sh"
    
    # deploy-target.sh
    cat > "$SCRIPT_DIR/deploy-target.sh" << 'EOF'
#!/bin/bash
# Deploy to target script
if [ -z "$1" ]; then
    echo "Usage: $0 <target-ip> [file]"
    exit 1
fi
source "$SDK_DIR/environment-setup.sh"
bbbsdk-deploy "$@"
EOF
    chmod +x "$SCRIPT_DIR/deploy-target.sh"
    
    # qmake-setup.sh
    cat > "$SCRIPT_DIR/qmake-setup.sh" << 'EOF'
#!/bin/bash
# QMake setup script
source "$SDK_DIR/environment-setup.sh"
echo "QMake environment:"
echo "  QMAKESPEC: $QMAKESPEC"
echo "  QT_QMAKE_EXECUTABLE: $QT_QMAKE_EXECUTABLE"
echo ""
qmake --version
EOF
    chmod +x "$SCRIPT_DIR/qmake-setup.sh"
    
    print_success "Build scripts created in $SCRIPT_DIR/"
}

