#!/bin/bash

echo "======================================"
echo "  REKit Installation"
echo "======================================"
echo

# Check dependencies
echo "[*] Checking dependencies..."
if ! pkg-config --exists capstone; then
    echo "[-] libcapstone not found. Installing..."
    sudo apt-get update
    sudo apt-get install -y libcapstone-dev
fi

if ! command -v gcc &> /dev/null; then
    echo "[-] gcc not found. Installing..."
    sudo apt-get install -y build-essential
fi

echo "[+] Dependencies OK"
echo

# Build
echo "[*] Building tools..."
make clean
make

if [ $? -eq 0 ]; then
    echo "[+] Build successful"
else
    echo "[-] Build failed"
    exit 1
fi

echo

# Test
echo "[*] Testing tools..."
./bin/syscall-tracer /bin/echo "REKit Test" > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "[+] Tools working"
else
    echo "[-] Test failed"
    exit 1
fi

echo
echo "======================================"
echo "  Installation Complete!"
echo "======================================"
echo
echo "Tools installed in: $(pwd)/bin/"
echo
echo "Quick start:"
echo "  ./bin/syscall-tracer /bin/ls"
echo "  ./bin/dbi-advanced /bin/cat open read"
echo "  ./bin/strings /bin/ls"
echo
echo "Documentation:"
echo "  README.md - Full documentation"
echo "  QUICKSTART.md - Quick examples"
echo "  PROJECT.md - Project overview"
echo
echo "Optional: Install to system (requires sudo)"
echo "  sudo make install"
