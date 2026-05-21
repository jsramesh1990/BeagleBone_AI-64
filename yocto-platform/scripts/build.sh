#!/bin/bash
# scripts/build.sh - Main build orchestration script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLATFORM_ROOT="$(dirname "$SCRIPT_DIR")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  -m, --machine MACHINE    Target machine"
    echo "  -t, --target TARGET      Build target (default: core-image-base)"
    echo "  -c, --config CONFIG      Kas config file"
    echo "  -d, --dev                Development build"
    echo "  -r, --release            Release build"
    echo "  -s, --sdk                Build SDK"
    echo "  -h, --help               Show this help"
}

MACHINE="qemux86-64"
TARGET="core-image-base"
CONFIG="kas/dev/dev.yml"
BUILD_SDK=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -m|--machine)
            MACHINE="$2"
            shift 2
            ;;
        -t|--target)
            TARGET="$2"
            shift 2
            ;;
        -c|--config)
            CONFIG="$2"
            shift 2
            ;;
        -d|--dev)
            CONFIG="kas/dev/dev.yml"
            shift
            ;;
        -r|--release)
            CONFIG="kas/release/release.yml"
            shift
            ;;
        -s|--sdk)
            BUILD_SDK=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            usage
            exit 1
            ;;
    esac
done

echo -e "${GREEN}Building for machine: $MACHINE${NC}"
echo -e "${GREEN}Target: $TARGET${NC}"
echo -e "${GREEN}Config: $CONFIG${NC}"

# Clean old build artifacts if needed
if [ "$CLEAN" = "true" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf build/
fi

# Build the image
echo -e "${GREEN}Starting build...${NC}"
kas build "$CONFIG" \
    --machine "$MACHINE" \
    --target "$TARGET"

# Build SDK if requested
if [ "$BUILD_SDK" = true ]; then
    echo -e "${GREEN}Building SDK...${NC}"
    kas shell "$CONFIG" --machine "$MACHINE" \
        -c "bitbake $TARGET -c populate_sdk"
fi

echo -e "${GREEN}Build complete!${NC}"
