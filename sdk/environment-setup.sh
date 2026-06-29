
### sdk/environment-setup.sh

```bash
#!/bin/bash
# BBB AI-64 SDK Environment Setup Script

# ============================================================================
# Configuration
# ============================================================================
SDK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLCHAIN_DIR="${SDK_DIR}/toolchain"
SYSROOT="${TOOLCHAIN_DIR}/sysroot"

# Cross-compile prefix
CROSS_COMPILE="arm-poky-linux-gnueabi-"

# ============================================================================
# Toolchain Paths
# ============================================================================
export PATH="${TOOLCHAIN_DIR}/bin:${PATH}"

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
export QT_QMAKE_EXECUTABLE="${TOOLCHAIN_DIR}/bin/qmake"
export QMAKESPEC="${SDK_DIR}/qmake/mkspecs/linux-arm-gnueabi-g++"

# ============================================================================
# CMake
# ============================================================================
export CMAKE_TOOLCHAIN_FILE="${SDK_DIR}/cmake/Toolchain.cmake"

# ============================================================================
# Python
# ============================================================================
export PYTHONPATH="${SYSROOT}/usr/lib/python3.10/site-packages:${PYTHONPATH}"

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
echo "To verify:"
echo "  $CC --version"
echo "  $CXX --version"
echo ""

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

echo "Helper functions:"
echo "  bbbsdk-build  - Build application"
echo "  bbbsdk-clean  - Clean build"
echo "  bbbsdk-deploy - Deploy to target"
echo "  bbbsdk-debug  - Debug on target"
echo ""

# ============================================================================
# End of environment-setup.sh
# ============================================================================
