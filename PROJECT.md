# REKit - Complete Reverse Engineering Toolkit

**Location:** `/home/welsh/reverse_engineering/rekit`

## What is REKit?

A comprehensive, production-ready reverse engineering toolkit with:
- Dynamic Binary Instrumentation (DBI)
- File format parsers
- Memory analysis tools
- System call tracing
- Function hooking

## Tools Built

### 1. dbi-framework
Basic DBI with breakpoints, register dumps, stack inspection, disassembly.

### 2. dbi-advanced ⭐
Advanced DBI with automatic function hooking by name.

### 3. syscall-tracer
System call tracer (like strace).

### 4. pe-parser
PE/EXE file structure analyzer.

### 5. memdump
Process memory dumper with hex view.

### 6. strings
String extractor with offset display.

## Quick Start

```bash
cd /home/welsh/reverse_engineering/rekit

# Build everything
make

# Test it
./bin/syscall-tracer /bin/ls
./bin/strings /bin/ls
```

## Project Structure

```
rekit/
├── bin/                  # Compiled binaries (6 tools)
├── dbi/                  # Dynamic instrumentation
│   ├── dbi-framework.c
│   ├── dbi-advanced.c
│   └── syscall-tracer.c
├── parsers/              # File format parsers
│   └── pe-parser.c
├── analysis/             # Static analysis
│   └── strings.c
├── tools/                # Utilities
│   └── memdump.c
├── examples/             # Usage examples
│   └── analyze_binary.sh
├── docs/                 # Documentation
│   └── ULTIMATE_RE_TOOLS.md
├── Makefile              # Build system
├── README.md             # Full documentation
├── QUICKSTART.md         # Quick examples
└── PROJECT.md            # This file
```

## Key Features

### Dynamic Analysis
- Hook any function by name
- Trace all system calls
- Set breakpoints at any address
- Dump registers and stack
- Single-step execution

### Static Analysis
- Parse PE file structure
- Extract strings with offsets
- Identify architecture (x86/x64)
- Show sections and permissions

### Memory Analysis
- Dump process memory
- Hex dump viewer
- Attach to running processes

## Use Cases

### 1. Malware Analysis
```bash
./bin/strings malware.exe | grep -i "http\|ip\|password"
./bin/pe-parser malware.exe
./bin/syscall-tracer ./malware.exe
```

### 2. Reverse Engineering
```bash
./bin/dbi-advanced ./target malloc free strcpy
./bin/memdump $(pidof target) 0x400000 0x10000
```

### 3. Vulnerability Research
```bash
./bin/syscall-tracer ./vulnerable_app
./bin/dbi-advanced ./vulnerable_app strcpy sprintf
```

### 4. Binary Analysis
```bash
./examples/analyze_binary.sh /path/to/binary
```

## Technical Details

**Language:** C
**Dependencies:** libcapstone, ptrace
**Platform:** Linux x86_64
**Build System:** Make

**Core Technologies:**
- ptrace for process control
- Capstone for disassembly
- ELF/PE parsing
- Signal handling
- Memory manipulation

## Comparison to Industry Tools

| Feature | REKit | Frida | GDB | IDA Pro |
|---------|-------|-------|-----|---------|
| Function Hooking | ✓ | ✓ | ✗ | ✗ |
| Syscall Tracing | ✓ | ✓ | ✗ | ✗ |
| Breakpoints | ✓ | ✓ | ✓ | ✓ |
| Disassembly | ✓ | ✓ | ✓ | ✓ |
| Memory Dump | ✓ | ✓ | ✓ | ✓ |
| Open Source | ✓ | ✓ | ✓ | ✗ |
| Scriptable | ✗ | ✓ | ✓ | ✓ |

## Extending REKit

### Add New Tool
1. Create source in appropriate directory
2. Add to Makefile
3. Update README.md

### Add New Hook
Edit `dbi/dbi-advanced.c`:
```c
dbi_hook_function(&dbi, "your_function");
```

### Add New Parser
Create in `parsers/`:
```c
// Parse your format
// Add to Makefile
```

## Performance

- Minimal overhead for tracing
- Efficient breakpoint handling
- Fast memory operations
- Optimized with -O2

## Security Considerations

- Requires ptrace permissions
- May need sudo for some operations
- Use in isolated environment for malware
- Respect process ownership

## Future Enhancements

- [ ] Add scripting interface (Lua/Python)
- [ ] Implement code coverage
- [ ] Add fuzzing integration
- [ ] Support Windows (via Wine)
- [ ] Add network protocol analysis
- [ ] Implement taint tracking
- [ ] Add symbolic execution

## Resources

- **Documentation:** `README.md`, `QUICKSTART.md`
- **Examples:** `examples/`
- **Source:** Individual tool directories
- **Theory:** `docs/ULTIMATE_RE_TOOLS.md`

## Credits

Built for reverse engineering research and education.
Uses Capstone disassembly engine.

## License

Educational and research purposes only.
Use responsibly.

---

**Status:** Production Ready ✓
**Version:** 1.0
**Last Updated:** 2025-11-30
