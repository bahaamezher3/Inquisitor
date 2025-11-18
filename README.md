# Inquisitor

Inquisitor is a C++ ARP spoofing / traffic interception tool, packaged with a Docker test lab (attacker, gateway/FTP server, and victim).  
It is meant **only** for learning and authorized security testing in a controlled environment.

---

## 1. Prerequisites

- Linux host (recommended)
- Docker
- Docker Compose
- Internet access (for Alpine package repositories)

---

## 2. Project Structure

```text
Inquisitor/
├── Dockerfile            # Build environment for the attacker container (builds inquisitor)
├── docker-compose.yml    # Defines attacker, gateway, and victim containers
├── run_test.sh           # Helper script to build and run the lab (optional)
├── get_macs.sh           # Script to help list interfaces and MAC addresses
├── src/                  # C++ source code
│   ├── main.cpp
│   ├── InquisitorFacade.cpp/.hpp
│   ├── arp/
│   │   ├── ArpPoisoner.cpp/.hpp
│   ├── network/
│   │   ├── PacketCapture.cpp/.hpp
│   └── protocol/
│       ├── FtpParser.cpp/.hpp
└── ftp_uploads/          # Folder where uploaded test files can be stored

## 3. Quick Start – Build and Run the Lab

From the root of the project (Inquisitor/):

# Build images and start all containers in the background
```bash
docker-compose up -d --build
```

This will start 3 containers:

inquisitor_attacker – builds and runs the Inquisitor binary
inquisitor_gateway – acts as gateway + FTP server
inquisitor_victim – victim host
Verify they are running:
```bash
docker ps
```

You should see the three containers running.

## 4. How to Use the Lab

### 4.1. Get a Shell in Each Container

Attacker:
```bash
docker exec -it inquisitor_attacker sh
```

Gateway:
```bash
docker exec -it inquisitor_gateway sh
```

Victim:
```bash
docker exec -it inquisitor_victim sh
```
