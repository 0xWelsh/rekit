#!/bin/bash
# Complete binary analysis workflow

if [ $# -lt 1 ]; then
    echo "Usage: $0 <binary>"
    exit 1
fi

BINARY=$1
OUTPUT_DIR="analysis_$(basename $BINARY)_$(date +%s)"

mkdir -p $OUTPUT_DIR
cd $OUTPUT_DIR

echo "[*] Analyzing $BINARY"
echo

echo "[1/4] Extracting strings..."
../bin/strings $BINARY 4 > strings.txt
echo "    Saved to strings.txt"

echo "[2/4] Parsing structure..."
if file $BINARY | grep -q "PE32"; then
    ../bin/pe-parser $BINARY > structure.txt
    echo "    Saved to structure.txt"
else
    echo "    Not a PE file, skipping"
fi

echo "[3/4] Tracing syscalls..."
timeout 5 ../bin/syscall-tracer $BINARY > syscalls.txt 2>&1 || true
echo "    Saved to syscalls.txt"

echo "[4/4] Hooking common functions..."
timeout 5 ../bin/dbi-advanced $BINARY open read write close > hooks.txt 2>&1 || true
echo "    Saved to hooks.txt"

echo
echo "[+] Analysis complete: $OUTPUT_DIR"
ls -lh
