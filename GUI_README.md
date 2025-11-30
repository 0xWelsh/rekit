# REKit GUI

Graphical interface for the Reverse Engineering Toolkit.

## Features

### Dynamic Analysis Tab
- **Trace Syscalls** - Monitor system calls in real-time
- **Hook Functions** - Intercept function calls by name
- File browser for easy target selection
- Function list input (space-separated)

### Static Analysis Tab
- **Extract Strings** - Find printable strings in binaries
- **Parse PE** - Analyze PE/EXE file structure

### Memory Tab
- **Dump Memory** - Extract memory from running processes
- Hex dump viewer
- Address input with validation

## Usage

### Launch GUI
```bash
cd /home/welsh/reverse_engineering/rekit
./rekit-gui.sh
```

Or double-click `rekit.desktop`

### Quick Start

1. **Dynamic Analysis:**
   - Click "Browse" to select a binary
   - Enter functions to hook: `open read write close`
   - Click "Hook Functions"
   - Output appears in bottom panel

2. **Static Analysis:**
   - Select a binary
   - Click "Extract Strings" or "Parse PE"
   - View results in output panel

3. **Memory Dump:**
   - Enter PID in "Target" field
   - Enter address: `0x400000`
   - Click "Dump Memory"

## Screenshots

### Main Window
```
┌─────────────────────────────────────────────┐
│ REKit - Reverse Engineering Toolkit         │
├─────────────────────────────────────────────┤
│ [Dynamic Analysis] [Static] [Memory]        │
│                                             │
│ Target: /bin/cat              [Browse]      │
│ Functions: open read write close            │
│                                             │
│ [Trace Syscalls]  [Hook Functions]          │
│                                             │
├─────────────────────────────────────────────┤
│ === Output ===                              │
│ [HOOK] open()                               │
│   RDI: 0x7fff... RSI: 0x0 RDX: 0x0         │
│ [HOOK] read()                               │
│   RDI: 0x3 RSI: 0x... RDX: 0x2000          │
│                                             │
└─────────────────────────────────────────────┘
```

## Tabs

### 1. Dynamic Analysis
- Real-time tracing
- Function hooking
- Syscall monitoring
- Automatic timeout (3 seconds)

### 2. Static Analysis
- String extraction (min length: 4)
- PE header parsing
- Section analysis
- No execution required

### 3. Memory
- Live memory dumping
- Hex viewer
- Process attachment
- Configurable size

## Output Panel

- Monospace font for alignment
- Scrollable
- Auto-clears on new operation
- Shows errors and warnings

## Tips

1. **Use Browse button** - Easier than typing paths
2. **Multiple functions** - Space-separated: `malloc free strcpy`
3. **Timeout protection** - Operations auto-stop after 3 seconds
4. **Check permissions** - Some operations need sudo

## Keyboard Shortcuts

- `Ctrl+Q` - Quit (when implemented)
- `Tab` - Switch between tabs
- `Enter` - Execute (in entry fields)

## Requirements

- GTK+ 3.0
- All REKit CLI tools built
- X11 display

## Building

```bash
make gui
```

## Troubleshooting

**GUI doesn't start:**
```bash
# Check if GTK is installed
pkg-config --exists gtk+-3.0

# Install if missing
sudo apt-get install libgtk-3-dev
```

**No output:**
- Check if CLI tools are in `bin/`
- Verify file permissions
- Run from correct directory

**Permission denied:**
- Some operations need sudo
- Use CLI tools directly for privileged operations

## Extending

### Add New Tab
Edit `gui/rekit-gui.c`:
```c
GtkWidget* create_my_tab(AppWidgets *app) {
    // Create widgets
    return vbox;
}

// In main():
gtk_notebook_append_page(GTK_NOTEBOOK(app.notebook), 
    create_my_tab(&app), gtk_label_new("My Tab"));
```

### Add New Button
```c
GtkWidget *btn = gtk_button_new_with_label("My Action");
g_signal_connect(btn, "clicked", G_CALLBACK(on_my_action), app);
gtk_box_pack_start(GTK_BOX(box), btn, TRUE, TRUE, 0);
```

## Future Features

- [ ] Syntax highlighting in output
- [ ] Save output to file
- [ ] Process list picker
- [ ] Real-time updates
- [ ] Breakpoint manager
- [ ] Hex editor
- [ ] Disassembly view
- [ ] Graph visualization

## CLI vs GUI

| Feature | CLI | GUI |
|---------|-----|-----|
| Speed | Fast | Medium |
| Automation | Easy | Hard |
| User-friendly | No | Yes |
| Scripting | Yes | No |
| Visual | No | Yes |

Use CLI for automation, GUI for exploration.
