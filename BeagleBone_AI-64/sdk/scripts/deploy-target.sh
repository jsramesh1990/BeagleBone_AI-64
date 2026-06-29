#!/bin/bash
# Deploy to Target Script for BBB AI-64 SDK
# This script deploys applications to the target board

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
SDK_DIR="$(dirname "$SCRIPT_DIR")"

# Default values
TARGET_USER="${1:-root}"
TARGET_IP="${2:-192.168.7.2}"
TARGET_PORT="${3:-22}"
TARGET_DIR="${4:-/tmp}"
FILE_TO_DEPLOY="${5:-./app}"
REMOTE_NAME="${6:-$(basename "$FILE_TO_DEPLOY")}"
DEPLOY_METHOD="${7:-scp}"  # scp, rsync, ssh-copy-id

# ============================================================================
# Usage
# ============================================================================
show_usage() {
    cat << EOF
${CYAN}BBB AI-64 Deploy Script${NC}

Usage: $0 [OPTIONS] [TARGET_IP]

Options:
    -u, --user USER      Target SSH user (default: root)
    -p, --port PORT      Target SSH port (default: 22)
    -d, --dir DIR        Target directory (default: /tmp)
    -f, --file FILE      File to deploy (default: ./app)
    -n, --name NAME      Remote file name (default: basename of file)
    -m, --method METHOD  Deploy method: scp, rsync (default: scp)
    -r, --run            Run the application after deployment
    -h, --help           Show this help message
    -l, --list           List available targets

Examples:
    $0 192.168.7.2                      # Deploy to default target
    $0 -u root -f ./myapp 192.168.7.2   # Deploy with options
    $0 -m rsync -r 192.168.7.2          # Deploy with rsync and run
    $0 -l                               # List available targets

EOF
}

# ============================================================================
# List Targets
# ============================================================================
list_targets() {
    echo "${CYAN}Available Targets:${NC}"
    echo ""
    echo "  192.168.7.2   - Default BBB AI-64 (USB Ethernet)"
    echo "  192.168.1.100 - Network Ethernet"
    echo "  bbb-ai64.local - mDNS/DNS-SD name"
    echo ""
    echo "To add targets, edit the TARGETS array in this script."
}

# ============================================================================
# Check Target Connectivity
# ============================================================================
check_target() {
    print_info "Checking target connectivity..."
    
    if ping -c 1 -W 1 "$TARGET_IP" > /dev/null 2>&1; then
        print_success "Target $TARGET_IP is reachable"
        return 0
    else
        print_warning "Target $TARGET_IP is not responding to ping"
        return 1
    fi
}

# ============================================================================
# Check SSH Connectivity
# ============================================================================
check_ssh() {
    print_info "Checking SSH connectivity..."
    
    if ssh -o ConnectTimeout=5 -o BatchMode=yes -p "$TARGET_PORT" "$TARGET_USER@$TARGET_IP" "exit" 2>/dev/null; then
        print_success "SSH connection successful"
        return 0
    else
        print_warning "SSH connection failed"
        return 1
    fi
}

# ============================================================================
# Deploy with SCP
# ============================================================================
deploy_scp() {
    print_info "Deploying with SCP..."
    
    local file_size=$(du -h "$FILE_TO_DEPLOY" | cut -f1)
    print_info "File size: $file_size"
    
    # Copy the file
    if scp -P "$TARGET_PORT" "$FILE_TO_DEPLOY" "$TARGET_USER@$TARGET_IP:$TARGET_DIR/$REMOTE_NAME"; then
        print_success "File deployed to $TARGET_DIR/$REMOTE_NAME"
        return 0
    else
        print_error "SCP deployment failed"
        return 1
    fi
}

# ============================================================================
# Deploy with Rsync
# ============================================================================
deploy_rsync() {
    print_info "Deploying with Rsync..."
    
    local file_size=$(du -h "$FILE_TO_DEPLOY" | cut -f1)
    print_info "File size: $file_size"
    
    # Set permissions
    chmod +x "$FILE_TO_DEPLOY" 2>/dev/null || true
    
    # Copy the file
    if rsync -avz -e "ssh -p $TARGET_PORT" "$FILE_TO_DEPLOY" "$TARGET_USER@$TARGET_IP:$TARGET_DIR/$REMOTE_NAME"; then
        print_success "File deployed to $TARGET_DIR/$REMOTE_NAME"
        return 0
    else
        print_error "Rsync deployment failed"
        return 1
    fi
}

# ============================================================================
# Run Application on Target
# ============================================================================
run_on_target() {
    print_info "Running application on target..."
    
    ssh -p "$TARGET_PORT" "$TARGET_USER@$TARGET_IP" "chmod +x $TARGET_DIR/$REMOTE_NAME && $TARGET_DIR/$REMOTE_NAME"
    
    print_success "Application execution complete"
}

# ============================================================================
# Interactive SSH Session
# ============================================================================
ssh_to_target() {
    print_info "Starting SSH session to target..."
    ssh -p "$TARGET_PORT" "$TARGET_USER@$TARGET_IP"
}

# ============================================================================
# Main
# ============================================================================
main() {
    local run_after=false
    local ssh_session=false
    
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -u|--user)
                TARGET_USER="$2"
                shift 2
                ;;
            -p|--port)
                TARGET_PORT="$2"
                shift 2
                ;;
            -d|--dir)
                TARGET_DIR="$2"
                shift 2
                ;;
            -f|--file)
                FILE_TO_DEPLOY="$2"
                shift 2
                ;;
            -n|--name)
                REMOTE_NAME="$2"
                shift 2
                ;;
            -m|--method)
                DEPLOY_METHOD="$2"
                shift 2
                ;;
            -r|--run)
                run_after=true
                shift
                ;;
            -s|--ssh)
                ssh_session=true
                shift
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            -l|--list)
                list_targets
                exit 0
                ;;
            *)
                TARGET_IP="$1"
                shift
                ;;
        esac
    done
    
    # SSH session only
    if [ "$ssh_session" = "true" ]; then
        ssh_to_target
        exit 0
    fi
    
    print_header "BBB AI-64 Deploy Script"
    echo ""
    print_info "Target: $TARGET_USER@$TARGET_IP:$TARGET_PORT"
    print_info "Remote Directory: $TARGET_DIR"
    print_info "File: $FILE_TO_DEPLOY -> $REMOTE_NAME"
    print_info "Method: $DEPLOY_METHOD"
    echo ""
    
    # Check if file exists
    if [ ! -e "$FILE_TO_DEPLOY" ]; then
        print_error "File not found: $FILE_TO_DEPLOY"
        exit 1
    fi
    
    # Check target connectivity
    if ! check_target; then
        print_warning "Target may be unreachable, but continuing..."
    fi
    
    # Check SSH connectivity
    if ! check_ssh; then
        print_error "SSH connection failed. Please verify:"
        echo "  - Target IP: $TARGET_IP"
        echo "  - SSH port: $TARGET_PORT"
        echo "  - User: $TARGET_USER"
        echo ""
        print_info "You may need to set up SSH keys:"
        echo "  ssh-copy-id -p $TARGET_PORT $TARGET_USER@$TARGET_IP"
        exit 1
    fi
    
    # Deploy based on method
    case $DEPLOY_METHOD in
        scp)
            deploy_scp || exit 1
            ;;
        rsync)
            deploy_rsync || exit 1
            ;;
        ssh-copy-id)
            print_info "Setting up SSH keys..."
            ssh-copy-id -p "$TARGET_PORT" "$TARGET_USER@$TARGET_IP"
            ;;
        *)
            print_error "Unknown deploy method: $DEPLOY_METHOD"
            exit 1
            ;;
    esac
    
    # Run if requested
    if [ "$run_after" = "true" ]; then
        echo ""
        run_on_target
    fi
    
    echo ""
    print_success "Deployment complete!"
    echo ""
    print_info "To run the application:"
    echo "  ssh -p $TARGET_PORT $TARGET_USER@$TARGET_IP $TARGET_DIR/$REMOTE_NAME"
    echo ""
    print_info "To start a shell on the target:"
    echo "  $0 -s"
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of deploy-target.sh
# ============================================================================
