#!/bin/bash
# Convert tool output to JSON format

TOOL="$1"
shift
ARGS="$@"

if [ -z "$TOOL" ]; then
    echo '{"error": "No tool specified"}'
    exit 1
fi

# Generate JSON output
{
    echo "{"
    echo "  \"tool\": \"$TOOL\","
    echo "  \"timestamp\": \"$(date -Iseconds)\","
    echo "  \"args\": \"$ARGS\","
    
    case "$TOOL" in
        "strings")
            echo "  \"strings\": ["
            $TOOL "$ARGS" 2>/dev/null | awk 'BEGIN{first=1} {
                if (!first) print ",";
                first=0;
                gsub(/"/, "\\\"", $0);
                offset=$1; $1="";
                printf "    {\"offset\": \"%s\", \"value\": \"%s\"}", offset, substr($0,2)
            }'
            echo ""
            echo "  ]"
            ;;
            
        "elf-parser"|"pe-parser")
            echo "  \"output\": \""
            $TOOL "$ARGS" 2>/dev/null | sed 's/"/\\"/g' | sed ':a;N;$!ba;s/\n/\\n/g'
            echo "\""
            ;;
            
        *)
            echo "  \"output\": \""
            $TOOL $ARGS 2>/dev/null | sed 's/"/\\"/g' | sed ':a;N;$!ba;s/\n/\\n/g'
            echo "\""
            ;;
    esac
    
    echo "}"
} 2>/dev/null
