#!/bin/bash
# Run integration tests for BBB AI-64

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
    print_header "Running Integration Tests"
    echo ""
    
    TEST_RESULTS="test_results_$(date +%Y%m%d_%H%M%S).xml"
    echo "<testsuite name=\"Integration Tests\">" > "$TEST_RESULTS"
    
    # Test Sensor Service
    print_info "Testing Sensor Service..."
    if command -v sensor-service &> /dev/null; then
        if sensor-service --test; then
            print_success "Sensor Service: PASSED"
            echo "  <testcase name=\"sensor-service\" status=\"passed\"/>" >> "$TEST_RESULTS"
        else
            print_error "Sensor Service: FAILED"
            echo "  <testcase name=\"sensor-service\" status=\"failed\"/>" >> "$TEST_RESULTS"
        fi
    else
        print_warning "Sensor Service not found"
        echo "  <testcase name=\"sensor-service\" status=\"skipped\"/>" >> "$TEST_RESULTS"
    fi
    
    # Test Device Manager
    print_info "Testing Device Manager..."
    if command -v device-manager &> /dev/null; then
        if device-manager --test; then
            print_success "Device Manager: PASSED"
            echo "  <testcase name=\"device-manager\" status=\"passed\"/>" >> "$TEST_RESULTS"
        else
            print_error "Device Manager: FAILED"
            echo "  <testcase name=\"device-manager\" status=\"failed\"/>" >> "$TEST_RESULTS"
        fi
    else
        print_warning "Device Manager not found"
        echo "  <testcase name=\"device-manager\" status=\"skipped\"/>" >> "$TEST_RESULTS"
    fi
    
    # Test GUI Application
    print_info "Testing GUI Application..."
    if command -v gui-app &> /dev/null; then
        if gui-app --test --headless; then
            print_success "GUI Application: PASSED"
            echo "  <testcase name=\"gui-app\" status=\"passed\"/>" >> "$TEST_RESULTS"
        else
            print_error "GUI Application: FAILED"
            echo "  <testcase name=\"gui-app\" status=\"failed\"/>" >> "$TEST_RESULTS"
        fi
    else
        print_warning "GUI Application not found"
        echo "  <testcase name=\"gui-app\" status=\"skipped\"/>" >> "$TEST_RESULTS"
    fi
    
    # Test IPC Communication
    print_info "Testing IPC Communication..."
    if command -v ipc-test &> /dev/null; then
        if ipc-test; then
            print_success "IPC Communication: PASSED"
            echo "  <testcase name=\"ipc\" status=\"passed\"/>" >> "$TEST_RESULTS"
        else
            print_error "IPC Communication: FAILED"
            echo "  <testcase name=\"ipc\" status=\"failed\"/>" >> "$TEST_RESULTS"
        fi
    else
        print_warning "IPC test not found"
        echo "  <testcase name=\"ipc\" status=\"skipped\"/>" >> "$TEST_RESULTS"
    fi
    
    echo "</testsuite>" >> "$TEST_RESULTS"
    
    print_success "Integration tests complete!"
    echo ""
    print_info "Results saved to: $TEST_RESULTS"
}

# ============================================================================
# Run main
# ============================================================================
main "$@"

# ============================================================================
# End of run_integration_tests.sh
# ============================================================================
