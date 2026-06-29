#!/bin/bash
# Create a release of the BBB AI-64 project

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
DEPLOY_DIR="$BUILD_DIR/deploy/images/bbbai64"
RELEASE_DIR="$PROJECT_ROOT/releases"
VERSION="${1:-$(date +%Y%m%d)}"
RELEASE_NAME="bbbai64-${VERSION}"

# ============================================================================
# Main
# ============================================================================
main() {
    print_header "Creating Release: $RELEASE_NAME"
    echo ""
    
    # Create release directory
    mkdir -p "$RELEASE_DIR/$RELEASE_NAME"
    
    # Copy image files
    print_info "Copying image files..."
    cp "$DEPLOY_DIR"/*.wic.gz "$RELEASE_DIR/$RELEASE_NAME/" 2>/dev/null || true
    cp "$DEPLOY_DIR"/*.manifest "$RELEASE_DIR/$RELEASE_NAME/" 2>/dev/null || true
    cp "$DEPLOY_DIR"/*.dtb "$RELEASE_DIR/$RELEASE_NAME/" 2>/dev/null || true
    cp "$DEPLOY_DIR"/*.dtbo "$RELEASE_DIR/$RELEASE_NAME/" 2>/dev/null || true
    cp "$DEPLOY_DIR"/Image "$RELEASE_DIR/$RELEASE_NAME/" 2>/dev/null || true
    
    # Copy SDK if available
    if [ -d "$BUILD_DIR/deploy/sdk" ]; then
        print_info "Copying SDK..."
        cp "$BUILD_DIR/deploy/sdk"/*.sh "$RELEASE_DIR/$RELEASE_NAME/" 2>/dev/null || true
    fi
    
    # Create release notes
    cat > "$RELEASE_DIR/$RELEASE_NAME/RELEASE.txt" << EOF
========================================
  BBB AI-64 Release $VERSION
========================================

Build Date: $(date)
Commit: $(git rev-parse HEAD 2>/dev/null || echo "unknown")
Branch: $(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown")

Contents:
---------
- Yocto image (.wic.gz)
- Kernel Image
- Device tree files (.dtb, .dtbo)
- Manifest file
- SDK installer (if available)

Flashing Instructions:
---------------------
1. Extract the image:
   gunzip ${RELEASE_NAME}.wic.gz

2. Flash to SD card:
   sudo dd if=${RELEASE_NAME}.wic of=/dev/sdX bs=4M status=progress

3. Insert SD card and power on the BBB AI-64

Boot Instructions:
-----------------
1. Connect serial console (115200,8N1)
2. Power on the board
3. Login: root (no password)

Features:
---------
- Linux kernel 6.1
- Custom distribution
- Qt6 GUI
- Sensor support (IMU, GPS, Temperature, Pressure, Humidity)
- Docker containers
- OTA updates (SWUpdate)

Known Issues:
------------
- None

EOF
    
    # Create checksums
    print_info "Creating checksums..."
    cd "$RELEASE_DIR/$RELEASE_NAME"
    sha256sum *.wic.gz > checksums.sha256 2>/dev/null || true
    sha256sum *.sh >> checksums.sha256 2>/dev/null || true
    
    # Create a tarball
    print_info "Creating tarball..."
    cd "$RELEASE_DIR"
    tar -czf "${RELEASE_NAME}.tar.gz" "$RELEASE_NAME/"
    
    print_success "Release created!"
    echo ""
    print_info "Release location: $RELEASE_DIR/${RELEASE_NAME}.tar.gz"
    echo ""
    print_info "Release contents:"
    ls -la "$RELEASE_DIR/$RELEASE_NAME/"
    echo ""
    print_info "To extract:"
    echo "  tar -xzf $RELEASE_DIR/${RELEASE_NAME}.tar.gz"
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of release.sh
# ============================================================================
