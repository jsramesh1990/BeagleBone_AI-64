#!/bin/bash
# Run performance tests for BBB AI-64

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
# Main
# ============================================================================
main() {
    print_header "Running Performance Tests"
    echo ""
    
    # Check if running on BBB AI-64
    if [ ! -f /sys/firmware/devicetree/base/model ] || ! grep -q "BeagleBone AI-64" /sys/firmware/devicetree/base/model; then
        print_warning "Not running on BBB AI-64. Some tests may not work."
    fi
    
    RESULTS_FILE="performance_results_$(date +%Y%m%d_%H%M%S).json"
    echo "{" > "$RESULTS_FILE"
    echo "  \"timestamp\": \"$(date -Iseconds)\"," >> "$RESULTS_FILE"
    echo "  \"tests\": {" >> "$RESULTS_FILE"
    
    # CPU performance
    print_info "Testing CPU performance..."
    if command -v sysbench &> /dev/null; then
        echo "    \"cpu\": {" >> "$RESULTS_FILE"
        sysbench cpu run --threads=4 --time=10 | grep -E "events per second|total time" | while read line; do
            echo "      \"$line\"" >> "$RESULTS_FILE"
        done
        echo "    }," >> "$RESULTS_FILE"
    fi
    
    # Memory performance
    print_info "Testing Memory performance..."
    if command -v sysbench &> /dev/null; then
        echo "    \"memory\": {" >> "$RESULTS_FILE"
        sysbench memory run --threads=4 --time=10 | grep -E "transferred|total time" | while read line; do
            echo "      \"$line\"" >> "$RESULTS_FILE"
        done
        echo "    }," >> "$RESULTS_FILE"
    fi
    
    # Disk performance
    print_info "Testing Disk performance..."
    if command -v sysbench &> /dev/null; then
        echo "    \"disk\": {" >> "$RESULTS_FILE"
        sysbench fileio prepare
        sysbench fileio run --threads=4 --time=10 | grep -E "read|write|total" | while read line; do
            echo "      \"$line\"" >> "$RESULTS_FILE"
        done
        sysbench fileio cleanup
        echo "    }," >> "$RESULTS_FILE"
    fi
    
    # Network performance
    print_info "Testing Network performance..."
    if command -v iperf3 &> /dev/null; then
        echo "    \"network\": {" >> "$RESULTS_FILE"
        # Start iperf3 server in background
        iperf3 -s -D
        sleep 2
        iperf3 -c localhost -t 10 | grep -E "sender|receiver" | while read line; do
            echo "      \"$line\"" >> "$RESULTS_FILE"
        done
        pkill iperf3
        echo "    }" >> "$RESULTS_FILE"
    fi
    
    echo "  }" >> "$RESULTS_FILE"
    echo "}" >> "$RESULTS_FILE"
    
    print_success "Performance tests complete!"
    echo ""
    print_info "Results saved to: $RESULTS_FILE"
    cat "$RESULTS_FILE"
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of run_performance_tests.sh
# ============================================================================
