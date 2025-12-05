#!/bin/bash
# Dynamic analysis with syscall tracing and function hooking

if [ $# -lt 1 ]; then
    echo "Usage: $0 <binary> [args...] [output_dir]"
    echo "Example: $0 /bin/cat file.txt"
    echo "Example: $0 ./malware --config test.conf output_dir"
    exit 1
fi

BINARY="$1"
shift

# Check if last arg is output dir
if [ $# -gt 0 ] && [ -d "${!#}" ]; then
    OUTPUT_DIR="${!#}"
    set -- "${@:1:$(($#-1))}"
else
    OUTPUT_DIR="trace_$(basename $BINARY)_$(date +%s)"
fi

ARGS="$@"

if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found: $BINARY"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
cd "$(dirname $0)/.."

echo "=== REKit Dynamic Analysis ==="
echo "Binary: $BINARY"
echo "Args: $ARGS"
echo "Output: $OUTPUT_DIR"
echo ""

# 1. Syscall trace
echo "[1/3] Tracing system calls..."
timeout 10s ./bin/syscall-tracer "$BINARY" $ARGS > "$OUTPUT_DIR/syscalls.txt" 2>&1 || true

# 2. Hook common functions
echo "[2/3] Hooking common functions..."
timeout 10s ./bin/dbi-advanced "$BINARY" open read write close socket connect send recv $ARGS > "$OUTPUT_DIR/hooks.txt" 2>&1 || true

# 3. Analysis
echo "[3/3] Analyzing traces..."
{
    echo "=== Trace Analysis Summary ==="
    echo "Binary: $BINARY"
    echo "Date: $(date)"
    echo ""
    
    echo "=== System Call Statistics ==="
    if [ -f "$OUTPUT_DIR/syscalls.txt" ]; then
        echo "Total syscalls: $(grep -c "syscall" "$OUTPUT_DIR/syscalls.txt" 2>/dev/null || echo 0)"
        echo ""
        echo "Top syscalls:"
        grep "syscall" "$OUTPUT_DIR/syscalls.txt" 2>/dev/null | awk '{print $2}' | sort | uniq -c | sort -rn | head -10
    fi
    echo ""
    
    echo "=== File Operations ==="
    grep -iE "open|read|write|close" "$OUTPUT_DIR/syscalls.txt" 2>/dev/null | head -20
    echo ""
    
    echo "=== Network Operations ==="
    grep -iE "socket|connect|send|recv|bind|listen" "$OUTPUT_DIR/syscalls.txt" 2>/dev/null | head -20
    echo ""
    
    echo "=== Process Operations ==="
    grep -iE "fork|exec|clone|exit" "$OUTPUT_DIR/syscalls.txt" 2>/dev/null | head -20
    
} > "$OUTPUT_DIR/trace_summary.txt"

echo ""
echo "=== Analysis Complete ==="
echo "Results saved to: $OUTPUT_DIR/"
echo ""
echo "Files created:"
echo "  - syscalls.txt       (all system calls)"
echo "  - hooks.txt          (hooked function calls)"
echo "  - trace_summary.txt  (analysis summary)"
echo ""
echo "Quick view: cat $OUTPUT_DIR/trace_summary.txt"
