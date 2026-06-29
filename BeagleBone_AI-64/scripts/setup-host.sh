#!/bin/bash
# Setup host system for Yocto development on BBB AI-64

set -e

# ============================================================================
# Colors for output
# ============================================================================
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
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

# ============================================================================
# Detect OS
# ============================================================================
detect_os() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        OS=$ID
        VERSION=$VERSION_ID
    else
        print_error "Cannot detect OS"
        exit 1
    fi
}

# ============================================================================
# Check requirements
# ============================================================================
check_requirements() {
    print_info "Checking system requirements..."
    
    # Check disk space (need at least 50GB free)
    AVAILABLE_SPACE=$(df -BG . | awk 'NR==2 {print $4}' | sed 's/G//')
    if [ "$AVAILABLE_SPACE" -lt 50 ]; then
        print_warning "Low disk space: ${AVAILABLE_SPACE}GB available (recommended 50GB+)"
    fi
    
    # Check memory (need at least 4GB)
    TOTAL_MEM=$(free -g | awk '/^Mem:/{print $2}')
    if [ "$TOTAL_MEM" -lt 4 ]; then
        print_warning "Low memory: ${TOTAL_MEM}GB available (recommended 8GB+)"
    fi
    
    # Check CPU cores
    CPU_CORES=$(nproc)
    print_info "CPU cores: $CPU_CORES"
}

# ============================================================================
# Install packages (Ubuntu/Debian)
# ============================================================================
install_ubuntu_packages() {
    print_header "Installing Ubuntu/Debian packages"
    
    sudo apt-get update
    
    sudo apt-get install -y \
        gawk \
        wget \
        git \
        diffstat \
        unzip \
        texinfo \
        gcc \
        build-essential \
        chrpath \
        socat \
        cpio \
        python3 \
        python3-pip \
        python3-pexpect \
        xz-utils \
        debianutils \
        iputils-ping \
        python3-git \
        python3-jinja2 \
        libegl1-mesa \
        libsdl1.2-dev \
        xterm \
        python3-subunit \
        mesa-common-dev \
        zstd \
        liblz4-tool \
        file \
        locales \
        libacl1 \
        liblzma-dev \
        libxml2-utils \
        ccache \
        curl \
        wget \
        rsync \
        tmux \
        vim \
        htop \
        \
        # Additional tools for development
        git-lfs \
        meld \
        tree \
        ncdu \
        silversearcher-ag \
        ripgrep \
        fd-find
    
    # Install repo tool
    print_info "Installing repo tool..."
    mkdir -p ~/.local/bin
    curl https://storage.googleapis.com/git-repo-downloads/repo > ~/.local/bin/repo
    chmod a+x ~/.local/bin/repo
    
    # Add to PATH if not already there
    if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
        echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
        export PATH="$HOME/.local/bin:$PATH"
    fi
    
    # Install pip packages
    print_info "Installing Python packages..."
    pip3 install --user \
        jinja2 \
        pyyaml \
        termcolor \
        argcomplete \
        coloredlogs \
        python-gitlab \
        pygithub \
        requests \
        boto3
    
    # Setup locale
    sudo locale-gen en_US.UTF-8
    sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
}

# ============================================================================
# Install packages (Fedora/RHEL)
# ============================================================================
install_fedora_packages() {
    print_header "Installing Fedora/RHEL packages"
    
    sudo dnf install -y \
        gawk \
        wget \
        git \
        diffstat \
        unzip \
        texinfo \
        gcc \
        gcc-c++ \
        chrpath \
        socat \
        cpio \
        python3 \
        python3-pip \
        python3-pexpect \
        xz-devel \
        debianutils \
        iputils \
        python3-git \
        python3-jinja2 \
        mesa-libEGL \
        SDL-devel \
        xterm \
        python3-subunit \
        mesa-libGL-devel \
        zstd \
        lz4 \
        file \
        glibc-locale-source \
        libacl-devel \
        liblzma-devel \
        libxml2-devel \
        ccache \
        curl \
        wget \
        rsync
    
    # Install repo tool
    mkdir -p ~/.local/bin
    curl https://storage.googleapis.com/git-repo-downloads/repo > ~/.local/bin/repo
    chmod a+x ~/.local/bin/repo
}

# ============================================================================
# Install packages (Arch Linux)
# ============================================================================
install_arch_packages() {
    print_header "Installing Arch Linux packages"
    
    sudo pacman -S --needed \
        gawk \
        wget \
        git \
        diffstat \
        unzip \
        texinfo \
        gcc \
        base-devel \
        chrpath \
        socat \
        cpio \
        python \
        python-pip \
        python-pexpect \
        xz \
        debianutils \
        iputils \
        python-git \
        python-jinja \
        mesa \
        sdl \
        xterm \
        python-subunit \
        mesa-utils \
        zstd \
        lz4 \
        file \
        ccache \
        curl \
        rsync
    
    # Install repo tool
    yay -S --needed repo
}

# ============================================================================
# Setup ccache
# ============================================================================
setup_ccache() {
    print_header "Setting up ccache"
    
    CCACHE_DIR="${HOME}/.ccache"
    mkdir -p ${CCACHE_DIR}
    
    # Set max cache size (20GB)
    ccache -M 20G
    
    # Add to bashrc
    if ! grep -q "ccache" ~/.bashrc; then
        echo 'export CCACHE_DIR="${HOME}/.ccache"' >> ~/.bashrc
        echo 'export PATH="/usr/lib/ccache:$PATH"' >> ~/.bashrc
        echo 'export CCACHE_COMPRESS=1' >> ~/.bashrc
        echo 'export CCACHE_COMPRESSLEVEL=6' >> ~/.bashrc
    fi
    
    print_success "ccache configured (max size: 20GB)"
}

# ============================================================================
# Setup Git
# ============================================================================
setup_git() {
    print_header "Setting up Git"
    
    # Configure Git for large repos
    git config --global core.compression 1
    git config --global core.preloadindex true
    git config --global core.fscache true
    
    # Increase buffer size for large repos
    git config --global http.postBuffer 524288000
    
    # Setup Git LFS
    if command -v git-lfs &> /dev/null; then
        git lfs install --skip-repo
    fi
    
    print_success "Git configured for large repositories"
}

# ============================================================================
# Setup environment
# ============================================================================
setup_environment() {
    print_header "Setting up environment"
    
    # Create directories
    mkdir -p ~/yocto
    mkdir -p ~/yocto/downloads
    mkdir -p ~/yocto/sstate-cache
    
    # Add aliases
    if ! grep -q "yocto-alias" ~/.bashrc; then
        cat >> ~/.bashrc << 'EOF'
# Yocto aliases
alias yocto-env='cd ~/yocto && source poky/oe-init-build-env build'
alias yocto-build='bitbake'
alias yocto-clean='bitbake -c cleanall'
alias yocto-fetch='bitbake -c fetchall'
alias yocto-sdk='bitbake -c populate_sdk'
alias yocto-menu='bitbake -c menuconfig'
EOF
    fi
    
    # Setup environment variables
    if ! grep -q "YOCTO_DIR" ~/.bashrc; then
        cat >> ~/.bashrc << 'EOF'
# Yocto environment
export YOCTO_DIR="$HOME/yocto"
export YOCTO_DOWNLOADS="$YOCTO_DIR/downloads"
export YOCTO_SSTATE="$YOCTO_DIR/sstate-cache"
export PATH="$HOME/.local/bin:$PATH"
EOF
    fi
    
    print_success "Environment configured"
}

# ============================================================================
# Setup Python virtual environment (optional)
# ============================================================================
setup_python_venv() {
    print_header "Setting up Python virtual environment"
    
    if [ "$1" == "--venv" ]; then
        python3 -m venv ~/yocto-venv
        source ~/yocto-venv/bin/activate
        pip install --upgrade pip
        pip install \
            jinja2 \
            pyyaml \
            termcolor \
            coloredlogs \
            python-gitlab \
            pygithub \
            requests \
            boto3
        
        echo "source ~/yocto-venv/bin/activate" >> ~/.bashrc
        print_success "Python virtual environment created"
    fi
}

# ============================================================================
# Main
# ============================================================================
main() {
    print_header "Yocto Host Setup for BBB AI-64"
    echo ""
    
    # Detect OS
    detect_os
    
    # Check requirements
    check_requirements
    
    # Install packages based on OS
    case $OS in
        ubuntu|debian)
            install_ubuntu_packages
            ;;
        fedora|rhel|centos)
            install_fedora_packages
            ;;
        arch|manjaro)
            install_arch_packages
            ;;
        *)
            print_error "Unsupported OS: $OS"
            exit 1
            ;;
    esac
    
    # Setup tools
    setup_ccache
    setup_git
    setup_environment
    
    # Setup Python venv if requested
    if [ "$1" == "--venv" ]; then
        setup_python_venv
    fi
    
    print_header "Setup Complete!"
    echo ""
    print_success "Host system is ready for Yocto development"
    echo ""
    print_info "Next steps:"
    echo "  1. Clone the repository: git clone <repo-url>"
    echo "  2. Run: ./scripts/sync-sources.sh"
    echo "  3. Run: ./scripts/build-image.sh"
    echo ""
    print_info "To use ccache, source your bashrc or restart terminal"
    echo ""
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of setup-host.sh
# ============================================================================
