#!/bin/bash
# setup.sh - Initialize the Yocto platform for study

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}   Setting up Yocto Platform Study Environment${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════${NC}"

# Install dependencies
echo -e "\n${GREEN}Installing system dependencies...${NC}"
sudo apt-get update
sudo apt-get install -y \
    gawk wget git diffstat unzip texinfo gcc build-essential \
    chrpath socat cpio python3 python3-pip python3-pexpect \
    xz-utils debianutils iputils-ping libsdl1.2-dev xterm \
    make zstd liblz4-tool curl file

# Install kas
echo -e "\n${GREEN}Installing kas...${NC}"
pip3 install --user kas
export PATH="$HOME/.local/bin:$PATH"

# Clone required layers
echo -e "\n${GREEN}Fetching Yocto layers...${NC}"
if [ ! -d "sources/poky" ]; then
    git clone -b kirkstone git://git.yoctoproject.org/poky sources/poky
fi

if [ ! -d "sources/meta-openembedded" ]; then
    git clone -b kirkstone git://github.com/openembedded/meta-openembedded sources/meta-openembedded
fi

# Setup build environment
echo -e "\n${GREEN}Setting up build environment...${NC}"
source sources/poky/oe-init-build-env build

# Make scripts executable
chmod +x scripts/*.sh

echo -e "\n${GREEN}Setup complete!${NC}"
echo -e "\n${BLUE}Next steps:${NC}"
echo "  1. Run './scripts/study.sh' to start interactive learning"
echo "  2. Or run './scripts/build.sh -d -m qemux86-64' for a quick build"
echo "  3. Explore the layer structure in 'layers/' directory"
echo "  4. Read 'docs/getting-started.md' for more information"
