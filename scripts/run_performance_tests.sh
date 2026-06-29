#!/bin/bash
# Performance test script for CI

echo "Running performance tests..."

# CPU performance
echo "Testing CPU performance..."
sysbench cpu run --threads=4

# Memory performance
echo "Testing Memory performance..."
sysbench memory run

# Disk performance
echo "Testing Disk performance..."
sysbench fileio prepare
sysbench fileio run
sysbench fileio cleanup

# Network performance
echo "Testing Network performance..."
iperf3 -c localhost -t 10

echo "Performance tests complete"
