# Error Handling Documentation

## Overview
REKit tools implement comprehensive error handling for robustness and better user experience.

## Error Handling Features

### 1. Input Validation
All tools validate inputs before processing:
- File existence and accessibility
- Valid PIDs for process operations
- Reasonable size limits
- Valid parameter ranges

### 2. Descriptive Error Messages
Errors include:
- What went wrong
- Why it failed (errno message)
- Hints for resolution

### 3. JSON Error Format
When using `--json` flag, errors are returned in JSON:
```json
{
  "error": "Cannot open file: No such file or directory"
}
```

### 4. Proper Exit Codes
- `0`: Success
- `1`: Error occurred

## Tool-Specific Error Handling

### strings

**Validations:**
- File must exist and be readable
- File size limit: 100MB
- Min length range: 1-1024

**Error Examples:**
```bash
# File not found
$ ./bin/strings /nonexistent
Error: Cannot open file '/nonexistent': No such file or directory

# Invalid min length
$ ./bin/strings /bin/ls 9999
Error: Invalid min_length (must be 1-1024)

# JSON mode
$ ./bin/strings /nonexistent --json
{"error": "Cannot open file: No such file or directory"}
```

### elf-parser

**Validations:**
- File must exist and be readable
- File must be at least ELF header size
- File size limit: 500MB
- Must have valid ELF magic bytes

**Error Examples:**
```bash
# File not found
$ ./bin/elf-parser /nonexistent
Error: Cannot open file '/nonexistent': No such file or directory

# Not an ELF file
$ ./bin/elf-parser /bin/echo.txt
Error: Not a valid ELF file

# File too small
$ ./bin/elf-parser tiny_file
Error: File too small to be valid ELF

# JSON mode
$ ./bin/elf-parser /nonexistent --json
{"error": "Cannot open file: No such file or directory"}
```

### syscall-tracer

**Validations:**
- Program must exist and be executable
- Fork must succeed
- ptrace must be allowed

**Error Examples:**
```bash
# File not executable
$ ./bin/syscall-tracer /etc/passwd
Error: Cannot execute '/etc/passwd': Permission denied

# Program not found
$ ./bin/syscall-tracer /nonexistent
Error: Cannot execute '/nonexistent': No such file or directory

# ptrace failed
$ ./bin/syscall-tracer /bin/ls
Error: ptrace(TRACEME) failed: Operation not permitted
Hint: Check ptrace_scope settings
```

### memdump

**Validations:**
- PID must be valid (> 0)
- Process must exist
- Size must be reasonable (max 10MB)
- Must have permission to attach

**Error Examples:**
```bash
# Invalid PID
$ ./bin/memdump 0 0x400000 0x1000
Error: Invalid PID

# Process not found
$ ./bin/memdump 99999 0x400000 0x1000
Error: Process 99999 not found or no permission: No such process

# Permission denied
$ ./bin/memdump 1 0x400000 0x1000
Error: Cannot attach to process: Operation not permitted
Hint: Try running with sudo

# Size too large
$ ./bin/memdump 1234 0x400000 0x10000000
Error: Size too large (max 10MB)

# Invalid memory address
$ sudo ./bin/memdump 1234 0xdeadbeef 0x1000
Error: Cannot read memory at 0xdeadbeef: Input/output error
```

## Common Error Scenarios

### Permission Errors

**Problem:** Operation not permitted
```bash
Error: Cannot attach to process: Operation not permitted
```

**Solutions:**
1. Run with sudo: `sudo ./bin/memdump ...`
2. Check ptrace_scope: `cat /proc/sys/kernel/yama/ptrace_scope`
3. Temporarily allow: `echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope`

### File Access Errors

**Problem:** Cannot open file
```bash
Error: Cannot open file '/path/to/file': Permission denied
```

**Solutions:**
1. Check file permissions: `ls -l /path/to/file`
2. Run with appropriate permissions
3. Verify file exists: `file /path/to/file`

### Memory Errors

**Problem:** Memory allocation failed
```bash
Error: Memory allocation failed
```

**Solutions:**
1. Reduce size parameter
2. Check available memory: `free -h`
3. Close other applications

### Process Errors

**Problem:** Process not found
```bash
Error: Process 1234 not found or no permission
```

**Solutions:**
1. Verify PID: `ps -p 1234`
2. Check if process still running
3. Use correct PID: `pidof process_name`

## Error Handling Best Practices

### 1. Check Return Values
```bash
#!/bin/bash
if ! ./bin/strings binary > output.txt 2>&1; then
    echo "String extraction failed"
    exit 1
fi
```

### 2. Capture Errors
```bash
# Separate stdout and stderr
./bin/elf-parser binary > structure.txt 2> errors.txt

# Check for errors
if [ -s errors.txt ]; then
    echo "Errors occurred:"
    cat errors.txt
fi
```

### 3. JSON Error Handling
```python
import json
import subprocess

result = subprocess.run(
    ['./bin/strings', 'binary', '--json'],
    capture_output=True, text=True
)

data = json.loads(result.stdout)

if 'error' in data:
    print(f"Error: {data['error']}")
    exit(1)

# Process strings
for s in data['strings']:
    print(s['value'])
```

### 4. Graceful Degradation
```bash
#!/bin/bash
# Try to analyze, continue on error

for binary in samples/*; do
    if ./bin/elf-parser "$binary" > "results/${binary}.txt" 2>&1; then
        echo "✓ $binary"
    else
        echo "✗ $binary (skipped)"
    fi
done
```

## Debugging Errors

### Enable Verbose Output
```bash
# Use strace to see system calls
strace -e trace=open,read ./bin/strings binary

# Check ptrace permissions
cat /proc/sys/kernel/yama/ptrace_scope

# Verify file access
ls -l binary
file binary
```

### Common Issues

**Issue:** "Operation not permitted" with ptrace
```bash
# Check ptrace_scope
cat /proc/sys/kernel/yama/ptrace_scope
# 0 = no restrictions
# 1 = restricted (default on Ubuntu)
# 2 = admin only
# 3 = no ptrace

# Temporarily allow (until reboot)
echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
```

**Issue:** "Cannot read memory"
```bash
# Check if address is valid
cat /proc/<pid>/maps | grep <address>

# Try different address from maps
./bin/memdump <pid> <valid_address> 0x1000
```

**Issue:** "File too large"
```bash
# Check file size
ls -lh binary

# Use streaming tools for large files
strings binary | head -1000
```

## Error Codes Reference

| Exit Code | Meaning |
|-----------|---------|
| 0 | Success |
| 1 | General error |
| 2 | Invalid arguments (future) |
| 3 | Permission denied (future) |

## Logging Errors

### To File
```bash
# Log all errors
./bin/strings binary 2>> errors.log

# Log with timestamp
{
    echo "=== $(date) ==="
    ./bin/strings binary 2>&1
} >> analysis.log
```

### Structured Logging
```bash
#!/bin/bash
# Log errors in structured format

{
    echo "{"
    echo "  \"timestamp\": \"$(date -Iseconds)\","
    echo "  \"tool\": \"strings\","
    echo "  \"file\": \"$1\","
    
    if ./bin/strings "$1" > /dev/null 2>&1; then
        echo "  \"status\": \"success\""
    else
        echo "  \"status\": \"error\","
        echo "  \"error\": \"$(./bin/strings "$1" 2>&1 | tail -1)\""
    fi
    
    echo "}"
} >> analysis.log
```

## Future Enhancements

Planned improvements:
- More specific exit codes
- Verbose mode (`-v` flag)
- Warning messages (non-fatal)
- Progress indicators for long operations
- Retry logic for transient errors
- Error recovery suggestions

## Testing Error Handling

```bash
# Test invalid inputs
./bin/strings /nonexistent
./bin/strings /bin/ls 9999
./bin/elf-parser /etc/passwd
./bin/memdump 0 0x0 0x0

# Test edge cases
touch empty_file
./bin/strings empty_file

# Test permissions
chmod 000 test_file
./bin/strings test_file
chmod 644 test_file

# Test large files (if available)
./bin/strings /path/to/large/file
```

## Support

If you encounter errors not covered here:
1. Check file permissions and accessibility
2. Verify tool requirements (ptrace, memory, etc.)
3. Run with sudo if needed
4. Check system logs: `dmesg | tail`
5. Report issues with full error message
