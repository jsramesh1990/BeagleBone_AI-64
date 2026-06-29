#!/bin/bash
# Compare performance results with baseline

BASELINE_FILE="$1"
CURRENT_FILE="$2"

if [ ! -f "$BASELINE_FILE" ]; then
    echo "Baseline file not found: $BASELINE_FILE"
    exit 1
fi

if [ ! -f "$CURRENT_FILE" ]; then
    echo "Current results file not found: $CURRENT_FILE"
    exit 1
fi

echo "Comparing performance..."
echo "Baseline: $BASELINE_FILE"
echo "Current: $CURRENT_FILE"

# Compare CPU performance
BASELINE_CPU=$(jq '.cpu.ops_per_sec' "$BASELINE_FILE")
CURRENT_CPU=$(jq '.cpu.ops_per_sec' "$CURRENT_FILE")
CPU_DIFF=$(echo "scale=2; ($CURRENT_CPU - $BASELINE_CPU) / $BASELINE_CPU * 100" | bc)

echo "CPU Performance: Baseline=$BASELINE_CPU, Current=$CURRENT_CPU, Diff=${CPU_DIFF}%"

# Compare Memory performance
BASELINE_MEM=$(jq '.memory.ops_per_sec' "$BASELINE_FILE")
CURRENT_MEM=$(jq '.memory.ops_per_sec' "$CURRENT_FILE")
MEM_DIFF=$(echo "scale=2; ($CURRENT_MEM - $BASELINE_MEM) / $BASELINE_MEM * 100" | bc)

echo "Memory Performance: Baseline=$BASELINE_MEM, Current=$CURRENT_MEM, Diff=${MEM_DIFF}%"

# Check if performance degraded significantly
THRESHOLD=10
if (( $(echo "$CPU_DIFF < -$THRESHOLD" | bc -l) )); then
    echo "Warning: CPU performance degraded by more than ${THRESHOLD}%"
    exit 1
fi

if (( $(echo "$MEM_DIFF < -$THRESHOLD" | bc -l) )); then
    echo "Warning: Memory performance degraded by more than ${THRESHOLD}%"
    exit 1
fi

echo "Performance comparison passed!"
exit 0
