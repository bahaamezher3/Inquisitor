#!/bin/bash

# Inquisitor Setup Script
# Prepares the complete project structure

set -e

echo "======================================"
echo "Inquisitor Project Setup"
echo "======================================"
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# Create directory structure
info "Creating directory structure..."
mkdir -p src/arp
mkdir -p src/network
mkdir -p src/protocol
mkdir -p tests

success "Directory structure created"

# Create header files if they don't exist
info "Setting up source files..."

# Note: The main code is already provided in the artifact above
# This script creates the directory structure and placeholder files

# Create .gitignore
cat > .gitignore << 'EOF'
# Compiled files
*.o
*.out
inquisitor

# Build directories
obj/
build/

# IDE
.vscode/
.idea/
*.swp
*~

# OS
.DS_Store
Thumbs.db

# Test files
test_*.txt
/tmp/
EOF

success "Created .gitignore"

# Create a helper script for getting MAC addresses
cat > get_macs.sh << 'EOF'
#!/bin/bash
echo "Network Interfaces and MAC Addresses:"
echo "======================================"
ip link show | grep -E "^[0-9]|link/ether" | sed 'N;s/\n/ /'
echo ""
echo "ARP Table:"
echo "======================================"
ip neigh show
EOF

chmod +x get_macs.sh
success "Created get_macs.sh helper script"

# Create a quick start guide
cat > QUICKSTART.md << 'EOF'
# Quick Start Guide

## 1. Build the Project

```bash
make
```

## 2. Get Network Information

```bash
# Get your network interfaces
ip link show

# Get MAC addresses in your network
./get_macs.sh

# Or use arp
arp -a
```

## 3. Run Inquisitor

```bash
# Replace with actual IPs and MACs from your network
sudo ./inquisitor \
    192.168.1.1 00:11:22:33:44:55 \
    192.168.1.100 aa:bb:cc:dd:ee:ff
```

## 4. Docker Test Environment

```bash
# Start the test environment
make docker-compose-up

# Enter attacker container
docker exec -it inquisitor_attacker /bin/bash

# Inside container:
# 1. Check network interfaces
ip addr

# 2. Check ARP table for MAC addresses
ip neigh show

# 3. Run inquisitor
./inquisitor <gateway-ip> <gateway-mac> <victim-ip> <victim-mac>
```

## 5. Test FTP from Victim

In another terminal:

```bash
# Enter victim container
docker exec -it inquisitor_victim sh

# Connect to FTP server
ftp 192.168.100.1

# Login as anonymous
> user anonymous
> pass

# Test commands
> ls
> put /etc/hosts
> get hosts
> quit
```

## 6. Cleanup

```bash
# Stop with CTRL+C (ARP tables will be restored automatically)

# Stop Docker environment
make docker-compose-down

# Clean build files
make clean
```

## Tips

- Always run with root/sudo privileges
- Use `-v` flag for verbose FTP traffic
- CTRL+C gracefully stops and restores ARP tables
- Check `ip neigh` to verify ARP table changes
- Use `tcpdump` to verify ARP packets: `tcpdump -i eth0 arp`
EOF

success "Created QUICKSTART.md"

# Check dependencies
echo ""
info "Checking dependencies..."

check_dependency() {
    if command -v $1 &> /dev/null; then
        success "$1 is installed"
        return 0
    else
        echo "    ✗ $1 is NOT installed"
        return 1
    fi
}

MISSING_DEPS=0

check_dependency "g++" || MISSING_DEPS=1
check_dependency "make" || MISSING_DEPS=1

if ldconfig -p 2>/dev/null | grep -q libpcap; then
    success "libpcap is installed"
else
    echo "    ✗ libpcap is NOT installed"
    MISSING_DEPS=1
fi

check_dependency "docker" || info "Docker not found (optional for testing)"

echo ""
if [ $MISSING_DEPS -eq 1 ]; then
    echo "Some dependencies are missing. Install with:"
    echo ""
    echo "  Ubuntu/Debian:"
    echo "    sudo apt-get install build-essential libpcap-dev"
    echo ""
    echo "  Fedora/RHEL:"
    echo "    sudo dnf install gcc-c++ make libpcap-devel"
    echo ""
    echo "  Arch:"
    echo "    sudo pacman -S base-devel libpcap"
    echo ""
fi

# Summary
echo ""
echo "======================================"
echo "Setup Complete!"
echo "======================================"
echo ""
echo "Next steps:"
echo "  1. Copy all source code from the artifacts"
echo "  2. Run: make"
echo "  3. Run: sudo ./inquisitor <args>"
echo ""
echo "For Docker testing:"
echo "  1. Run: make docker-compose-up"
echo "  2. Run: docker exec -it inquisitor_attacker /bin/bash"
echo ""
echo "Read QUICKSTART.md for detailed instructions"
echo ""