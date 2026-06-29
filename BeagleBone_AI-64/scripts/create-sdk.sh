#!/bin/bash
# Flash Yocto image to SD card for BBB AI-64

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
IMAGE_NAME="${1:-custom-image}"
DEVICE="${2:-/dev/sdX}"

# ============================================================================
# Functions
# ============================================================================
find_image() {
    local image_path="$DEPLOY_DIR/${IMAGE_NAME}-bbbai64.wic.gz"
    if [ -f "$image_path" ]; then
        echo "$image_path"
        return 0
    fi
    
    # Try to find any .wic.gz file
    local wic_file=$(find "$DEPLOY_DIR" -name "*.wic.gz" | head -1)
    if [ -n "$wic_file" ]; then
        echo "$wic_file"
        return 0
    fi
    
    print_error "No image found in $DEPLOY_DIR"
    return 1
}

check_device() {
    if [ ! -b "$DEVICE" ]; then
        print_error "Device $DEVICE does not exist"
        return 1
    fi
    
    # Check if device is mounted
    if mount | grep -q "$DEVICE"; then
        print_error "Device $DEVICE is mounted. Please unmount it first."
        return 1
    fi
    
    return 0
}

unmount_device() {
    print_info "Unmounting device partitions..."
    sudo umount ${DEVICE}* 2>/dev/null || true
}

flash_image() {
    local image="$1"
    
    print_info "Flashing $image to $DEVICE..."
    print_warning "This will overwrite all data on $DEVICE"
    echo ""
    read -p "Continue? (yes/no): " confirm
    
    if [ "$confirm" != "yes" ]; then
        print_info "Flash cancelled"
        exit 0
    fi
    
    # Unmount any mounted partitions
    unmount_device
    
    # Flash the image
    print_info "Writing image (this may take a few minutes)..."
    
    if command -v bmaptool &> /dev/null; then
        # Use bmaptool for faster flashing
        bmaptool copy "$image" "$DEVICE"
    else
        # Use dd as fallback
        gunzip -c "$image" | sudo dd of="$DEVICE" bs=4M status=progress
        sync
    fi
    
    print_success "Image flashed successfully!"
}

verify_flash() {
    print_info "Verifying flash..."
    
    # Check if device is readable
    if sudo dd if="$DEVICE" bs=4M count=1 status=none > /dev/null; then
        print_success "Verification passed"
    else
        print_warning "Verification failed. Please check the device."
    fi
}

# ============================================================================
# Main
# ============================================================================
main() {
    print_header "Flashing SD Card for BBB AI-64"
    echo ""
    print_info "Image: $IMAGE_NAME"
    print_info "Device: $DEVICE"
    echo ""
    
    # Validate
    if [ "$DEVICE" = "/dev/sdX" ]; then
        print_error "Please specify the correct device"
        echo ""
        echo "Usage: $0 [image-name] [device]"
        echo ""
        echo "Example: $0 custom-image /dev/sdb"
        echo ""
        echo "Available devices:"
        lsblk -o NAME,SIZE,TYPE,MOUNTPOINT | grep -E "disk|part"
        exit 1
    fi
    
    # Find image
    IMAGE_PATH=$(find_image)
    if [ -z "$IMAGE_PATH" ]; then
        exit 1
    fi
    print_info "Image: $IMAGE_PATH"
    
    # Check device
    if ! check_device; then
        exit 1
    fi
    
    # Flash image
    flash_image "$IMAGE_PATH"
    
    # Verify
    verify_flash
    
    echo ""
    print_success "SD card ready!"
    print_info "Insert the SD card into the BBB AI-64 and power on"
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of flash-sdcard.sh
# ============================================================================
