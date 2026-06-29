#!/bin/bash
# Flash Yocto image to SD card for BBB AI-64
# Supports multiple flash methods, verification, and backup

set -e

# ============================================================================
# Colors for output
# ============================================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
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

print_header() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}========================================${NC}"
}

print_step() {
    echo -e "${MAGENTA}>>>${NC} $1"
}

# ============================================================================
# Configuration
# ============================================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
DEPLOY_DIR="$BUILD_DIR/deploy/images/bbbai64"

# Default values
IMAGE_NAME="${1:-custom-image}"
DEVICE="${2:-}"
FLASH_METHOD="${3:-auto}"  # auto, dd, bmaptool, etcher
VERIFY="${4:-true}"
BACKUP_ORIGINAL="${5:-false}"
COMPRESSED="${6:-true}"

# ============================================================================
# Help
# ============================================================================
show_help() {
    cat << EOF
${CYAN}BBB AI-64 SD Card Flasher${NC}

Usage: $0 [IMAGE_NAME] [DEVICE] [METHOD] [VERIFY] [BACKUP] [COMPRESSED]

Arguments:
  IMAGE_NAME    Name of the image to flash (default: custom-image)
  DEVICE        SD card device (e.g., /dev/sdb)
  METHOD        Flash method: auto, dd, bmaptool, etcher (default: auto)
  VERIFY        Verify after flash: true/false (default: true)
  BACKUP        Backup original SD card: true/false (default: false)
  COMPRESSED    Image is compressed: true/false (default: true)

Examples:
  $0 custom-image /dev/sdb
  $0 production-image /dev/sdc dd true true
  $0 --list-devices

Options:
  --list-devices    List available devices
  --help            Show this help message

EOF
}

# ============================================================================
# Find image
# ============================================================================
find_image() {
    local image_path="$DEPLOY_DIR/${IMAGE_NAME}-bbbai64.wic"
    local image_gz_path="$DEPLOY_DIR/${IMAGE_NAME}-bbbai64.wic.gz"
    local image_bmap_path="$DEPLOY_DIR/${IMAGE_NAME}-bbbai64.wic.bmap"
    
    # Check for uncompressed image first
    if [ -f "$image_path" ]; then
        echo "$image_path"
        return 0
    fi
    
    # Check for compressed image
    if [ -f "$image_gz_path" ]; then
        echo "$image_gz_path"
        return 0
    fi
    
    # Try to find any .wic or .wic.gz file
    local wic_file=$(find "$DEPLOY_DIR" -maxdepth 1 -name "*.wic" | head -1)
    if [ -n "$wic_file" ]; then
        echo "$wic_file"
        return 0
    fi
    
    local wic_gz_file=$(find "$DEPLOY_DIR" -maxdepth 1 -name "*.wic.gz" | head -1)
    if [ -n "$wic_gz_file" ]; then
        echo "$wic_gz_file"
        return 0
    fi
    
    print_error "No image found in $DEPLOY_DIR"
    echo ""
    print_info "Available files in $DEPLOY_DIR:"
    ls -la "$DEPLOY_DIR" 2>/dev/null || echo "  (directory empty)"
    return 1
}

# ============================================================================
# List devices
# ============================================================================
list_devices() {
    print_header "Available Devices"
    echo ""
    echo "Device     Size    Type       Model"
    echo "--------  -------  ---------  -------------------------------"
    
    lsblk -o NAME,SIZE,TYPE,MODEL,MOUNTPOINT | grep -E "disk|part" | while read line; do
        # Colorize if it looks like an SD card
        if echo "$line" | grep -qiE "sd[b-z]|mmcblk"; then
            echo -e "${GREEN}$line${NC}"
        else
            echo "$line"
        fi
    done
    echo ""
    
    # Show mounted devices warning
    echo "${YELLOW}WARNING: Do not flash to devices that are mounted!${NC}"
    echo ""
}

# ============================================================================
# Check device
# ============================================================================
check_device() {
    if [ ! -b "$DEVICE" ]; then
        print_error "Device $DEVICE does not exist"
        echo ""
        print_info "Available devices:"
        list_devices
        return 1
    fi
    
    # Check if device is a block device
    if [ ! -e "$DEVICE" ]; then
        print_error "Device $DEVICE is not a block device"
        return 1
    fi
    
    # Check if device is mounted
    if mount | grep -q "$DEVICE"; then
        print_warning "Device $DEVICE is currently mounted!"
        echo ""
        read -p "Unmount it? (yes/no): " unmount_confirm
        
        if [ "$unmount_confirm" = "yes" ]; then
            print_info "Unmounting device..."
            sudo umount ${DEVICE}* 2>/dev/null || true
        else
            print_error "Cannot flash mounted device. Please unmount it first."
            return 1
        fi
    fi
    
    # Check if device is too small
    local device_size=$(sudo blockdev --getsize64 "$DEVICE" 2>/dev/null || echo "0")
    local min_size=$((4 * 1024 * 1024 * 1024)) # 4GB minimum
    
    if [ "$device_size" -lt "$min_size" ]; then
        print_warning "Device is smaller than 4GB (${device_size} bytes)"
        read -p "Continue anyway? (yes/no): " continue_anyway
        if [ "$continue_anyway" != "yes" ]; then
            return 1
        fi
    fi
    
    return 0
}

# ============================================================================
# Backup original SD card
# ============================================================================
backup_original() {
    if [ "$BACKUP_ORIGINAL" != "true" ]; then
        return 0
    fi
    
    print_step "Backing up original SD card..."
    
    local backup_file="backup_$(basename "$DEVICE")_$(date +%Y%m%d_%H%M%S).img"
    local backup_path="/tmp/$backup_file"
    
    print_info "Creating backup to $backup_path"
    print_warning "This may take several minutes..."
    
    sudo dd if="$DEVICE" of="$backup_path" bs=4M status=progress
    
    if [ -f "$backup_path" ]; then
        print_success "Backup created: $backup_path"
        echo ""
        print_info "To restore the backup:"
        echo "  sudo dd if=$backup_path of=$DEVICE bs=4M status=progress"
    else
        print_warning "Backup failed"
    fi
}

# ============================================================================
# Flash methods
# ============================================================================
flash_with_dd() {
    local image="$1"
    local is_compressed="$2"
    
    print_step "Flashing with dd..."
    
    # Unmount partitions
    sudo umount ${DEVICE}* 2>/dev/null || true
    
    if [ "$is_compressed" = "true" ]; then
        print_info "Writing compressed image (gunzip + dd)..."
        gunzip -c "$image" | sudo dd of="$DEVICE" bs=4M status=progress
    else
        print_info "Writing uncompressed image..."
        sudo dd if="$image" of="$DEVICE" bs=4M status=progress
    fi
    
    sync
    print_success "Flash complete"
}

flash_with_bmaptool() {
    local image="$1"
    local is_compressed="$2"
    
    print_step "Flashing with bmaptool..."
    
    # Check if bmaptool is installed
    if ! command -v bmaptool &> /dev/null; then
        print_warning "bmaptool not found. Installing..."
        sudo apt-get update && sudo apt-get install -y bmap-tools || {
            print_error "Failed to install bmaptool"
            return 1
        }
    fi
    
    # Unmount partitions
    sudo umount ${DEVICE}* 2>/dev/null || true
    
    # Find bmap file
    local bmap_file="${image%.gz}.bmap"
    if [ "$is_compressed" = "true" ]; then
        bmap_file="${image%.wic.gz}.wic.bmap"
    fi
    
    if [ ! -f "$bmap_file" ]; then
        print_warning "Bmap file not found: $bmap_file"
        print_info "Falling back to dd..."
        flash_with_dd "$image" "$is_compressed"
        return $?
    fi
    
    print_info "Using bmap file: $bmap_file"
    
    if [ "$is_compressed" = "true" ]; then
        print_info "Writing compressed image with bmaptool..."
        bmaptool copy --bmap "$bmap_file" "$image" "$DEVICE"
    else
        print_info "Writing uncompressed image with bmaptool..."
        bmaptool copy --bmap "$bmap_file" "$image" "$DEVICE"
    fi
    
    sync
    print_success "Flash complete"
}

flash_with_etcher() {
    print_step "Flashing with balenaEtcher..."
    
    # Check if etcher is installed
    if ! command -v balena-etcher-electron &> /dev/null; then
        print_warning "balenaEtcher not found. Please install it first."
        print_info "Visit: https://www.balena.io/etcher/"
        return 1
    fi
    
    print_info "Launching balenaEtcher..."
    print_info "Please select the image and device manually."
    
    balena-etcher-electron &
    local etcher_pid=$!
    
    print_info "Waiting for Etcher to complete..."
    wait $etcher_pid
    
    print_success "Flash complete"
}

# ============================================================================
# Verify flash
# ============================================================================
verify_flash() {
    if [ "$VERIFY" != "true" ]; then
        return 0
    fi
    
    print_step "Verifying flash..."
    
    # Check if device is readable
    if sudo dd if="$DEVICE" bs=4M count=1 status=none > /dev/null 2>&1; then
        print_success "Device is readable"
    else
        print_warning "Device read test failed"
        return 1
    fi
    
    # Check partition table
    if sudo fdisk -l "$DEVICE" 2>/dev/null | grep -q "Device"; then
        print_success "Partition table detected"
    else
        print_warning "Partition table not found"
    fi
    
    # Check boot partition
    local boot_part="${DEVICE}1"
    if [ -e "$boot_part" ]; then
        # Try to mount and check for boot files
        local temp_mount="/mnt/boot_verify"
        mkdir -p "$temp_mount"
        
        if sudo mount "$boot_part" "$temp_mount" 2>/dev/null; then
            if [ -f "$temp_mount/MLO" ] || [ -f "$temp_mount/u-boot.img" ]; then
                print_success "Boot files found"
            else
                print_warning "Boot files not found"
            fi
            sudo umount "$temp_mount"
        fi
        rmdir "$temp_mount" 2>/dev/null || true
    fi
    
    print_success "Verification complete"
}

# ============================================================================
# Main flash function
# ============================================================================
flash_image() {
    local image="$1"
    local image_name=$(basename "$image")
    local is_compressed=false
    
    if [[ "$image" == *.gz ]]; then
        is_compressed=true
    fi
    
    print_header "Flashing Image"
    echo ""
    print_info "Image: $image_name"
    print_info "Device: $DEVICE"
    print_info "Method: $FLASH_METHOD"
    print_info "Compressed: $is_compressed"
    echo ""
    
    # Confirmation
    print_warning "This will OVERWRITE all data on $DEVICE!"
    echo ""
    print_info "Image size: $(du -h "$image" | cut -f1)"
    print_info "Device size: $(sudo blockdev --getsize64 "$DEVICE" 2>/dev/null | numfmt --to=iec || echo "unknown")"
    echo ""
    
    read -p "Continue with flash? (yes/no): " confirm
    
    if [ "$confirm" != "yes" ]; then
        print_info "Flash cancelled"
        exit 0
    fi
    
    echo ""
    
    # Backup original if requested
    backup_original
    
    # Select flash method
    case "$FLASH_METHOD" in
        dd)
            flash_with_dd "$image" "$is_compressed"
            ;;
        bmaptool)
            flash_with_bmaptool "$image" "$is_compressed"
            ;;
        etcher)
            flash_with_etcher
            ;;
        auto)
            # Try bmaptool first, fallback to dd
            if command -v bmaptool &> /dev/null; then
                flash_with_bmaptool "$image" "$is_compressed" || flash_with_dd "$image" "$is_compressed"
            else
                flash_with_dd "$image" "$is_compressed"
            fi
            ;;
        *)
            print_error "Unknown flash method: $FLASH_METHOD"
            exit 1
            ;;
    esac
    
    # Verify
    verify_flash
}

# ============================================================================
# Main
# ============================================================================
main() {
    # Check for help or list devices
    if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
        show_help
        exit 0
    fi
    
    if [ "$1" = "--list-devices" ]; then
        list_devices
        exit 0
    fi
    
    print_header "BBB AI-64 SD Card Flasher"
    echo ""
    
    # Validate device
    if [ -z "$DEVICE" ]; then
        print_error "No device specified"
        echo ""
        show_help
        echo ""
        print_info "Available devices:"
        list_devices
        exit 1
    fi
    
    if [ "$DEVICE" = "/dev/sdX" ] || [ "$DEVICE" = "/dev/mmcblkX" ]; then
        print_error "Please specify the correct device"
        echo ""
        show_help
        echo ""
        print_info "Available devices:"
        list_devices
        exit 1
    fi
    
    # Check device
    if ! check_device; then
        exit 1
    fi
    
    # Find image
    IMAGE_PATH=$(find_image)
    if [ -z "$IMAGE_PATH" ]; then
        exit 1
    fi
    
    # Flash the image
    flash_image "$IMAGE_PATH"
    
    echo ""
    print_header "Flash Complete!"
    echo ""
    print_success "SD card is ready for BBB AI-64"
    echo ""
    print_info "Next steps:"
    echo "  1. Insert the SD card into the BBB AI-64"
    echo "  2. Connect serial console (115200, 8N1)"
    echo "  3. Power on the board"
    echo "  4. Login: root (no password)"
    echo ""
    print_info "To verify the flash, you can check the boot partition:"
    echo "  mkdir -p /mnt/sdcard"
    echo "  sudo mount ${DEVICE}1 /mnt/sdcard"
    echo "  ls -la /mnt/sdcard"
    echo "  sudo umount /mnt/sdcard"
    echo ""
}

# ============================================================================
# Signal handling
# ============================================================================
cleanup() {
    print_warning "Interrupted. Cleaning up..."
    # Unmount any mounted partitions
    sudo umount ${DEVICE}* 2>/dev/null || true
    exit 1
}

trap cleanup SIGINT SIGTERM

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of flash-sdcard.sh
# ============================================================================
