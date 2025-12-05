# REKit Automation Scripts

## Overview
Pre-built workflows for common reverse engineering tasks. All scripts are in `examples/` directory.

## Scripts

### 1. full_analysis.sh
**Purpose:** Complete static analysis of a binary  
**Safety:** Safe (no execution)

```bash
./examples/full_analysis.sh /path/to/binary [output_dir]
```

**What it does:**
- Detects file type (ELF/PE)
- Parses structure (headers, sections, symbols)
- Extracts all strings
- Filters interesting strings (URLs, passwords, etc.)
- Generates file hashes (MD5, SHA256)
- Creates summary report

**Output files:**
- `basic_info.txt` - File info and hashes
- `structure.txt` - Binary structure
- `strings.txt` - All strings
- `interesting_strings.txt` - Filtered strings
- `summary.txt` - Quick overview

**Use case:** First look at unknown binary

---

### 2. malware_triage.sh
**Purpose:** Quick malware assessment  
**Safety:** Safe (static analysis only)

```bash
./examples/malware_triage.sh suspicious_binary
```

**What it does:**
- Extracts indicators (URLs, IPs, emails)
- Finds suspicious patterns (crypto, network, execution)
- Calculates risk score
- Generates triage report with recommendations

**Output files:**
- `REPORT.txt` - Main triage report (with risk score)
- `file_info.txt` - Basic info
- `indicators.txt` - URLs, IPs, emails
- `suspicious.txt` - Suspicious strings
- `all_strings.txt` - Complete string dump

**Risk scoring:**
- LOW: < 10 indicators
- MEDIUM: 10-20 indicators
- HIGH: > 20 indicators

**Use case:** Quick malware assessment before deep analysis

---

### 3. trace_analysis.sh
**Purpose:** Dynamic analysis with tracing  
**Safety:** ⚠️ EXECUTES the binary

```bash
./examples/trace_analysis.sh <binary> [args] [output_dir]
```

**Examples:**
```bash
# Trace a command
./examples/trace_analysis.sh /bin/cat file.txt

# Trace with arguments
./examples/trace_analysis.sh ./program --config test.conf

# Custom output directory
./examples/trace_analysis.sh ./program output_dir
```

**What it does:**
- Traces all system calls
- Hooks common functions (open, read, write, socket, etc.)
- Analyzes file/network/process operations
- Generates statistics

**Output files:**
- `syscalls.txt` - All system calls
- `hooks.txt` - Hooked function calls
- `trace_summary.txt` - Analysis with statistics

**Timeout:** 10 seconds (prevents infinite loops)

**Use case:** Understanding program behavior

---

### 4. memory_analysis.sh
**Purpose:** Dump and analyze process memory  
**Safety:** ⚠️ Requires sudo, attaches to running process

```bash
./examples/memory_analysis.sh <pid|process_name>
```

**Examples:**
```bash
# By process name
./examples/memory_analysis.sh firefox

# By PID
./examples/memory_analysis.sh 1234
```

**What it does:**
- Reads process memory map
- Dumps heap and stack (64KB samples)
- Extracts strings from memory
- Lists loaded libraries
- Finds interesting strings in memory

**Output files:**
- `maps.txt` - Memory map
- `regions.txt` - Memory regions
- `heap.bin` - Heap dump
- `stack.bin` - Stack dump
- `heap_strings.txt` - Strings from heap
- `stack_strings.txt` - Strings from stack
- `analysis.txt` - Analysis report

**Use case:** Runtime memory forensics, finding credentials in memory

---

## Workflow Examples

### Analyzing Unknown Binary
```bash
# 1. Static analysis first
./examples/full_analysis.sh unknown_binary

# 2. Check the summary
cat analysis_unknown_binary_*/summary.txt

# 3. If safe, run dynamic analysis
./examples/trace_analysis.sh unknown_binary
```

### Malware Analysis Workflow
```bash
# 1. Quick triage (safe)
./examples/malware_triage.sh malware.exe

# 2. Check risk score in report
cat triage_malware.exe_*/REPORT.txt

# 3. If needed, analyze in isolated VM
# (Use REMnux - see remnux_vm_setup.md)
```

### Debugging Running Process
```bash
# 1. Find the process
ps aux | grep target_app

# 2. Dump memory
./examples/memory_analysis.sh target_app

# 3. Check for interesting strings
grep -i "password\|key" memdump_target_app_*/analysis.txt
```

### Comparing Two Binaries
```bash
# Analyze both
./examples/full_analysis.sh binary1 analysis1
./examples/full_analysis.sh binary2 analysis2

# Compare strings
diff analysis1/strings.txt analysis2/strings.txt

# Compare structure
diff analysis1/structure.txt analysis2/structure.txt
```

## Integration with Manual Tools

### After Automation
```bash
# Run full analysis
./examples/full_analysis.sh target

# Then use manual tools for deep dive
./bin/dbi-advanced target malloc free strcpy
./bin/syscall-tracer target
```

### Custom Workflows
Create your own scripts using the tools:
```bash
#!/bin/bash
# Custom analysis
./bin/elf-parser "$1" > structure.txt
./bin/strings "$1" | grep -i "http" > urls.txt
./bin/syscall-tracer "$1" > trace.txt
```

## Safety Guidelines

### Static Analysis (Safe)
- `full_analysis.sh` ✓
- `malware_triage.sh` ✓

### Dynamic Analysis (Dangerous)
- `trace_analysis.sh` ⚠️ Executes binary
- `memory_analysis.sh` ⚠️ Requires sudo

**For malware:**
1. Use static analysis first
2. Run dynamic analysis in isolated VM only
3. Never on production systems
4. Use REMnux or similar isolated environment

## Tips

### Batch Analysis
```bash
for binary in samples/*; do
    ./examples/malware_triage.sh "$binary"
done
```

### Quick Triage
```bash
# One-liner for quick check
./examples/malware_triage.sh sample.exe && cat triage_*/REPORT.txt | grep "RISK SCORE"
```

### Memory Monitoring
```bash
# Continuous monitoring
while true; do
    ./examples/memory_analysis.sh target_process
    sleep 60
done
```

### Automated Reporting
```bash
# Generate report for multiple samples
for sample in *.exe; do
    ./examples/full_analysis.sh "$sample"
    echo "Analyzed: $sample" >> report.txt
done
```

## Customization

All scripts are simple bash scripts. Customize them:

1. Edit timeout values
2. Add more string patterns
3. Change output formats
4. Add custom analysis steps

Example:
```bash
# Edit malware_triage.sh
vim examples/malware_triage.sh

# Add custom pattern
grep -iE "your_pattern" "$OUTPUT_DIR/all_strings.txt"
```

## Troubleshooting

### Permission Denied
```bash
chmod +x examples/*.sh
```

### Memory Analysis Fails
```bash
# Needs sudo
sudo ./examples/memory_analysis.sh process_name
```

### Timeout Too Short
Edit script and change:
```bash
timeout 10s  # Change to 30s or remove timeout
```

### Binary Not Found
Use absolute paths:
```bash
./examples/full_analysis.sh /full/path/to/binary
```

## Performance

- **full_analysis.sh**: ~1-5 seconds
- **malware_triage.sh**: ~2-10 seconds
- **trace_analysis.sh**: 10 seconds (timeout)
- **memory_analysis.sh**: ~5-15 seconds

## Output Management

All scripts create timestamped directories:
```
analysis_binary_1733385600/
triage_malware_1733385601/
trace_program_1733385602/
memdump_1234_1733385603/
```

Clean up old analyses:
```bash
# Remove analyses older than 7 days
find . -name "analysis_*" -mtime +7 -exec rm -rf {} \;
find . -name "triage_*" -mtime +7 -exec rm -rf {} \;
```
