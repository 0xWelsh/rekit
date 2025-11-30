# REKit Study Guide - Essential Files to Understand

## Core Concepts (Read in Order)

### 1. Dynamic Binary Instrumentation Basics
**File:** `dbi/dbi-framework.c` (200 lines)
**Learn:**
- How ptrace works
- Setting breakpoints (INT3 instruction)
- Reading/writing process memory
- Register inspection
- Single-stepping

**Key Functions:**
```c
ptrace(PTRACE_ATTACH)      // Attach to process
ptrace(PTRACE_PEEKTEXT)    // Read memory
ptrace(PTRACE_POKETEXT)    // Write memory (set breakpoint)
ptrace(PTRACE_GETREGS)     // Read registers
ptrace(PTRACE_SINGLESTEP)  // Execute one instruction
```

---

### 2. Function Hooking
**File:** `dbi/dbi-advanced.c` (150 lines)
**Learn:**
- Symbol resolution from ELF
- Automatic function finding
- Hook management
- Argument inspection

**Key Concepts:**
- Parse ELF symbol table
- Find function address by name
- Replace first byte with 0xCC (INT3)
- Restore original byte after hit

---

### 3. System Call Tracing
**File:** `dbi/syscall-tracer.c` (80 lines)
**Learn:**
- PTRACE_SYSCALL mode
- Syscall entry/exit detection
- Argument extraction
- Return value capture

**Key Pattern:**
```c
ptrace(PTRACE_SYSCALL)  // Stop at syscall entry/exit
// Entry: orig_rax = syscall number, rdi/rsi/rdx = args
// Exit: rax = return value
```

---

### 4. PE File Parsing
**File:** `parsers/pe-parser.c` (150 lines)
**Learn:**
- DOS header structure
- PE header layout
- Section headers
- Entry point location

**Key Structures:**
```c
DOS_HEADER    // MZ signature, PE offset
FILE_HEADER   // Machine type, section count
OPTIONAL_HEADER // Entry point, image base
SECTION_HEADER  // .text, .data, .rdata
```

---

### 5. Memory Operations
**File:** `tools/memdump.c` (70 lines)
**Learn:**
- Process attachment
- Memory reading
- Hex dump formatting

---

### 6. GUI Integration
**File:** `gui/rekit-gui.c` (350 lines)
**Learn:**
- GTK widget creation
- Event handling
- Running CLI tools from GUI
- Output capture

---

## Study Path

### Beginner (Start Here)
1. `dbi/syscall-tracer.c` - Simplest, understand ptrace basics
2. `tools/memdump.c` - Memory operations
3. `analysis/strings.c` - File parsing basics

### Intermediate
4. `dbi/dbi-framework.c` - Breakpoints and debugging
5. `parsers/pe-parser.c` - Binary format parsing

### Advanced
6. `dbi/dbi-advanced.c` - Symbol resolution and hooking
7. `gui/rekit-gui.c` - GUI integration

---

## Key Concepts by File

### dbi/syscall-tracer.c
```
Lines: 80
Complexity: ★☆☆☆☆
Concepts:
  - ptrace basics
  - Process control
  - Syscall interception
```

### dbi/dbi-framework.c
```
Lines: 200
Complexity: ★★★☆☆
Concepts:
  - Breakpoint management
  - Register inspection
  - Memory manipulation
  - Disassembly (Capstone)
```

### dbi/dbi-advanced.c
```
Lines: 150
Complexity: ★★★★☆
Concepts:
  - ELF parsing
  - Symbol table reading
  - Dynamic function resolution
  - Hook lifecycle
```

### parsers/pe-parser.c
```
Lines: 150
Complexity: ★★☆☆☆
Concepts:
  - Binary file formats
  - Structure parsing
  - Header navigation
```

### gui/rekit-gui.c
```
Lines: 350
Complexity: ★★★☆☆
Concepts:
  - GTK programming
  - Event-driven design
  - Process execution
  - Output capture
```

---

## Understanding Flow

### How Syscall Tracing Works
```
1. Fork process
2. Child: ptrace(TRACEME) + exec
3. Parent: Wait for child to stop
4. Loop:
   - ptrace(SYSCALL) - Resume until syscall
   - Read registers (syscall number, args)
   - ptrace(SYSCALL) - Resume until syscall exit
   - Read return value
```

### How Function Hooking Works
```
1. Parse ELF to find function address
2. Read original byte at address
3. Write 0xCC (INT3 breakpoint)
4. When hit:
   - Restore original byte
   - Decrement RIP
   - Single-step
   - Re-insert breakpoint
```

### How Breakpoints Work
```
Original:  55 48 89 e5 ...  (push rbp; mov rbp, rsp)
Modified:  CC 48 89 e5 ...  (int3; mov rbp, rsp)
           ↑
           Causes SIGTRAP when executed
```

---

## Important Code Snippets

### Setting a Breakpoint
```c
// Read original instruction
long orig = ptrace(PTRACE_PEEKTEXT, pid, addr, NULL);

// Replace first byte with INT3 (0xCC)
long trap = (orig & ~0xFF) | 0xCC;
ptrace(PTRACE_POKETEXT, pid, addr, trap);
```

### Reading Registers
```c
struct user_regs_struct regs;
ptrace(PTRACE_GETREGS, pid, NULL, &regs);

printf("RIP: 0x%llx\n", regs.rip);
printf("RAX: 0x%llx\n", regs.rax);
```

### Finding Function in ELF
```c
// Read ELF header
Elf64_Ehdr ehdr;
read(fd, &ehdr, sizeof(ehdr));

// Read section headers
lseek(fd, ehdr.e_shoff, SEEK_SET);
Elf64_Shdr shdr[ehdr.e_shnum];
read(fd, shdr, sizeof(Elf64_Shdr) * ehdr.e_shnum);

// Find symbol table
for (int i = 0; i < ehdr.e_shnum; i++) {
    if (shdr[i].sh_type == SHT_SYMTAB) {
        // Read symbols
        Elf64_Sym *syms = ...;
        // Match by name
    }
}
```

---

## Testing Your Understanding

After reading each file, try:

1. **syscall-tracer.c**
   - Modify to filter specific syscalls
   - Add timestamp to each call

2. **dbi-framework.c**
   - Add conditional breakpoints
   - Dump stack on break

3. **dbi-advanced.c**
   - Hook multiple functions
   - Modify function arguments

4. **pe-parser.c**
   - Add import table parsing
   - Extract resources

5. **gui/rekit-gui.c**
   - Add new button
   - Create new tab

---

## External Resources

### ptrace
- `man ptrace` - System call documentation
- Linux kernel source: `kernel/ptrace.c`

### ELF Format
- `man elf` - ELF file format
- `/usr/include/elf.h` - Structure definitions

### PE Format
- Microsoft PE/COFF specification
- `wine` source code for examples

### GTK
- GTK+ 3 documentation
- Tutorial: https://www.gtk.org/docs/

---

## Quick Reference

### File Sizes
```
syscall-tracer.c    80 lines   ★ Start here
memdump.c          70 lines   ★ Easy
strings.c          60 lines   ★ Easy
pe-parser.c       150 lines   ★★ Medium
dbi-framework.c   200 lines   ★★★ Medium
dbi-advanced.c    150 lines   ★★★★ Hard
rekit-gui.c       350 lines   ★★★ Medium
```

### Dependencies
```
syscall-tracer    → ptrace only
dbi-framework     → ptrace + capstone
dbi-advanced      → ptrace + capstone + ELF
pe-parser         → file I/O only
rekit-gui         → GTK + all tools
```

---

## Recommended Order

**Day 1:** Read `syscall-tracer.c` + test it
**Day 2:** Read `dbi-framework.c` + understand breakpoints
**Day 3:** Read `dbi-advanced.c` + understand hooking
**Day 4:** Read `pe-parser.c` + understand file formats
**Day 5:** Read `rekit-gui.c` + understand GUI

**Total:** ~1000 lines of well-commented C code
