# JSON Output Documentation

## Overview
REKit tools support JSON output for easy integration with other tools, scripts, and analysis pipelines.

## Tools with JSON Support

### 1. strings
```bash
./bin/strings <file> [min_length] --json
```

**Output format:**
```json
{
  "tool": "strings",
  "file": "/path/to/binary",
  "strings": [
    {"offset": "0x00001000", "value": "string content"},
    {"offset": "0x00001020", "value": "another string"}
  ]
}
```

### 2. elf-parser
```bash
./bin/elf-parser <file> --json
```

**Output format:**
```json
{
  "file": "/path/to/binary",
  "type": "ELF",
  "class": "ELF64",
  "machine": "x86-64",
  "entry_point": "0x1000",
  "sections": 31,
  "segments": 13
}
```

### 3. json_analysis.sh (Complete Report)
```bash
./examples/json_analysis.sh <binary> [output.json]
```

**Output format:**
```json
{
  "binary": "/path/to/binary",
  "timestamp": "2025-12-05T09:44:15+03:00",
  "file_type": "ELF",
  "hashes": {
    "md5": "...",
    "sha1": "...",
    "sha256": "..."
  },
  "file_info": {
    "size": 35208,
    "permissions": "755",
    "type": "ELF 64-bit LSB pie executable..."
  },
  "structure": {
    "file": "/path/to/binary",
    "type": "ELF",
    "class": "ELF64",
    "machine": "x86-64",
    "entry_point": "0x3010",
    "sections": 31,
    "segments": 13
  },
  "strings": {
    "total": 170,
    "sample": [...],
    "interesting": [...]
  },
  "indicators": {
    "urls": ["https://example.com"],
    "ips": ["192.168.1.1"],
    "emails": ["user@example.com"]
  }
}
```

## Use Cases

### 1. Integration with Python
```python
import json
import subprocess

# Run analysis
result = subprocess.run(
    ['./bin/strings', 'binary', '--json'],
    capture_output=True, text=True
)

# Parse JSON
data = json.loads(result.stdout)

# Process strings
for s in data['strings']:
    print(f"{s['offset']}: {s['value']}")
```

### 2. Pipeline Processing
```bash
# Generate JSON report
./examples/json_analysis.sh malware.exe report.json

# Extract specific data with jq
cat report.json | jq '.hashes.sha256'
cat report.json | jq '.indicators.urls[]'
cat report.json | jq '.strings.interesting[]'
```

### 3. Batch Analysis
```bash
#!/bin/bash
# Analyze multiple binaries and collect results

echo "[" > results.json
first=1

for binary in samples/*; do
    if [ $first -eq 0 ]; then
        echo "," >> results.json
    fi
    first=0
    
    ./examples/json_analysis.sh "$binary" - >> results.json
done

echo "]" >> results.json
```

### 4. Database Import
```python
import json
import sqlite3

# Load JSON report
with open('report.json') as f:
    data = json.load(f)

# Insert into database
conn = sqlite3.connect('analysis.db')
c = conn.cursor()

c.execute('''INSERT INTO binaries 
             (name, md5, sha256, file_type, size)
             VALUES (?, ?, ?, ?, ?)''',
          (data['binary'], 
           data['hashes']['md5'],
           data['hashes']['sha256'],
           data['file_type'],
           data['file_info']['size']))

conn.commit()
```

### 5. Web API Integration
```python
import requests
import json

# Generate report
with open('report.json') as f:
    data = json.load(f)

# Send to API
response = requests.post(
    'https://api.example.com/analysis',
    json=data,
    headers={'Content-Type': 'application/json'}
)
```

## jq Examples

### Extract hashes
```bash
./examples/json_analysis.sh binary report.json
jq '.hashes' report.json
```

### Find URLs
```bash
jq '.indicators.urls[]' report.json
```

### Count strings
```bash
jq '.strings.total' report.json
```

### Filter interesting strings
```bash
jq '.strings.interesting[] | select(.value | contains("password"))' report.json
```

### Get entry point
```bash
jq '.structure.entry_point' report.json
```

### Compare two binaries
```bash
diff <(jq -S . binary1.json) <(jq -S . binary2.json)
```

## Automation Examples

### Monitor directory for new samples
```bash
#!/bin/bash
inotifywait -m /samples -e create |
while read path action file; do
    ./examples/json_analysis.sh "$path$file" "/reports/${file}.json"
done
```

### Generate HTML report from JSON
```python
import json
from jinja2 import Template

with open('report.json') as f:
    data = json.load(f)

template = Template('''
<html>
<h1>Analysis Report: {{ binary }}</h1>
<h2>Hashes</h2>
<ul>
  <li>MD5: {{ hashes.md5 }}</li>
  <li>SHA256: {{ hashes.sha256 }}</li>
</ul>
<h2>URLs Found</h2>
<ul>
{% for url in indicators.urls %}
  <li>{{ url }}</li>
{% endfor %}
</ul>
</html>
''')

print(template.render(**data))
```

### Elasticsearch integration
```bash
#!/bin/bash
# Index analysis results in Elasticsearch

for report in reports/*.json; do
    curl -X POST "localhost:9200/malware/_doc" \
         -H 'Content-Type: application/json' \
         -d @"$report"
done
```

## Error Handling

All tools return error in JSON format when --json flag is used:

```json
{
  "error": "Cannot open file"
}
```

Check for errors:
```bash
if jq -e '.error' report.json > /dev/null 2>&1; then
    echo "Error occurred"
    jq '.error' report.json
fi
```

## Performance

JSON output adds minimal overhead:
- strings: ~5% slower
- elf-parser: ~2% slower
- json_analysis.sh: ~10% slower (due to formatting)

## Tips

1. **Validate JSON**: Always validate with `jq` or `python -m json.tool`
2. **Pretty print**: Use `jq '.' file.json` for readable output
3. **Streaming**: For large outputs, use streaming JSON parsers
4. **Compression**: Compress JSON reports: `gzip report.json`
5. **Schema**: Define JSON schema for validation

## Schema Definition

Example JSON Schema for validation:
```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["binary", "timestamp", "hashes"],
  "properties": {
    "binary": {"type": "string"},
    "timestamp": {"type": "string", "format": "date-time"},
    "hashes": {
      "type": "object",
      "required": ["md5", "sha256"],
      "properties": {
        "md5": {"type": "string", "pattern": "^[a-f0-9]{32}$"},
        "sha256": {"type": "string", "pattern": "^[a-f0-9]{64}$"}
      }
    }
  }
}
```

## Future Enhancements

Planned JSON support for:
- syscall-tracer
- dbi-advanced
- memdump
- pe-parser
