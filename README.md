# REKit - Reverse Engineering Toolkit

A comprehensive collection of reverse engineering tools for binary analysis, dynamic instrumentation, and malware research.

## Tools

### Dynamic Binary Instrumentation (DBI)

**dbi-framework** - Basic DBI with breakpoints
```bash
./bin/dbi-framework /bin/cat 0x401000
```
- Set breakpoints at any address
- Dump registers, stack, disassembly
- Single-step execution

**dbi-advanced** - Function hooking framework
```bash
./bin/dbi-advanced /bin/cat open read write close
```
- Hook functions by name
- Trace function calls with arguments
- Automatic symbol resolution

**syscall-tracer** - System call tracer
```bash
./bin/syscall-tracer /bin/ls -la
```
- Trace all syscalls
- Show arguments and return values
- Similar to strace

### Parsers

**pe-parser** - PE/EXE file structure analyzer
```bash
./bin/pe-parser file.exe
```
- Parse DOS/PE headers
- List sections with permissions
- Show entry point, image base
- Identify x86/x64 architecture

### Analysis Tools

**strings** - Extract printable strings
```bash
./bin/strings binary.exe 4
```
- Extract ASCII strings
- Show file offsets
- Configurable minimum length

**memdump** - Memory dumper
```bash
./bin/memdump <pid> 0x400000 0x1000 output.bin
```
- Dump process memory
- Hex dump or save to file
- Attach to running process

## Building

```bash
make
```

Builds all tools to `bin/` directory.

### Requirements
- gcc
- libcapstone-dev
- Linux kernel with ptrace support

## Usage Examples

### Hook all file operations
```bash
./bin/dbi-advanced /usr/bin/cat open read close
```

### Trace a program's syscalls
```bash
./bin/syscall-tracer /bin/ls
```

### Analyze PE structure
```bash
./bin/pe-parser malware.exe
```

### Dump memory from running process
```bash
# Find PID
ps aux | grep target

# Dump memory
./bin/memdump 1234 0x400000 0x10000 dump.bin
```

### Extract strings from binary
```bash
./bin/strings suspicious.exe 6
```

## Architecture

```
rekit/
├── dbi/              Dynamic instrumentation
├── parsers/          File format parsers
├── analysis/         Static analysis tools
├── tools/            Utility tools
├── docs/             Documentation
├── examples/         Example usage
└── bin/              Compiled binaries
```

## Advanced Usage

### Combine tools for malware analysis

```bash
# 1. Extract strings
./bin/strings malware.exe > strings.txt

# 2. Parse structure
./bin/pe-parser malware.exe > structure.txt

# 3. Trace execution
./bin/syscall-tracer ./malware.exe > trace.txt

# 4. Hook suspicious functions
./bin/dbi-advanced ./malware.exe CreateFileA WriteFile
```

### Memory forensics

```bash
# Dump entire process
./bin/memdump $(pidof target) 0x400000 0x100000 full_dump.bin

# Extract strings from dump
./bin/strings full_dump.bin
```

## Extending

### Add new hooks to dbi-advanced.c
```c
dbi_hook_function(&dbi, "malloc");
dbi_hook_function(&dbi, "free");
```

### Create custom analysis tool
```c
#include <stdio.h>
// Use existing parsers as library
```

## Security Note

These tools use ptrace and require appropriate permissions:
- May need sudo for some operations
- Respect process ownership
- Use responsibly for legitimate research only

## Resources

- Documentation: `docs/`
- Examples: `examples/`
- Source code: Individual tool directories

## License

Educational and research purposes only.
