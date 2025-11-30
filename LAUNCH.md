# REKit Launch Guide

## GUI Mode (Recommended for Beginners)

### Start GUI
```bash
cd /home/welsh/reverse_engineering/rekit
./rekit-gui.sh
```

### Using the GUI

**Tab 1: Dynamic Analysis**
1. Click "Browse" to select a binary (e.g., `/bin/cat`)
2. Enter functions to hook: `open read write close`
3. Click "Hook Functions"
4. View output in bottom panel

**Tab 2: Static Analysis**
1. Select a binary
2. Click "Extract Strings" to find text
3. Click "Parse PE" for Windows executables

**Tab 3: Memory**
1. Enter PID in "Target" field
2. Enter address: `0x400000`
3. Click "Dump Memory"

## CLI Mode (For Automation)

### Trace System Calls
```bash
./bin/syscall-tracer /bin/ls
```

### Hook Functions
```bash
./bin/dbi-advanced /bin/cat open read write close
```

### Extract Strings
```bash
./bin/strings /bin/ls 4
```

### Parse PE File
```bash
./bin/pe-parser malware.exe
```

### Dump Memory
```bash
./bin/memdump <pid> 0x400000 0x1000
```

## Quick Examples

### Example 1: Analyze Unknown Binary
```bash
# GUI: Load file → Extract Strings → Parse PE
# CLI:
./bin/strings unknown.exe > strings.txt
./bin/pe-parser unknown.exe > structure.txt
```

### Example 2: Monitor File Operations
```bash
# GUI: Target=/bin/cat, Functions=open read close → Hook
# CLI:
./bin/dbi-advanced /bin/cat open read close
```

### Example 3: Trace Program Behavior
```bash
# GUI: Target=/usr/bin/wget → Trace Syscalls
# CLI:
./bin/syscall-tracer /usr/bin/wget http://example.com
```

## Choosing GUI vs CLI

**Use GUI when:**
- Learning reverse engineering
- Exploring unknown binaries
- Need visual feedback
- One-off analysis

**Use CLI when:**
- Automating analysis
- Scripting workflows
- Processing multiple files
- Remote/headless systems

## Desktop Integration

### Create Desktop Shortcut
```bash
cp rekit.desktop ~/.local/share/applications/
```

Now REKit appears in your application menu!

### Create Launcher
```bash
# Add to ~/.bashrc
alias rekit='/home/welsh/reverse_engineering/rekit/rekit-gui.sh'

# Now just type:
rekit
```

## Troubleshooting

### GUI won't start
```bash
# Check GTK
pkg-config --exists gtk+-3.0

# Install if missing
sudo apt-get install libgtk-3-dev
make gui
```

### No output in GUI
- Verify tools are built: `ls bin/`
- Check file permissions
- Run from correct directory

### Permission errors
```bash
# Some operations need sudo
sudo ./bin/memdump <pid> 0x400000 0x1000
```

## Tips

1. **Start with GUI** - Easier to learn
2. **Move to CLI** - For automation
3. **Use Browse button** - Avoid typos
4. **Check output panel** - Shows errors
5. **Timeout is 3 seconds** - Prevents hangs

## Next Steps

1. Try the GUI with `/bin/ls`
2. Hook some functions
3. Extract strings from a binary
4. Read `GUI_README.md` for details
5. Explore CLI tools for scripting

## Support

- Documentation: `README.md`, `QUICKSTART.md`
- GUI Help: `GUI_README.md`
- Examples: `examples/`
- Layout: `GUI_LAYOUT.txt`
