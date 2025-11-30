## Essential Files to Study (in order)

### Start Here - Easy (210 lines total)

1. dbi/syscall-tracer.c [80 lines] ★☆☆☆☆
   - Learn: ptrace basics, syscall interception
   - Simplest tool

2. tools/memdump.c [70 lines] ★☆☆☆☆
   - Learn: Memory reading, hex dump
   - Easy to understand

3. analysis/strings.c [60 lines] ★☆☆☆☆
   - Learn: File parsing
   - No ptrace, just file I/O

### Core Concepts - Medium (350 lines)

4. dbi/dbi-framework.c [200 lines] ★★★☆☆
   - Learn: Breakpoints (0xCC), register inspection
   - **Most important for understanding DBI**

5. parsers/pe-parser.c [150 lines] ★★☆☆☆
   - Learn: Binary formats, PE structure
   - Windows executable parsing

### Advanced - Hard (500 lines)

6. dbi/dbi-advanced.c [150 lines] ★★★★☆
   - Learn: Symbol resolution, function hooking
   - **Most complex, read last**

7. gui/rekit-gui.c [350 lines] ★★★☆☆
   - Learn: GTK, GUI integration
   - Ties everything together

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━


## Quick Summary

Total: ~1000 lines of C code  
Time: ~5 hours to read all  
Start: dbi/syscall-tracer.c (easiest)  
Core: dbi/dbi-framework.c (most important)  
Advanced: dbi/dbi-advanced.c (hardest)

Full study guide: STUDY_GUIDE.md  
Quick reference: ESSENTIAL_FILES.txt
