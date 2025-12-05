#!/bin/bash
# Memory dump and analysis for running process

if [ $# -lt 1 ]; then
    echo "Usage: $0 <pid|process_name>"
    echo "Example: $0 1234"
    echo "Example: $0 firefox"
    exit 1
fi

INPUT="$1"
OUTPUT_DIR="memdump_${INPUT}_$(date +%s)"

# Resolve PID
if [[ "$INPUT" =~ ^[0-9]+$ ]]; then
    PID="$INPUT"
else
    PID=$(pidof "$INPUT" | awk '{print $1}')
    if [ -z "$PID" ]; then
        echo "Error: Process not found: $INPUT"
        exit 1
    fi
fi

if ! ps -p "$PID" > /dev/null 2>&1; then
    echo "Error: Process $PID not running"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
cd "$(dirname $0)/.."

PROC_NAME=$(ps -p "$PID" -o comm=)

echo "=== REKit Memory Analysis ==="
echo "Process: $PROC_NAME (PID: $PID)"
echo "Output: $OUTPUT_DIR"
echo ""

# Get memory maps
echo "[1/4] Reading memory maps..."
cp "/proc/$PID/maps" "$OUTPUT_DIR/maps.txt" 2>/dev/null || {
    echo "Error: Cannot read process memory (try sudo)"
    exit 1
}

# Dump key regions
echo "[2/4] Dumping memory regions..."
{
    echo "=== Memory Regions ==="
    cat "$OUTPUT_DIR/maps.txt"
} > "$OUTPUT_DIR/regions.txt"

# Dump heap
HEAP=$(grep "\[heap\]" "$OUTPUT_DIR/maps.txt" | head -1)
if [ -n "$HEAP" ]; then
    HEAP_START=$(echo "$HEAP" | awk '{print $1}' | cut -d'-' -f1)
    HEAP_SIZE="0x10000"  # 64KB sample
    echo "  Dumping heap at $HEAP_START..."
    sudo ./bin/memdump "$PID" "$HEAP_START" "$HEAP_SIZE" "$OUTPUT_DIR/heap.bin" 2>/dev/null || true
fi

# Dump stack
STACK=$(grep "\[stack\]" "$OUTPUT_DIR/maps.txt" | head -1)
if [ -n "$STACK" ]; then
    STACK_START=$(echo "$STACK" | awk '{print $1}' | cut -d'-' -f1)
    STACK_SIZE="0x10000"  # 64KB sample
    echo "  Dumping stack at $STACK_START..."
    sudo ./bin/memdump "$PID" "$STACK_START" "$STACK_SIZE" "$OUTPUT_DIR/stack.bin" 2>/dev/null || true
fi

# Extract strings from dumps
echo "[3/4] Extracting strings from memory..."
if [ -f "$OUTPUT_DIR/heap.bin" ]; then
    ./bin/strings "$OUTPUT_DIR/heap.bin" 4 > "$OUTPUT_DIR/heap_strings.txt" 2>&1
fi
if [ -f "$OUTPUT_DIR/stack.bin" ]; then
    ./bin/strings "$OUTPUT_DIR/stack.bin" 4 > "$OUTPUT_DIR/stack_strings.txt" 2>&1
fi

# Analysis
echo "[4/4] Analyzing memory..."
{
    echo "=== Memory Analysis Report ==="
    echo "Process: $PROC_NAME (PID: $PID)"
    echo "Date: $(date)"
    echo ""
    
    echo "=== Memory Layout ==="
    grep -E "r-xp|rw-p" "$OUTPUT_DIR/maps.txt" | head -20
    echo ""
    
    echo "=== Loaded Libraries ==="
    grep "\.so" "$OUTPUT_DIR/maps.txt" | awk '{print $6}' | sort -u
    echo ""
    
    if [ -f "$OUTPUT_DIR/heap_strings.txt" ]; then
        echo "=== Heap Strings (sample) ==="
        head -30 "$OUTPUT_DIR/heap_strings.txt"
        echo ""
    fi
    
    if [ -f "$OUTPUT_DIR/stack_strings.txt" ]; then
        echo "=== Stack Strings (sample) ==="
        head -30 "$OUTPUT_DIR/stack_strings.txt"
        echo ""
    fi
    
    echo "=== Interesting Strings ==="
    cat "$OUTPUT_DIR"/*_strings.txt 2>/dev/null | grep -iE "password|key|token|http|file|path" | head -20
    
} > "$OUTPUT_DIR/analysis.txt"

echo ""
echo "=== Analysis Complete ==="
echo "Results saved to: $OUTPUT_DIR/"
echo ""
echo "Files created:"
echo "  - maps.txt          (memory map)"
echo "  - regions.txt       (memory regions)"
echo "  - heap.bin          (heap dump)"
echo "  - stack.bin         (stack dump)"
echo "  - heap_strings.txt  (strings from heap)"
echo "  - stack_strings.txt (strings from stack)"
echo "  - analysis.txt      (analysis report)"
echo ""
echo "Quick view: cat $OUTPUT_DIR/analysis.txt"
