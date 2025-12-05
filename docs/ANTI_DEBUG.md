# Anti-Debug Detection

## Overview
The anti-debug detector identifies common anti-debugging techniques used by malware and protected software to detect and evade analysis.

## Usage

```bash
./bin/anti-debug-detect <binary> [--json]
```

**Examples:**
```bash
# Analyze a binary
./bin/anti-debug-detect suspicious.exe

# JSON output
./bin/anti-debug-detect malware --json
```

## Detected Techniques

### 1. Ptrace Detection
**What it is:** Checks if a debugger is attached using ptrace

**Indicators:**
- Strings: `ptrace`, `PTRACE`, `PT_DENY_ATTACH`
- Strings: `debugger`, `IsDebuggerPresent`

**How it works:**
- Linux: `ptrace(PTRACE_TRACEME, 0, 0, 0)` fails if already traced
- Checks `/proc/self/status` for TracerPid

**Bypass:**
- Patch ptrace calls
- Use LD_PRELOAD to hook ptrace
- Modify kernel ptrace behavior

### 2. Timing Checks
**What it is:** Measures execution time to detect single-stepping

**Indicators:**
- Strings: `rdtsc`, `RDTSC`, `clock_gettime`
- Strings: `gettimeofday`, `QueryPerformanceCounter`

**How it works:**
- Measures time before and after code block
- If time difference is large, debugger detected
- RDTSC instruction reads CPU timestamp counter

**Bypass:**
- Patch timing checks
- Hook timing functions to return consistent values
- Use hardware breakpoints instead of single-stepping

### 3. Breakpoint Detection
**What it is:** Scans own code for INT3 (0xCC) instructions

**Indicators:**
- High count of INT3 (0xCC) bytes in code
- More than 10 INT3 instructions is suspicious

**How it works:**
- Software breakpoints replace instruction with INT3 (0xCC)
- Program scans its own code for 0xCC bytes
- Calculates checksum of code sections

**Bypass:**
- Use hardware breakpoints (limited to 4)
- Restore original bytes after breakpoint hit
- Use memory breakpoints

### 4. Parent Process Check
**What it is:** Checks if parent process is a debugger

**Indicators:**
- Strings: `getppid`, `PPID`
- Strings: `/proc/self/status`, `TracerPid`

**How it works:**
- Gets parent process ID
- Checks if parent is gdb, lldb, etc.
- Reads `/proc/self/status` for TracerPid field

**Bypass:**
- Launch from shell, not directly from debugger
- Use process injection
- Modify /proc filesystem

### 5. LD_PRELOAD Check
**What it is:** Detects library injection/hooking

**Indicators:**
- Strings: `LD_PRELOAD`, `LD_DEBUG`
- Strings: `/proc/self/maps`

**How it works:**
- Checks LD_PRELOAD environment variable
- Scans `/proc/self/maps` for unexpected libraries
- Verifies function pointers

**Bypass:**
- Unset LD_PRELOAD after injection
- Use direct syscalls instead of libc
- Inject at lower level (kernel module)

## Output Format

### Text Output
```
=== Anti-Debug Detection Report ===
File: suspicious_binary

Techniques Detected:
  [X] Ptrace detection
  [X] Timing checks
  [ ] Breakpoint detection
  [X] Parent process check
  [ ] LD_PRELOAD check

Statistics:
  INT3 instructions: 5
  Suspicious strings: 8

Risk Assessment:
  ⚠ MEDIUM - Multiple techniques detected
```

### JSON Output
```json
{
  "file": "suspicious_binary",
  "anti_debug_detected": true,
  "techniques": {
    "ptrace_detection": true,
    "timing_checks": true,
    "breakpoint_detection": false,
    "parent_process_check": true,
    "ld_preload_check": false
  },
  "statistics": {
    "int3_instructions": 5,
    "suspicious_strings": 8,
    "risk_score": 3
  }
}
```

## Risk Scoring

| Score | Level | Description |
|-------|-------|-------------|
| 0 | None | No anti-debug detected |
| 1-2 | LOW | Basic protection |
| 3-4 | MEDIUM | Multiple techniques |
| 5 | HIGH | Heavily protected |

## Common Patterns

### Malware
- High risk score (4-5)
- Multiple techniques combined
- Obfuscated anti-debug code
- Custom/unknown techniques

### Legitimate Software
- Low risk score (0-2)
- Standard protection (ptrace only)
- Clear anti-debug code
- Well-known techniques

### Packed Binaries
- Often include anti-debug
- UPX, VMProtect, Themida
- Check before unpacking
- May need to bypass first

## Bypassing Anti-Debug

### 1. Patching
```bash
# Find ptrace calls
objdump -d binary | grep ptrace

# Patch with NOPs
# Replace call with 0x90 (NOP)
```

### 2. LD_PRELOAD Hook
```c
// fake_ptrace.c
long ptrace(int request, pid_t pid, void *addr, void *data) {
    // Always return success
    return 0;
}

// Compile and use
gcc -shared -fPIC fake_ptrace.c -o fake_ptrace.so
LD_PRELOAD=./fake_ptrace.so ./binary
```

### 3. GDB Scripts
```gdb
# Skip ptrace checks
catch syscall ptrace
commands
  set $rax = 0
  continue
end

# Handle timing checks
set $rdtsc_value = 0
```

### 4. Kernel Modifications
```bash
# Disable ptrace restrictions
echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope

# Hide debugger from /proc
# Requires kernel module
```

## Integration with REKit

### With Full Analysis
```bash
# Check for anti-debug first
./bin/anti-debug-detect malware > antidebug.txt

# If detected, prepare bypass
if grep -q "anti_debug_detected.*true" antidebug.txt; then
    echo "Anti-debug detected, using bypass..."
    LD_PRELOAD=./bypass.so ./bin/syscall-tracer malware
else
    ./bin/syscall-tracer malware
fi
```

### With Automation
```bash
# Add to analysis pipeline
./examples/full_analysis.sh binary
./bin/anti-debug-detect binary >> analysis_*/antidebug.txt
```

### With JSON Pipeline
```bash
# Combine with other analysis
{
    echo "{"
    echo "  \"structure\": $(./bin/elf-parser binary --json),"
    echo "  \"anti_debug\": $(./bin/anti-debug-detect binary --json)"
    echo "}"
} > complete_analysis.json
```

## Advanced Techniques (Not Detected)

These require dynamic analysis:

1. **Hardware Breakpoint Detection**
   - Checks DR0-DR7 registers
   - Requires runtime analysis

2. **Exception Handling**
   - Triggers exceptions to detect debugger
   - Needs execution to detect

3. **Code Checksums**
   - Calculates hash of code sections
   - Can detect modifications

4. **VM Detection**
   - Checks for virtual machine
   - CPUID, timing, artifacts

5. **Sandbox Detection**
   - Looks for analysis environment
   - File paths, processes, registry

## False Positives

Some legitimate software may trigger detection:

- **Timing functions**: Used for performance monitoring
- **INT3 instructions**: Used for assertions/debugging
- **Process checks**: Used for licensing

Always verify with manual analysis.

## Examples

### Clean Binary
```bash
$ ./bin/anti-debug-detect /bin/echo
=== Anti-Debug Detection Report ===
File: /bin/echo

Techniques Detected:
  [ ] Ptrace detection
  [ ] Timing checks
  [ ] Breakpoint detection
  [ ] Parent process check
  [ ] LD_PRELOAD check

Statistics:
  INT3 instructions: 3
  Suspicious strings: 0

Risk Assessment:
  ✓ No anti-debug techniques detected
```

### Protected Binary
```bash
$ ./bin/anti-debug-detect malware
=== Anti-Debug Detection Report ===
File: malware

Techniques Detected:
  [X] Ptrace detection
  [X] Timing checks
  [X] Breakpoint detection
  [X] Parent process check
  [X] LD_PRELOAD check

Statistics:
  INT3 instructions: 45
  Suspicious strings: 15

Risk Assessment:
  ⚠ HIGH - Heavily protected binary
```

## Tips

1. **Check before analysis**: Run detector before dynamic analysis
2. **Combine with strings**: Look for anti-debug strings manually
3. **Test bypasses**: Verify bypass works before full analysis
4. **Document findings**: Note which techniques are used
5. **Update patterns**: Add new anti-debug patterns as discovered

## References

- Linux ptrace man page: `man ptrace`
- Anti-debugging techniques: https://anti-debug.checkpoint.com/
- Bypassing anti-debug: Various CTF writeups
- REKit anti-debug directory: `/home/welsh/reverse_engineering/anti_debug`

## Future Enhancements

Planned features:
- Dynamic detection (runtime analysis)
- Automatic bypass generation
- More technique patterns
- Windows anti-debug detection
- VM/sandbox detection
- Detailed disassembly of anti-debug code
