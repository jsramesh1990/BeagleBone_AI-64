#!/bin/bash
# Compare performance results with baseline

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
BASELINE_FILE="$1"
CURRENT_FILE="$2"
THRESHOLD="${3:-10}"  # Percentage threshold

# ============================================================================
# Main
# ============================================================================
main() {
    if [ -z "$BASELINE_FILE" ] || [ -z "$CURRENT_FILE" ]; then
        print_error "Usage: $0 <baseline.json> <current.json> [threshold%]"
        exit 1
    fi
    
    if [ ! -f "$BASELINE_FILE" ]; then
        print_error "Baseline file not found: $BASELINE_FILE"
        exit 1
    fi
    
    if [ ! -f "$CURRENT_FILE" ]; then
        print_error "Current file not found: $CURRENT_FILE"
        exit 1
    fi
    
    print_header "Comparing Performance"
    echo ""
    print_info "Baseline: $BASELINE_FILE"
    print_info "Current: $CURRENT_FILE"
    print_info "Threshold: ${THRESHOLD}%"
    echo ""
    
    # Check if jq is available
    if ! command -v jq &> /dev/null; then
        print_error "jq is required for JSON parsing"
        exit 1
    fi
    
    FAILED=0
    
    # Compare CPU performance
    if jq -e '.tests.cpu' "$BASELINE_FILE" > /dev/null 2>&1; then
        BASELINE_CPU=$(jq -r '.tests.cpu."events per second:"' "$BASELINE_FILE" 2>/dev/null | grep -o '[0-9.]*' | head -1)
        CURRENT_CPU=$(jq -r '.tests.cpu."events per second:"' "$CURRENT_FILE" 2>/dev/null | grep -o '[0-9.]*' | head -1)
        
        if [ -n "$BASELINE_CPU" ] && [ -n "$CURRENT_CPU" ]; then
            CPU_DIFF=$(echo "scale=2; ($CURRENT_CPU - $BASELINE_CPU) / $BASELINE_CPU * 100" | bc)
            echo "CPU Performance: Baseline=$BASELINE_CPU, Current=$CURRENT_CPU, Diff=${CPU_DIFF}%"
            
            if (( $(echo "$CPU_DIFF < -$THRESHOLD" | bc -l) )); then
                print_error "CPU performance degraded by ${CPU_DIFF}% (threshold: ${THRESHOLD}%)"
                FAILED=1
            else
                print_success "CPU performance OK (${CPU_DIFF}%)"
            fi
        fi
    fi
    
    # Compare Memory performance
    if jq -e '.tests.memory' "$BASELINE_FILE" > /dev/null 2>&1; then
        BASELINE_MEM=$(jq -r '.tests.memory."total time:"' "$BASELINE_FILE" 2>/dev/null | grep -o '[0-9.]*' | head -1)
        CURRENT_MEM=$(jq -r '.tests.memory."total time:"' "$CURRENT_FILE" 2>/dev/null | grep -o '[0-9.]*' | head -1)
        
        if [ -n "$BASELINE_MEM" ] && [ -n "$CURRENT_MEM" ]; then
            MEM_DIFF=$(echo "scale=2; ($CURRENT_MEM - $BASELINE_MEM) / $BASELINE_MEM * 100" | bc)
            echo "Memory Performance: Baseline=${BASELINE_MEM}s, Current=${CURRENT_MEM}s, Diff=${MEM_DIFF}%"
            
            if (( $(echo "$MEM_DIFF > $THRESHOLD" | bc -l) )); then
                print_error "Memory performance degraded by ${MEM_DIFF}% (threshold: ${THRESHOLD}%)"
                FAILED=1
            else
                print_success "Memory performance OK (${MEM_DIFF}%)"
            fi
        fi
    fi
    
    # Compare Disk performance
    if jq -e '.tests.disk' "$BASELINE_FILE" > /dev/null 2>&1; then
        BASELINE_READ=$(jq -r '.tests.disk."read:"' "$BASELINE_FILE" 2>/dev/null | grep -o '[0-9.]*' | head -1)
        CURRENT_READ=$(jq -r '.tests.disk."read:"' "$CURRENT_FILE" 2>/dev/null | grep -o '[0-9.]*' | head -1)
        
        if [ -n "$BASELINE_READ" ] && [ -n "$CURRENT_READ" ]; then
            READ_DIFF=$(echo "scale=2; ($CURRENT_READ - $BASELINE_READ) / $BASELINE_READ * 100" | bc)
            echo "Disk Read Performance: Baseline=$BASELINE_READ, Current=$CURRENT_READ, Diff=${READ_DIFF}%"
            
            if (( $(echo "$READ_DIFF < -$THRESHOLD" | bc -l) )); then
                print_error "Disk read performance degraded by ${READ_DIFF}% (threshold: ${THRESHOLD}%)"
                FAILED=1
            else
                print_success "Disk read performance OK (${READ_DIFF}%)"
            fi
        fi
    fi
    
    if [ $FAILED -eq 1 ]; then
        print_error "Performance comparison FAILED!"
        exit 1
    else
        print_success "Performance comparison PASSED!"
        exit 0
    fi
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of compare_performance.sh
# ============================================================================
