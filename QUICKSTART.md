# REKit Quick Start

## Installation

```bash
cd /home/welsh/reverse_engineering/rekit
make
```

All tools are now in `bin/`

## Quick Examples

### 1. Trace System Calls
```bash
./bin/syscall-tracer /bin/ls
```

Output:
```
execve(59, 0x..., 0x..., 0x...) = 0
brk(12, 0x0, 0x..., 0x...) = 0x...
open(2, 0x..., 0x..., 0x...) = 3
read(0, 0x3, 0x..., 0x...) = 832
```

### 2. Hook Functions
```bash
./bin/dbi-advanced /bin/cat open read close
```

Output:
```
[+] Hooked open at 0x7f...
[+] Hooked read at 0x7f...
[+] Hooked close at 0x7f...

[HOOK] open()
  RDI: 0x... RSI: 0x0 RDX: 0x0

[HOOK] read()
  RDI: 0x3 RSI: 0x... RDX: 0x2000
```

### 3. Extract Strings
```bash
./bin/strings /bin/ls 4
```

Output:
```
0x00002000: /lib64/ld-linux-x86-64.so.2
0x00002020: libc.so.6
0x00002030: printf
0x00002038: strcmp
```

### 4. Analyze PE File
```bash
./bin/pe-parser malware.exe
```

Output:
```
DOS Header:
   Magic: MZ (0x5A4D)
   PE Offset: 0xF8

COFF File Header:
   Machine: 0x8664 (x64)
   Sections: 6
   Entry Point: 0x1400
```

### 5. Dump Process Memory
```bash
# Get PID
ps aux | grep firefox

# Dump memory
./bin/memdump 1234 0x400000 0x1000
```

Output:
```
0x00400000: 4d 5a 90 00 03 00 00 00  |MZ......|
0x00400008: 04 00 00 00 ff ff 00 00  |........|
```

## Complete Analysis Workflow

```bash
./examples/analyze_binary.sh /path/to/binary
```

Creates directory with:
- strings.txt
- structure.txt
- syscalls.txt
- hooks.txt

## Common Use Cases

### Malware Analysis
```bash
# Safe analysis without execution
./bin/strings malware.exe > strings.txt
./bin/pe-parser malware.exe > structure.txt

# Dynamic analysis (use VM!)
./bin/syscall-tracer ./malware.exe
./bin/dbi-advanced ./malware.exe CreateFileA WriteFile
```

### Reverse Engineering
```bash
# Find interesting strings
./bin/strings target | grep -i password

# Hook crypto functions
./bin/dbi-advanced ./target AES_encrypt MD5_Init
```

### Memory Forensics
```bash
# Dump running process
./bin/memdump $(pidof target) 0x400000 0x100000 dump.bin

# Analyze dump
./bin/strings dump.bin
```

## Tool Comparison

| Tool | Purpose | Input | Output |
|------|---------|-------|--------|
| syscall-tracer | Trace syscalls | Program | Call log |
| dbi-advanced | Hook functions | Program + names | Hook log |
| strings | Extract strings | Binary file | String list |
| pe-parser | Parse PE | EXE file | Structure |
| memdump | Dump memory | PID + address | Memory dump |

## Tips

1. **Use timeout** for long-running traces:
   ```bash
   timeout 10 ./bin/syscall-tracer ./program
   ```

2. **Combine with grep**:
   ```bash
   ./bin/strings binary | grep -i "http\|password\|key"
   ```

3. **Redirect output**:
   ```bash
   ./bin/dbi-advanced ./program open > hooks.log 2>&1
   ```

4. **Run in VM** for malware:
   - Never run malware on host
   - Use snapshots
   - Isolate network

## Next Steps

- Read full documentation: `README.md`
- Check examples: `examples/`
- Extend tools: Modify source in `dbi/`, `parsers/`, etc.
- Learn Frida for production use
