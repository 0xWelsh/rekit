#!/bin/bash
# Full binary analysis workflow

if [ $# -lt 1 ]; then
    echo "Usage: $0 <binary> [output_dir]"
    exit 1
fi

BINARY="$1"
OUTPUT_DIR="${2:-analysis_$(basename $BINARY)_$(date +%s)}"

if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found: $BINARY"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
cd "$(dirname $0)/.."

echo "=== REKit Full Analysis ==="
echo "Binary: $BINARY"
echo "Output: $OUTPUT_DIR"
echo ""

# Detect file type
FILE_TYPE=$(file "$BINARY" | grep -o "ELF\|PE32")

# 1. File structure
echo "[1/5] Parsing file structure..."
if [[ "$FILE_TYPE" == "ELF" ]]; then
    ./bin/elf-parser "$BINARY" > "$OUTPUT_DIR/structure.txt" 2>&1
else
    ./bin/pe-parser "$BINARY" > "$OUTPUT_DIR/structure.txt" 2>&1
fi

# 2. String extraction
echo "[2/5] Extracting strings..."
./bin/strings "$BINARY" 4 > "$OUTPUT_DIR/strings.txt" 2>&1

# 3. Interesting strings
echo "[3/5] Finding interesting strings..."
grep -iE "http|ftp|password|key|token|api|secret|admin|root|exec|cmd|shell|/bin/|\.dll|\.so" "$OUTPUT_DIR/strings.txt" > "$OUTPUT_DIR/interesting_strings.txt" 2>&1

# 4. Basic info
echo "[4/5] Gathering basic info..."
{
    echo "=== File Info ==="
    file "$BINARY"
    echo ""
    echo "=== Size ==="
    ls -lh "$BINARY"
    echo ""
    echo "=== MD5 ==="
    md5sum "$BINARY"
    echo ""
    echo "=== SHA256 ==="
    sha256sum "$BINARY"
} > "$OUTPUT_DIR/basic_info.txt"

# 5. Summary
echo "[5/5] Creating summary..."
{
    echo "=== Analysis Summary ==="
    echo "Binary: $BINARY"
    echo "Date: $(date)"
    echo "Type: $FILE_TYPE"
    echo ""
    echo "=== Statistics ==="
    echo "Total strings: $(wc -l < "$OUTPUT_DIR/strings.txt")"
    echo "Interesting strings: $(wc -l < "$OUTPUT_DIR/interesting_strings.txt")"
    echo ""
    echo "=== Entry Point ==="
    grep -i "entry" "$OUTPUT_DIR/structure.txt" | head -1
    echo ""
    echo "=== Top Interesting Strings ==="
    head -20 "$OUTPUT_DIR/interesting_strings.txt"
} > "$OUTPUT_DIR/summary.txt"

echo ""
echo "=== Analysis Complete ==="
echo "Results saved to: $OUTPUT_DIR/"
echo ""
echo "Files created:"
echo "  - basic_info.txt          (file info, hashes)"
echo "  - structure.txt           (headers, sections, symbols)"
echo "  - strings.txt             (all strings)"
echo "  - interesting_strings.txt (filtered strings)"
echo "  - summary.txt             (quick overview)"
echo ""
echo "Quick view: cat $OUTPUT_DIR/summary.txt"
