#!/bin/bash
echo "Network Interfaces and MAC Addresses:"
echo "======================================"
ip link show | grep -E "^[0-9]|link/ether" | sed 'N;s/\n/ /'
echo ""
echo "ARP Table:"
echo "======================================"
ip neigh show
