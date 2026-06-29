#!/bin/bash
# Build Yocto image for BBB AI-64

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
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
SOURCES_DIR="$PROJECT_ROOT/sources"

IMAGE_NAME="${1:-custom-image}"
MACHINE="${2:-bbbai64}"
DISTRO="${3:-poky}"
BUILD_TARGETS="${4:-$IMAGE_NAME}"
CLEAN="${5:-false}"

# ============================================================================
# Main
# ============================================================================
main() {
    print_header "Building BBB AI-64 Image"
    echo ""
    print_info "Image: $IMAGE_NAME"
    print_info "Machine: $MACHINE"
    print_info "Distro: $DISTRO"
    echo ""
    
    # Setup environment
    cd "$SOURCES_DIR/poky"
    source oe-init-build-env "$BUILD_DIR"
    
    # Configure build
    echo "MACHINE = \"$MACHINE\"" >> conf/local.conf
    echo "DISTRO = \"$DISTRO\"" >> conf/local.conf
    
    # Clean if requested
    if [ "$CLEAN" = "true" ]; then
        print_info "Cleaning build..."
        bitbake -c cleanall "$IMAGE_NAME"
    fi
    
    # Build the image
    print_info "Building image (this may take several hours)..."
    bitbake "$IMAGE_NAME"
    
    print_success "Build completed!"
    echo ""
    print_info "Image location: $BUILD_DIR/deploy/images/$MACHINE/"
    echo ""
    print_info "To flash to SD card:"
    echo "  ./scripts/flash-sdcard.sh $IMAGE_NAME /dev/sdX"
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of build-image.sh
# ============================================================================
