#!/bin/bash
# Integration test script for CI

echo "Running integration tests..."

# Test sensor service
echo "Testing sensor service..."
sensor-service --test || exit 1

# Test device manager
echo "Testing device manager..."
device-manager --test || exit 1

# Test GUI application
echo "Testing GUI application..."
gui-app --test --headless || exit 1

# Test IPC communication
echo "Testing IPC communication..."
ipc-test || exit 1

# Test update mechanism
echo "Testing update mechanism..."
swupdate-test || exit 1

echo "All integration tests passed!"
