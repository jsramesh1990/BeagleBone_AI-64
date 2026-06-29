#!/bin/bash
# QMake Setup Script for BBB AI-64 SDK
# This script sets up the qmake environment for cross-compilation

set -e

# ============================================================================
# Colors
# ============================================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
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
QMAKE_MKSPECS="$SDK_DIR/qmake/mkspecs"

# ============================================================================
# Usage
# ============================================================================
show_usage() {
    cat << EOF
${CYAN}BBB AI-64 QMake Setup Script${NC}

Usage: $0 [OPTIONS]

Options:
    -s, --sdk PATH      Path to Yocto SDK (default: /opt/poky/1.0)
    -t, --target TARGET Target triple (default: cortexa72-poky-linux)
    -h, --help          Show this help message
    -v, --version       Show version information

This script sets up the qmake environment for cross-compilation.

Examples:
    $0                              # Use default SDK path
    $0 -s /opt/poky/2.0             # Use custom SDK path
    $0 -t aarch64-poky-linux        # Use different target triple

EOF
}

# ============================================================================
# Setup QMake Environment
# ============================================================================
setup_qmake_env() {
    local sdk_path="$1"
    local target_triple="$2"
    
    print_info "Setting up QMake environment..."
    
    # Check SDK path
    if [ ! -d "$sdk_path" ]; then
        print_error "SDK path not found: $sdk_path"
        return 1
    fi
    
    # Find qmake
    local qmake_bin=$(find "$sdk_path" -name "qmake" -type f 2>/dev/null | head -1)
    if [ -z "$qmake_bin" ]; then
        print_warning "qmake not found in SDK path"
        return 1
    fi
    print_info "QMake binary: $qmake_bin"
    
    # Find sysroot
    local sysroot="$sdk_path/sysroots/$target_triple"
    if [ ! -d "$sysroot" ]; then
        print_warning "Sysroot not found: $sysroot"
        return 1
    fi
    print_info "Sysroot: $sysroot"
    
    # Find Qt include path
    local qt_include="$sysroot/usr/include/qt6"
    if [ ! -d "$qt_include" ]; then
        print_warning "Qt include not found: $qt_include"
        return 1
    fi
    print_info "Qt include: $qt_include"
    
    # Find Qt library path
    local qt_lib="$sysroot/usr/lib"
    if [ ! -d "$qt_lib" ]; then
        print_warning "Qt library path not found: $qt_lib"
        return 1
    fi
    print_info "Qt library: $qt_lib"
    
    # Create qmake.conf if it doesn't exist
    local qmake_conf="$QMAKE_MKSPECS/linux-arm-gnueabi-g++/qmake.conf"
    if [ ! -f "$qmake_conf" ]; then
        print_info "Creating qmake.conf..."
        mkdir -p "$(dirname "$qmake_conf")"
        cat > "$qmake_conf" << EOF
# QMake configuration for BBB AI-64 Cross-Compilation
MAKEFILE_GENERATOR = UNIX
CONFIG += incremental
QMAKE_INCREMENTAL_STYLE = sublib

include(../common/linux.conf)
include(../common/gcc-base-unix.conf)
include(../common/g++-unix.conf)

QMAKE_CC                = ${CROSS_COMPILE}gcc
QMAKE_CXX               = ${CROSS_COMPILE}g++
QMAKE_LINK              = ${CROSS_COMPILE}g++
QMAKE_LINK_SHLIB        = ${CROSS_COMPILE}g++
QMAKE_AR                = ${CROSS_COMPILE}ar cqs
QMAKE_OBJCOPY           = ${CROSS_COMPILE}objcopy
QMAKE_STRIP             = ${CROSS_COMPILE}strip
QMAKE_RANLIB            = ${CROSS_COMPILE}ranlib

QMAKE_DEFAULT_SYSROOT = $sysroot

QMAKE_CFLAGS            = -O2 -pipe -g -feliminate-unused-debug-types
QMAKE_CXXFLAGS          = -O2 -pipe -g -feliminate-unused-debug-types
QMAKE_CFLAGS            += -mcpu=cortex-a72 -mtune=cortex-a72
QMAKE_CXXFLAGS          += -mcpu=cortex-a72 -mtune=cortex-a72
QMAKE_CFLAGS            += -mfpu=neon -mfloat-abi=hard
QMAKE_CXXFLAGS          += -mfpu=neon -mfloat-abi=hard
QMAKE_CFLAGS            += -fstack-protector-strong -fPIE
QMAKE_CXXFLAGS          += -fstack-protector-strong -fPIE

QMAKE_LFLAGS            = -Wl,-rpath-link,$sysroot/usr/lib
QMAKE_LFLAGS            += -Wl,-z,relro,-z,now
QMAKE_LFLAGS            += -Wl,--no-undefined

QMAKE_INCDIR            = $sysroot/usr/include
QMAKE_INCDIR_QT         = $sysroot/usr/include/qt6
QMAKE_INCDIR_OPENGL     = $sysroot/usr/include/GL
QMAKE_INCDIR_OPENGL_ES2 = $sysroot/usr/include/GLES2
QMAKE_INCDIR_EGL        = $sysroot/usr/include

QMAKE_LIBDIR            = $sysroot/usr/lib
QMAKE_LIBDIR_QT         = $sysroot/usr/lib
QMAKE_LIBDIR_OPENGL     = $sysroot/usr/lib

QT_MAJOR_VERSION = 6
QT_MINOR_VERSION = 4
QT_PATCH_VERSION = 2

QT_CONFIG += shared release reduce_relocations
QT_CONFIG += linuxfb eglfs glib iconv icu
QT_CONFIG += png freetype harfbuzz jpeg gif zlib
QT_CONFIG += openssl opengl opengles2 udev evdev
QT_CONFIG += systemd dbus qpa

QPA_DEFAULT_PLATFORM = linuxfb
QMAKE_QPA_PLATFORM = linuxfb

PKG_CONFIG = pkg-config
PKG_CONFIG_SYSROOT_DIR = $sysroot
PKG_CONFIG_LIBDIR = $sysroot/usr/lib/pkgconfig

QMAKE_INSTALL_PREFIX = /usr
QMAKE_INSTALL_BINDIR = /usr/bin
QMAKE_INSTALL_LIBDIR = /usr/lib
QMAKE_INSTALL_HEADERS = /usr/include
EOF
        print_success "Created qmake.conf"
    fi
    
    # Create qplatformdefs.h if it doesn't exist
    local platform_defs="$QMAKE_MKSPECS/linux-arm-gnueabi-g++/qplatformdefs.h"
    if [ ! -f "$platform_defs" ]; then
        print_info "Creating qplatformdefs.h..."
        cat > "$platform_defs" << EOF
#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

#include "../linux-g++/qplatformdefs.h"

#define QT_CACHE_LINE_SIZE 64
#define QT_ARCH_ARM 1
#define QT_HAVE_NEON 1
#define QT_HAVE_ARM_NEON 1

#if defined(__ARMEL__) || defined(__ARM_LITTLE_ENDIAN__)
#define Q_BYTE_ORDER Q_LITTLE_ENDIAN
#else
#define Q_BYTE_ORDER Q_BIG_ENDIAN
#endif

#ifndef QT_NO_THREAD
#define QT_USE_PTHREADS
#endif

#define QT_USE_GLIBC

#endif
EOF
        print_success "Created qplatformdefs.h"
    fi
    
    # Create environment script
    local env_script="$SDK_DIR/qmake-env.sh"
    cat > "$env_script" << EOF
#!/bin/bash
# QMake environment for BBB AI-64 Cross-Compilation

export QMAKESPEC="$QMAKE_MKSPECS/linux-arm-gnueabi-g++"
export QT_QMAKE_EXECUTABLE="$qmake_bin"
export PKG_CONFIG_SYSROOT_DIR="$sysroot"
export PKG_CONFIG_LIBDIR="$sysroot/usr/lib/pkgconfig"
export CROSS_COMPILE="${CROSS_COMPILE:-arm-poky-linux-gnueabi-}"

echo "QMake environment loaded:"
echo "  QMAKESPEC: $QMAKESPEC"
echo "  QT_QMAKE_EXECUTABLE: $QT_QMAKE_EXECUTABLE"
EOF
    chmod +x "$env_script"
    print_success "Created qmake-env.sh: $env_script"
    
    # Print summary
    echo ""
    print_header "QMake Setup Complete"
    echo ""
    print_info "To use qmake, source the environment:"
    echo "  source $env_script"
    echo ""
    print_info "Or set environment variables directly:"
    echo "  export QMAKESPEC=$QMAKE_MKSPECS/linux-arm-gnueabi-g++"
    echo "  export QT_QMAKE_EXECUTABLE=$qmake_bin"
    echo ""
    print_info "To test qmake:"
    echo "  qmake --version"
    echo ""
    print_success "QMake environment ready!"
}

# ============================================================================
# Main
# ============================================================================
main() {
    local sdk_path="/opt/poky/1.0"
    local target_triple="cortexa72-poky-linux"
    
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -s|--sdk)
                sdk_path="$2"
                shift 2
                ;;
            -t|--target)
                target_triple="$2"
                shift 2
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            -v|--version)
                echo "QMake Setup Script v1.0.0"
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
    
    print_header "BBB AI-64 QMake Setup"
    echo ""
    print_info "SDK Path: $sdk_path"
    print_info "Target Triple: $target_triple"
    echo ""
    
    # Setup qmake environment
    setup_qmake_env "$sdk_path" "$target_triple"
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of qmake-setup.sh
# ============================================================================
