# Network Routing Simulator

A C++ network routing simulator built as a baseline for comparing public routing
against privacy-preserving routing schemes in future research phases.

## Overview

This simulator models packet forwarding between routers in a single-process
environment.  All routers exist as in-memory objects — there is no actual
network communication.  The focus is on measuring **raw forwarding overhead**
(routing table lookups and packet traversal) using high-precision timing.

### Key Features

- **4 topology types**: Random connected graph, Ring, Full Mesh, Binary Tree
- **Modular routing table generation**: BFS-based (default), swappable via strategy pattern
- **TTL-based loop prevention**: Packets are dropped after 256 hops by default
- **Batch simulation**: Send hundreds of packets with aggregate statistics
- **Configurable logging**: SILENT / INFO / DEBUG verbosity levels
- **Scalable**: Designed for 10–500+ routers

## Architecture

```
┌─────────────┐    ┌──────────────────────────┐
│  Simulator   │───▶│       Network             │
│  (timing,    │    │  ┌─────────┐ ┌─────────┐ │
│   logging,   │    │  │ Router 0│ │ Router 1│ │
│   stats)     │    │  └─────────┘ └─────────┘ │
└─────────────┘    │  ┌─────────┐ ┌─────────┐ │
                    │  │ Router 2│ │ Router N│ │
       ▲            │  └─────────┘ └─────────┘ │
       │            └──────────┬───────────────┘
  ┌────┴────┐                  │
  │ Packet  │    ┌─────────────┴──────────────┐
  └─────────┘    │ RoutingTableGenerator (BFS)│
                 └────────────────────────────┘
```

| Class | Responsibility |
|-------|---------------|
| `Packet` | Data carrier: source, destination, payload, TTL |
| `Router` | Stores ID + vector-based routing table (O(1) lookup) |
| `RoutingTableGenerator` | Abstract strategy for populating routing tables |
| `BFSRoutingTableGenerator` | Concrete BFS strategy — guarantees valid paths |
| `Network` | Owns routers + adjacency list, generates topology, forwards packets |
| `Simulator` | Orchestrates timing, logging, single/batch simulations |

## Project Structure

```
routing-simulator/
├── CMakeLists.txt
├── Dockerfile
├── README.md
├── include/
│   ├── Packet.h
│   ├── Router.h
│   ├── RoutingTableGenerator.h
│   ├── Network.h
│   └── Simulator.h
└── src/
    ├── main.cpp
    ├── Packet.cpp
    ├── Router.cpp
    ├── RoutingTableGenerator.cpp
    ├── Network.cpp
    └── Simulator.cpp
```

## Build & Run

### Local Build (CMake)

```bash
# Configure and build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run
./build/routing_simulator        # Linux/macOS
.\build\Release\routing_simulator.exe   # Windows
```

### Docker

```bash
# Build image
docker build -t routing-simulator .

# Run
docker run routing-simulator
```

## Example Output

```
========================================
  Network Routing Simulator v1.0
  Baseline Public Routing Network
========================================

--- Demo 1: Random Topology (20 routers) ---

[SETUP] Generating routing tables (BFS) for 20 routers...
[SETUP] Topology ready: Random Connected Graph with 20 routers

[INFO]  Sending packet: Router 0 -> Router 19
[INFO]  Router 0 forwarded packet to Router 12
[INFO]  Router 12 forwarded packet to Router 5
[INFO]  Router 5 forwarded packet to Router 19
[INFO]  Packet delivered successfully

┌─────────────────────────────────────┐
│        Simulation Result            │
├─────────────────────────────────────┤
│  Source:       Router 0
│  Destination:  Router 19
│  Path:         0 -> 12 -> 5 -> 19
│  Hop Count:    3
│  Latency:      1.234 µs
│  Status:       Delivered ✓
└─────────────────────────────────────┘
```

## Future Research Extensions

This implementation is designed as a clean baseline.  Future phases may add:

- Secret sharing and privacy-preserving routing
- Simulated link latency / propagation delay
- Distributed routing protocols
- Oblivious routing concepts
- Cryptographic computation overhead benchmarking
- Comparative analysis dashboards

