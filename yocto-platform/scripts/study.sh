#!/bin/bash
# scripts/study.sh - Interactive Yocto learning environment

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

show_menu() {
    echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
    echo -e "${GREEN}   Yocto Platform Study Environment${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
    echo "1. Build for QEMU (ARM64)"
    echo "2. Build for QEMU (x86_64)"
    echo "3. Build with debug symbols"
    echo "4. Run QEMU image"
    echo "5. Explore layer structure"
    echo "6. Check layer dependencies"
    echo "7. Generate dependency graph"
    echo "8. Analyze package contents"
    echo "9. Clean build"
    echo "10. Exit"
    echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
}

explore_layers() {
    echo -e "${YELLOW}Exploring layer structure...${NC}"
    
    echo -e "\n${GREEN}=== Available Layers ===${NC}"
    find layers -name "layer.conf" -exec dirname {} \; | sed 's|layers/||'
    
    echo -e "\n${GREEN}=== Layer Dependencies ===${NC}"
    for layer in layers/*/conf/layer.conf; do
        if [ -f "$layer" ]; then
            echo -e "\n${BLUE}$(basename $(dirname $(dirname $layer)))${NC}"
            grep "LAYERDEPENDS" "$layer" || echo "  No dependencies"
            grep "LAYERDEPENDS_" "$layer" || true
        fi
    done
}

generate_deps() {
    echo -e "${YELLOW}Generating dependency graph...${NC}"
    
    if ! command -v dot &> /dev/null; then
        echo "Graphviz not installed. Install with: sudo apt-get install graphviz"
        return
    fi
    
    bitbake -g core-image-base
    dot -Tpng task-depends.dot -o task-deps.png
    echo -e "${GREEN}Graph generated: task-deps.png${NC}"
}

analyze_packages() {
    echo -e "${YELLOW}Analyzing package contents...${NC}"
    
    echo -e "\n${GREEN}=== Installed Packages ===${NC}"
    find build/tmp/deploy/rpm -name "*.rpm" -exec basename {} \; | sort | head -20
    
    echo -e "\n${GREEN}=== Package Sizes ===${NC}"
    find build/tmp/deploy/rpm -name "*.rpm" -exec ls -lh {} \; | awk '{print $5 "\t" $9}' | sort -hr | head -10
}

run_qemu() {
    MACHINE=${1:-qemuarm64}
    echo -e "${YELLOW}Starting QEMU for $MACHINE...${NC}"
    
    IMAGE="build/tmp/deploy/images/$MACHINE/core-image-base-$MACHINE.qemuboot.conf"
    if [ -f "$IMAGE" ]; then
        runqemu nographic
    else
        echo -e "${RED}Image not found. Please build first.${NC}"
    fi
}

# Main loop
while true; do
    show_menu
    read -p "Choose an option: " choice
    
    case $choice in
        1)
            ./scripts/build.sh -m qemuarm64 -t core-image-base
            ;;
        2)
            ./scripts/build.sh -m qemux86-64 -t core-image-base
            ;;
        3)
            ./scripts/build.sh -d -m qemux86-64
            ;;
        4)
            run_qemu
            ;;
        5)
            explore_layers
            ;;
        6)
            bitbake-layers show-layers
            ;;
        7)
            generate_deps
            ;;
        8)
            analyze_packages
            ;;
        9)
            echo -e "${YELLOW}Cleaning build...${NC}"
            rm -rf build/tmp
            ;;
        10)
            echo -e "${GREEN}Goodbye!${NC}"
            exit 0
            ;;
        *)
            echo -e "${RED}Invalid option${NC}"
            ;;
    esac
    
    echo
    read -p "Press Enter to continue..."
    clear
done
