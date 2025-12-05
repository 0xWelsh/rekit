#!/bin/bash
# Generate JSON report for binary analysis

if [ $# -lt 1 ]; then
    echo "Usage: $0 <binary> [output.json]"
    exit 1
fi

BINARY="$1"
OUTPUT="${2:-analysis.json}"

if [ ! -f "$BINARY" ]; then
    echo "{\"error\": \"Binary not found: $BINARY\"}"
    exit 1
fi

cd "$(dirname $0)/.."

# Detect file type
FILE_TYPE=$(file "$BINARY" | grep -o "ELF\|PE32" | head -1)

# Generate JSON
{
    echo "{"
    echo "  \"binary\": \"$BINARY\","
    echo "  \"timestamp\": \"$(date -Iseconds)\","
    echo "  \"file_type\": \"$FILE_TYPE\","
    
    # Hashes
    echo "  \"hashes\": {"
    echo "    \"md5\": \"$(md5sum "$BINARY" | awk '{print $1}')\","
    echo "    \"sha1\": \"$(sha1sum "$BINARY" | awk '{print $1}')\","
    echo "    \"sha256\": \"$(sha256sum "$BINARY" | awk '{print $1}')\""
    echo "  },"
    
    # File info
    echo "  \"file_info\": {"
    echo "    \"size\": $(stat -c%s "$BINARY"),"
    echo "    \"permissions\": \"$(stat -c%a "$BINARY")\","
    echo "    \"type\": \"$(file -b "$BINARY" | sed 's/"/\\"/g')\""
    echo "  },"
    
    # Structure
    if [[ "$FILE_TYPE" == "ELF" ]]; then
        echo "  \"structure\": $(./bin/elf-parser "$BINARY" --json 2>/dev/null || echo '{}'),"
    else
        echo "  \"structure\": {},"
    fi
    
    # Strings (limited to first 100)
    echo "  \"strings\": {"
    TEMP_JSON=$(mktemp)
    ./bin/strings "$BINARY" 4 --json 2>/dev/null > "$TEMP_JSON"
    
    # Extract just the strings array
    if [ -s "$TEMP_JSON" ]; then
        TOTAL=$(grep -c "offset" "$TEMP_JSON" || echo 0)
        echo "    \"total\": $TOTAL,"
        echo "    \"sample\": ["
        grep "offset" "$TEMP_JSON" | head -100 | sed '$ s/,$//'
        echo "    ],"
        
        # Interesting strings
        echo "    \"interesting\": ["
        grep "offset" "$TEMP_JSON" | grep -iE "http|password|key|token|exec|shell|/bin/" | head -20 | sed '$ s/,$//'
        echo "    ]"
    else
        echo "    \"total\": 0,"
        echo "    \"sample\": [],"
        echo "    \"interesting\": []"
    fi
    rm -f "$TEMP_JSON"
    echo "  },"
    
    # Indicators
    echo "  \"indicators\": {"
    TEMP_STRINGS=$(mktemp)
    ./bin/strings "$BINARY" 4 2>/dev/null > "$TEMP_STRINGS"
    
    # URLs
    echo "    \"urls\": ["
    URLS=$(grep -oE "https?://[a-zA-Z0-9./?=_-]+" "$TEMP_STRINGS" 2>/dev/null | sort -u | head -10)
    if [ -n "$URLS" ]; then
        echo "$URLS" | awk 'BEGIN{first=1} {if(!first)printf",\n"; first=0; printf "      \"%s\"", $0} END{printf "\n"}'
    fi
    echo "    ],"
    
    # IPs
    echo "    \"ips\": ["
    IPS=$(grep -oE "\b([0-9]{1,3}\.){3}[0-9]{1,3}\b" "$TEMP_STRINGS" 2>/dev/null | sort -u | head -10)
    if [ -n "$IPS" ]; then
        echo "$IPS" | awk 'BEGIN{first=1} {if(!first)printf",\n"; first=0; printf "      \"%s\"", $0} END{printf "\n"}'
    fi
    echo "    ],"
    
    # Emails
    echo "    \"emails\": ["
    EMAILS=$(grep -oE "\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b" "$TEMP_STRINGS" 2>/dev/null | sort -u | head -10)
    if [ -n "$EMAILS" ]; then
        echo "$EMAILS" | awk 'BEGIN{first=1} {if(!first)printf",\n"; first=0; printf "      \"%s\"", $0} END{printf "\n"}'
    fi
    echo "    ]"
    
    rm -f "$TEMP_STRINGS"
    echo "  }"
    
    echo "}"
} > "$OUTPUT"

echo "JSON report saved to: $OUTPUT"
