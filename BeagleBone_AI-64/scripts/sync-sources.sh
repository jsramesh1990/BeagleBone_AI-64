#!/bin/bash
# Sync sources for Yocto build

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
SOURCES_DIR="$PROJECT_ROOT/sources"

# Layers and their repositories
declare -A LAYERS=(
    ["poky"]="git://git.yoctoproject.org/poky"
    ["meta-openembedded"]="git://git.openembedded.org/meta-openembedded"
    ["meta-qt6"]="https://github.com/meta-qt6/meta-qt6.git"
    ["meta-ti"]="https://git.ti.com/git/processor-sdk/meta-ti.git"
    ["meta-security"]="git://git.yoctoproject.org/meta-security"
    ["meta-virtualization"]="https://github.com/meta-virtualization/meta-virtualization.git"
    ["meta-swupdate"]="https://github.com/sbabic/meta-swupdate.git"
    ["meta-custom"]="https://github.com/yourusername/meta-custom.git"
)

# Branches
declare -A BRANCHES=(
    ["poky"]="master"
    ["meta-openembedded"]="master"
    ["meta-qt6"]="master"
    ["meta-ti"]="master"
    ["meta-security"]="master"
    ["meta-virtualization"]="master"
    ["meta-swupdate"]="master"
    ["meta-custom"]="main"
)

# ============================================================================
# Sync function
# ============================================================================
sync_layer() {
    local name="$1"
    local url="$2"
    local branch="${3:-master}"
    local dir="$SOURCES_DIR/$name"
    
    print_info "Syncing $name..."
    
    if [ -d "$dir" ]; then
        cd "$dir"
        print_info "  Updating $name..."
        git fetch --all --tags --prune
        git checkout "$branch"
        git pull --rebase origin "$branch"
        cd - > /dev/null
    else
        print_info "  Cloning $name..."
        git clone -b "$branch" "$url" "$dir"
    fi
    
    print_success "  $name synced"
}

# ============================================================================
# Main
# ============================================================================
main() {
    print_header "Syncing Yocto Sources"
    echo ""
    
    # Create sources directory
    mkdir -p "$SOURCES_DIR"
    
    # Sync all layers
    for layer in "${!LAYERS[@]}"; do
        sync_layer "$layer" "${LAYERS[$layer]}" "${BRANCHES[$layer]}"
    done
    
    # Create .repo manifest (optional)
    print_info "Creating repo manifest..."
    cat > "$SOURCES_DIR/manifest.xml" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<manifest>
    <remote name="yocto" fetch="git://git.yoctoproject.org" />
    <remote name="github" fetch="https://github.com" />
    <remote name="ti" fetch="https://git.ti.com" />
    
    <project remote="yocto" name="poky" path="poky" revision="master" />
    <project remote="yocto" name="meta-openembedded" path="meta-openembedded" revision="master" />
    <project remote="github" name="meta-qt6/meta-qt6" path="meta-qt6" revision="master" />
    <project remote="ti" name="processor-sdk/meta-ti" path="meta-ti" revision="master" />
    <project remote="yocto" name="meta-security" path="meta-security" revision="master" />
    <project remote="github" name="meta-virtualization/meta-virtualization" path="meta-virtualization" revision="master" />
    <project remote="github" name="sbabic/meta-swupdate" path="meta-swupdate" revision="master" />
    <project remote="github" name="yourusername/meta-custom" path="meta-custom" revision="main" />
</manifest>
EOF
    
    print_success "Sources synchronized!"
    echo ""
    print_info "Layer locations:"
    for layer in "${!LAYERS[@]}"; do
        echo "  - $layer: $SOURCES_DIR/$layer"
    done
    echo ""
    print_info "Next steps:"
    echo "  1. cd $PROJECT_ROOT"
    echo "  2. ./scripts/build-image.sh"
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of sync-sources.sh
# ============================================================================
