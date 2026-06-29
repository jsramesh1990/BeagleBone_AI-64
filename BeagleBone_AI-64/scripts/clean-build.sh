#!/bin/bash
# Clean Yocto build artifacts

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

# Clean levels
CLEAN_LEVEL="${1:-all}"  # all, tmp, sstate, downloads, cache

# ============================================================================
# Clean functions
# ============================================================================
clean_tmp() {
    print_info "Cleaning tmp directory..."
    if [ -d "$BUILD_DIR/tmp" ]; then
        rm -rf "$BUILD_DIR/tmp"
        print_success "tmp directory removed"
    else
        print_warning "tmp directory not found"
    fi
}

clean_sstate() {
    print_info "Cleaning sstate-cache..."
    if [ -d "$PROJECT_ROOT/sstate-cache" ]; then
        rm -rf "$PROJECT_ROOT/sstate-cache"
        print_success "sstate-cache removed"
    elif [ -d "$BUILD_DIR/sstate-cache" ]; then
        rm -rf "$BUILD_DIR/sstate-cache"
        print_success "sstate-cache removed"
    else
        print_warning "sstate-cache not found"
    fi
}

clean_downloads() {
    print_info "Cleaning downloads..."
    if [ -d "$PROJECT_ROOT/downloads" ]; then
        rm -rf "$PROJECT_ROOT/downloads"
        print_success "downloads removed"
    elif [ -d "$BUILD_DIR/downloads" ]; then
        rm -rf "$BUILD_DIR/downloads"
        print_success "downloads removed"
    else
        print_warning "downloads not found"
    fi
}

clean_cache() {
    print_info "Cleaning cache files..."
    find "$PROJECT_ROOT" -type d -name ".cache" -exec rm -rf {} + 2>/dev/null || true
    find "$PROJECT_ROOT" -type f -name "*.pyc" -delete 2>/dev/null || true
    print_success "Cache files cleaned"
}

clean_conf() {
    print_info "Cleaning build configuration..."
    if [ -f "$BUILD_DIR/conf/local.conf" ]; then
        rm -f "$BUILD_DIR/conf/local.conf"
        print_success "local.conf removed"
    fi
    if [ -f "$BUILD_DIR/conf/bblayers.conf" ]; then
        rm -f "$BUILD_DIR/conf/bblayers.conf"
        print_success "bblayers.conf removed"
    fi
    print_warning "Configuration files removed. Run build script to regenerate."
}

clean_deploy() {
    print_info "Cleaning deploy directory..."
    if [ -d "$BUILD_DIR/deploy" ]; then
        rm -rf "$BUILD_DIR/deploy"
        print_success "deploy directory removed"
    else
        print_warning "deploy directory not found"
    fi
}

# ============================================================================
# Full clean (use with caution)
# ============================================================================
clean_all() {
    print_header "Full Clean"
    print_warning "This will remove ALL build artifacts, cache, and downloads"
    print_warning "You will need to re-download all sources"
    echo ""
    read -p "Are you sure? (yes/no): " confirm
    
    if [ "$confirm" != "yes" ]; then
        print_info "Clean cancelled"
        exit 0
    fi
    
    clean_tmp
    clean_sstate
    clean_downloads
    clean_cache
    clean_deploy
    clean_conf
    
    print_success "Full clean completed"
}

# ============================================================================
# Main
# ============================================================================
main() {
    print_header "Cleaning Yocto Build"
    echo ""
    
    case $CLEAN_LEVEL in
        all|full)
            clean_all
            ;;
        tmp)
            clean_tmp
            ;;
        sstate)
            clean_sstate
            ;;
        downloads)
            clean_downloads
            ;;
        cache)
            clean_cache
            ;;
        deploy)
            clean_deploy
            ;;
        conf)
            clean_conf
            ;;
        *)
            print_error "Unknown clean level: $CLEAN_LEVEL"
            echo ""
            echo "Usage: $0 [all|tmp|sstate|downloads|cache|deploy|conf]"
            echo ""
            echo "  all       - Remove everything (tmp, sstate, downloads, cache, deploy, conf)"
            echo "  tmp       - Remove tmp directory only"
            echo "  sstate    - Remove sstate-cache only"
            echo "  downloads - Remove downloads only"
            echo "  cache     - Remove cache files only"
            echo "  deploy    - Remove deploy directory only"
            echo "  conf      - Remove configuration files only"
            exit 1
            ;;
    esac
    
    echo ""
    print_success "Clean completed!"
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of clean-build.sh
# ============================================================================
