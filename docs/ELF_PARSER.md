# ELF Parser Documentation

## Overview
The ELF parser analyzes Linux/Unix executable and shared library files, extracting structural information useful for reverse engineering.

## Usage
```bash
./bin/elf-parser <elf_file>
```

## Output Sections

### 1. ELF Header
- **Magic**: ELF identification bytes (7f 45 4c 46)
- **Class**: 32-bit or 64-bit binary
- **Data**: Endianness (Little/Big Endian)
- **Type**: Executable, Shared Object, or Relocatable
- **Machine**: Architecture (x86, x86-64, ARM, ARM64)
- **Entry Point**: Program start address
- **Header counts**: Number of program and section headers

### 2. Program Headers (Segments)
Shows how the binary is loaded into memory:
- **LOAD**: Loadable segments (code, data)
- **DYNAMIC**: Dynamic linking information
- **INTERP**: Path to dynamic linker
- **GNU_STACK**: Stack permissions
- **Flags**: R (read), W (write), X (execute)

### 3. Section Headers
Detailed view of binary sections:
- **.text**: Executable code
- **.data**: Initialized data
- **.bss**: Uninitialized data
- **.rodata**: Read-only data (strings, constants)
- **.plt/.got**: Procedure Linkage Table / Global Offset Table
- **.symtab/.dynsym**: Symbol tables
- **Flags**: W (writable), A (allocatable), X (executable)

### 4. Symbol Table
Functions and variables (first 50 shown):
- **FUNC**: Functions
- **OBJECT**: Variables
- **Address**: Virtual address in memory

## Examples

### Analyze system binary
```bash
./bin/elf-parser /bin/ls
```

### Check if binary is stripped
```bash
./bin/elf-parser /bin/ls | grep -i symbol
# If "No symbol table found" - binary is stripped
```

### Find entry point
```bash
./bin/elf-parser ./program | grep "Entry Point"
```

### Check security features
```bash
./bin/elf-parser ./program | grep GNU_STACK
# RW = no execute protection
# R = NX enabled (good)
```

### Identify architecture
```bash
./bin/elf-parser ./binary | grep Machine
```

## Use Cases

### 1. Malware Analysis
```bash
# Check what the binary loads
./bin/elf-parser suspicious_binary | grep INTERP

# Find executable sections
./bin/elf-parser suspicious_binary | grep "X$"

# Look for suspicious symbols
./bin/elf-parser suspicious_binary | grep -i "exec\|system\|socket"
```

### 2. Reverse Engineering
```bash
# Find entry point for debugging
./bin/elf-parser ./target | grep "Entry Point"

# List all functions
./bin/elf-parser ./target | grep FUNC

# Check if PIE (Position Independent Executable)
./bin/elf-parser ./target | grep Type
# "Shared Object" = PIE enabled
# "Executable" = PIE disabled
```

### 3. Binary Hardening Check
```bash
# Check stack protection
./bin/elf-parser ./binary | grep GNU_STACK

# Check if stripped
./bin/elf-parser ./binary | grep -c FUNC

# Check RELRO
./bin/elf-parser ./binary | grep GNU_RELRO
```

## Technical Details

### ELF Structure
```
ELF Header
├── Program Headers (how to load)
├── Section Headers (what's inside)
└── Symbol Tables (names of functions/variables)
```

### Common Section Types
- **PROGBITS**: Program data (code, initialized data)
- **NOBITS**: Uninitialized data (.bss)
- **SYMTAB**: Symbol table (functions, variables)
- **STRTAB**: String table (symbol names)
- **DYNAMIC**: Dynamic linking info
- **RELA**: Relocation entries

### Section Flags
- **W**: Writable at runtime
- **A**: Allocated in memory
- **X**: Executable

## Comparison with Other Tools

| Feature | elf-parser | readelf | objdump |
|---------|-----------|---------|---------|
| Headers | ✓ | ✓ | ✓ |
| Sections | ✓ | ✓ | ✓ |
| Symbols | ✓ (50) | ✓ (all) | ✓ |
| Disassembly | ✗ | ✗ | ✓ |
| Simple output | ✓ | ✗ | ✗ |
| Fast | ✓ | ✓ | ✗ |

## Limitations
- Shows only first 50 symbols (to keep output readable)
- 64-bit ELF only (can be extended for 32-bit)
- No disassembly (use dbi-framework for that)
- No relocation details

## Integration with Other Tools

### With strings
```bash
./bin/elf-parser binary > structure.txt
./bin/strings binary > strings.txt
```

### With syscall-tracer
```bash
# Find entry point
ENTRY=$(./bin/elf-parser ./target | grep "Entry Point" | awk '{print $3}')
# Trace from entry
./bin/syscall-tracer ./target
```

### With dbi-advanced
```bash
# Find functions to hook
./bin/elf-parser ./target | grep FUNC | grep -i "open\|read\|write"
# Hook them
./bin/dbi-advanced ./target open read write
```

## Source Code
Location: `parsers/elf-parser.c` (~200 lines)

Key functions:
- `print_elf_header()` - Parse main header
- `print_program_headers()` - Parse segments
- `print_section_headers()` - Parse sections
- `print_symbols()` - Parse symbol table

## Further Reading
- ELF specification: https://refspecs.linuxfoundation.org/elf/elf.pdf
- `man elf`
- `/usr/include/elf.h`
