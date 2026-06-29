#!/bin/bash
# Build Application Script for BBB AI-64 SDK
# This script builds applications using the cross-compilation toolchain

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

# Source SDK environment if not already sourced
if [ -z "$CROSS_COMPILE" ]; then
    source "$SDK_DIR/environment-setup.sh"
fi

# Build settings
BUILD_TYPE="${1:-release}"  # release, debug, profile
CLEAN_BUILD="${2:-false}"   # true, false
VERBOSE="${3:-false}"       # true, false
JOBS="${4:-$(nproc)}"       # Number of parallel jobs

# ============================================================================
# Usage
# ============================================================================
show_usage() {
    cat << EOF
${CYAN}BBB AI-64 Build Script${NC}

Usage: $0 [OPTIONS] [SOURCE_DIR]

Options:
    -t, --type TYPE      Build type: release, debug, profile (default: release)
    -c, --clean          Clean before build
    -v, --verbose        Verbose output
    -j, --jobs N         Number of parallel jobs (default: $(nproc))
    -h, --help           Show this help message
    -l, --list           List supported build systems

Examples:
    $0                           # Build current directory
    $0 /path/to/source           # Build specific directory
    $0 -t debug -c               # Clean debug build
    $0 -v -j 4                   # Verbose build with 4 jobs
    $0 -l                        # List supported build systems

EOF
}

# ============================================================================
# Detect Build System
# ============================================================================
detect_build_system() {
    local dir="$1"
    
    if [ -f "$dir/CMakeLists.txt" ]; then
        echo "cmake"
    elif [ -f "$dir/Makefile" ] || [ -f "$dir/makefile" ]; then
        echo "make"
    elif [ -f "$dir/*.pro" ] 2>/dev/null || [ -f "$dir/qmake.conf" ]; then
        echo "qmake"
    elif [ -f "$dir/configure.ac" ] || [ -f "$dir/configure.in" ]; then
        echo "autotools"
    elif [ -f "$dir/meson.build" ]; then
        echo "meson"
    elif [ -f "$dir/Cargo.toml" ]; then
        echo "cargo"
    elif [ -f "$dir/setup.py" ] || [ -f "$dir/pyproject.toml" ]; then
        echo "python"
    else
        echo "unknown"
    fi
}

# ============================================================================
# Build Functions
# ============================================================================
build_with_cmake() {
    local dir="$1"
    local build_type="$2"
    local clean="$3"
    local verbose="$4"
    local jobs="$5"
    
    print_info "Building with CMake in $dir..."
    
    cd "$dir"
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Clean if requested
    if [ "$clean" = "true" ]; then
        print_info "Cleaning build directory..."
        rm -rf *
    fi
    
    # Configure
    local cmake_args=""
    case $build_type in
        release)
            cmake_args="-DCMAKE_BUILD_TYPE=Release"
            ;;
        debug)
            cmake_args="-DCMAKE_BUILD_TYPE=Debug"
            ;;
        profile)
            cmake_args="-DCMAKE_BUILD_TYPE=RelWithDebInfo"
            ;;
    esac
    
    # Add toolchain file
    cmake_args="$cmake_args -DCMAKE_TOOLCHAIN_FILE=$SDK_DIR/cmake/Toolchain.cmake"
    
    if [ "$verbose" = "true" ]; then
        cmake_args="$cmake_args -DCMAKE_VERBOSE_MAKEFILE=ON"
    fi
    
    print_info "Configuring with CMake..."
    cmake .. $cmake_args
    
    # Build
    print_info "Building with $jobs jobs..."
    make -j$jobs
    
    print_success "CMake build complete!"
}

build_with_make() {
    local dir="$1"
    local build_type="$2"
    local clean="$3"
    local verbose="$4"
    local jobs="$5"
    
    print_info "Building with Make in $dir..."
    
    cd "$dir"
    
    # Clean if requested
    if [ "$clean" = "true" ]; then
        print_info "Cleaning..."
        make clean
    fi
    
    # Set build variables
    export CFLAGS="-O2 -pipe -g"
    export CXXFLAGS="-O2 -pipe -g"
    export LDFLAGS="-Wl,-rpath-link,$SYSROOT/usr/lib"
    
    if [ "$build_type" = "debug" ]; then
        export CFLAGS="-O0 -g -DDEBUG"
        export CXXFLAGS="-O0 -g -DDEBUG"
    fi
    
    # Build
    local make_args=""
    if [ "$verbose" = "true" ]; then
        make_args="V=1"
    fi
    
    make $make_args -j$jobs
    
    print_success "Make build complete!"
}

build_with_qmake() {
    local dir="$1"
    local build_type="$2"
    local clean="$3"
    local verbose="$4"
    local jobs="$5"
    
    print_info "Building with QMake in $dir..."
    
    cd "$dir"
    
    # Clean if requested
    if [ "$clean" = "true" ]; then
        print_info "Cleaning..."
        make clean 2>/dev/null || true
        rm -f Makefile
    fi
    
    # Set QMake spec
    export QMAKESPEC="$SDK_DIR/qmake/mkspecs/linux-arm-gnueabi-g++"
    
    # Generate Makefile
    local qmake_args=""
    case $build_type in
        release)
            qmake_args="CONFIG+=release"
            ;;
        debug)
            qmake_args="CONFIG+=debug"
            ;;
        profile)
            qmake_args="CONFIG+=release CONFIG+=profile"
            ;;
    esac
    
    print_info "Running qmake..."
    qmake $qmake_args
    
    # Build
    local make_args=""
    if [ "$verbose" = "true" ]; then
        make_args="V=1"
    fi
    
    make $make_args -j$jobs
    
    print_success "QMake build complete!"
}

build_with_autotools() {
    local dir="$1"
    local build_type="$2"
    local clean="$3"
    local verbose="$4"
    local jobs="$5"
    
    print_info "Building with Autotools in $dir..."
    
    cd "$dir"
    
    # Clean if requested
    if [ "$clean" = "true" ]; then
        print_info "Cleaning..."
        make distclean 2>/dev/null || true
        rm -f Makefile
    fi
    
    # Configure
    print_info "Running configure..."
    ./configure --host=arm-poky-linux-gnueabi \
                --prefix=/usr \
                --with-sysroot="$SYSROOT" \
                CFLAGS="-O2 -pipe -g" \
                CXXFLAGS="-O2 -pipe -g" \
                LDFLAGS="-Wl,-rpath-link,$SYSROOT/usr/lib"
    
    # Build
    local make_args=""
    if [ "$verbose" = "true" ]; then
        make_args="V=1"
    fi
    
    make $make_args -j$jobs
    
    print_success "Autotools build complete!"
}

build_with_meson() {
    local dir="$1"
    local build_type="$2"
    local clean="$3"
    local verbose="$4"
    local jobs="$5"
    
    print_info "Building with Meson in $dir..."
    
    cd "$dir"
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Clean if requested
    if [ "$clean" = "true" ]; then
        print_info "Cleaning..."
        rm -rf *
    fi
    
    # Configure
    local meson_args=""
    case $build_type in
        release)
            meson_args="--buildtype=release"
            ;;
        debug)
            meson_args="--buildtype=debug"
            ;;
        profile)
            meson_args="--buildtype=debugoptimized"
            ;;
    esac
    
    # Cross-compile configuration
    cat > cross-compile.txt << EOF
[binaries]
c = '${CROSS_COMPILE}gcc'
cpp = '${CROSS_COMPILE}g++'
ar = '${CROSS_COMPILE}ar'
strip = '${CROSS_COMPILE}strip'

[host_machine]
system = 'linux'
cpu_family = 'arm'
cpu = 'armv8-a'
endian = 'little'

[properties]
sys_root = '$SYSROOT'
EOF
    
    print_info "Running meson..."
    meson .. $meson_args --cross-file cross-compile.txt
    
    # Build
    local meson_build_args=""
    if [ "$verbose" = "true" ]; then
        meson_build_args="-v"
    fi
    
    ninja -j$jobs $meson_build_args
    
    print_success "Meson build complete!"
}

build_with_cargo() {
    local dir="$1"
    local build_type="$2"
    local clean="$3"
    local verbose="$4"
    local jobs="$5"
    
    print_info "Building with Cargo in $dir..."
    
    cd "$dir"
    
    # Clean if requested
    if [ "$clean" = "true" ]; then
        print_info "Cleaning..."
        cargo clean
    fi
    
    # Build
    local cargo_args=""
    case $build_type in
        release)
            cargo_args="--release"
            ;;
        debug)
            cargo_args=""
            ;;
        profile)
            cargo_args="--release"
            ;;
    esac
    
    if [ "$verbose" = "true" ]; then
        cargo_args="$cargo_args -v"
    fi
    
    # Set cross-compilation environment
    export CARGO_BUILD_TARGET="aarch64-unknown-linux-gnu"
    export CC_arm64_linux_gnu="${CROSS_COMPILE}gcc"
    export CXX_arm64_linux_gnu="${CROSS_COMPILE}g++"
    export AR_arm64_linux_gnu="${CROSS_COMPILE}ar"
    
    cargo build $cargo_args -j $jobs
    
    print_success "Cargo build complete!"
}

# ============================================================================
# Main
# ============================================================================
main() {
    # Parse arguments
    local source_dir="."
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            -t|--type)
                BUILD_TYPE="$2"
                shift 2
                ;;
            -c|--clean)
                CLEAN_BUILD="true"
                shift
                ;;
            -v|--verbose)
                VERBOSE="true"
                shift
                ;;
            -j|--jobs)
                JOBS="$2"
                shift 2
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            -l|--list)
                echo "Supported build systems: cmake, make, qmake, autotools, meson, cargo, python"
                exit 0
                ;;
            *)
                source_dir="$1"
                shift
                ;;
        esac
    done
    
    print_header "BBB AI-64 Build Script"
    echo ""
    print_info "Source: $source_dir"
    print_info "Build Type: $BUILD_TYPE"
    print_info "Clean: $CLEAN_BUILD"
    print_info "Verbose: $VERBOSE"
    print_info "Jobs: $JOBS"
    echo ""
    
    # Validate source directory
    if [ ! -d "$source_dir" ]; then
        print_error "Source directory not found: $source_dir"
        exit 1
    fi
    
    # Detect build system
    BUILD_SYSTEM=$(detect_build_system "$source_dir")
    print_info "Detected build system: $BUILD_SYSTEM"
    echo ""
    
    # Build based on detected system
    case $BUILD_SYSTEM in
        cmake)
            build_with_cmake "$source_dir" "$BUILD_TYPE" "$CLEAN_BUILD" "$VERBOSE" "$JOBS"
            ;;
        make)
            build_with_make "$source_dir" "$BUILD_TYPE" "$CLEAN_BUILD" "$VERBOSE" "$JOBS"
            ;;
        qmake)
            build_with_qmake "$source_dir" "$BUILD_TYPE" "$CLEAN_BUILD" "$VERBOSE" "$JOBS"
            ;;
        autotools)
            build_with_autotools "$source_dir" "$BUILD_TYPE" "$CLEAN_BUILD" "$VERBOSE" "$JOBS"
            ;;
        meson)
            build_with_meson "$source_dir" "$BUILD_TYPE" "$CLEAN_BUILD" "$VERBOSE" "$JOBS"
            ;;
        cargo)
            build_with_cargo "$source_dir" "$BUILD_TYPE" "$CLEAN_BUILD" "$VERBOSE" "$JOBS"
            ;;
        python)
            print_info "Building Python package..."
            cd "$source_dir"
            if [ -f "setup.py" ]; then
                python3 setup.py build
            else
                pip3 wheel .
            fi
            print_success "Python build complete!"
            ;;
        *)
            print_error "Unknown build system in $source_dir"
            echo ""
            print_info "Detected files:"
            ls -la "$source_dir" | head -20
            exit 1
            ;;
    esac
    
    echo ""
    print_success "Build completed successfully!"
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of build-app.sh
# ============================================================================
