#!/bin/bash
# Complete build script for the BBB AI-64 project

set -e

# ============================================================================
# Configuration
# ============================================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
IMAGE_NAME="${1:-custom-image}"
MACHINE="${2:-bbbai64}"
DISTRO="${3:-poky}"
BUILD_TARGETS="${4:-$IMAGE_NAME}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ============================================================================
# Functions
# ============================================================================
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
# Setup Environment
# ============================================================================
print_info "Setting up build environment..."

# Check for required tools
if ! command -v bitbake &> /dev/null; then
    print_info "Initializing Yocto environment..."
    cd "$PROJECT_ROOT/sources/poky"
    source oe-init-build-env "$BUILD_DIR"
else
    print_info "Yocto environment already initialized"
fi

# ============================================================================
# Configure Build
# ============================================================================
print_info "Configuring build for ${MACHINE}..."

# Set machine and distro
export MACHINE="$MACHINE"
export DISTRO="$DISTRO"

# ============================================================================
# Build Image
# ============================================================================
print_info "Building ${IMAGE_NAME}..."
print_info "This may take several hours depending on your system"

cd "$BUILD_DIR"

# Check if we should clean first
if [ "$5" == "clean" ]; then
    print_info "Cleaning build..."
    bitbake -c cleanall "$IMAGE_NAME"
fi

# Build the image
if ! bitbake "$IMAGE_NAME"; then
    print_error "Build failed!"
    exit 1
fi

# ============================================================================
# Create SDK (optional)
# ============================================================================
if [ "$6" == "sdk" ]; then
    print_info "Creating SDK..."
    if ! bitbake -c populate_sdk "$IMAGE_NAME"; then
        print_warning "SDK creation failed"
    else
        print_success "SDK created successfully"
    fi
fi

# ============================================================================
# Build Summary
# ============================================================================
print_success "Build completed successfully!"
echo ""
echo "===== Build Summary ====="
echo "Machine: $MACHINE"
echo "Distro: $DISTRO"
echo "Image: $IMAGE_NAME"
echo ""
echo "Output location:"
echo "  - Image: $BUILD_DIR/deploy/images/$MACHINE/"
echo "  - SDK: $BUILD_DIR/deploy/sdk/"
echo ""
echo "Image files:"
ls -lh $BUILD_DIR/deploy/images/$MACHINE/*.wic 2>/dev/null || echo "  No image files found"
ls -lh $BUILD_DIR/deploy/images/$MACHINE/*.rootfs.tar.xz 2>/dev/null || echo "  No rootfs files found"
echo ""
echo "To flash to SD card:"
echo "  sudo dd if=$BUILD_DIR/deploy/images/$MACHINE/${IMAGE_NAME}-$MACHINE.wic of=/dev/sdX bs=4M status=progress"
echo ""
print_info "Build complete!"

# ============================================================================
# End of build-image.sh
# ============================================================================
