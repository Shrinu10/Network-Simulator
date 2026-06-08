# Network Routing Simulator

A modular C++17 network routing simulator built for research. Simulates packet
forwarding across configurable network topologies with high-precision timing
and optional simulated link latency.

**Purpose:** Establish a clean baseline for a public routing network. This will
later be compared against a privacy-preserving routing network using secret
sharing and cryptographic computations.

---

## Quick Start

### 1. Edit `config.txt`

Open `config.txt` and set your preferences:

```
routers           = 1000        # Number of routers (any positive integer)
topology          = RANDOM      # RANDOM, RING, MESH, TREE
packets           = 500         # Batch simulation size
simulate_latency  = ON          # ON or OFF
latency_min_ms    = 1.0         # Min link delay (ms)
latency_max_ms    = 10.0        # Max link delay (ms)
log_level         = SILENT      # SILENT, INFO, DEBUG
seed              = 42          # 0 = random, any other = reproducible
source            = 0           # Source router for single-packet test
destination       = -1          # -1 = last router
```

### 2. Build & Run

**Windows (MSVC):**
```bash
cmake -S . -B build
cmake --build build --config Release
.\build\Release\routing_simulator.exe
```

**Linux / macOS (GCC/Clang):**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/routing_simulator
```

**Docker:**
```bash
docker build -t routing-simulator .
docker run --rm routing-simulator
```

---

## Configuration Reference

All simulation parameters are in `config.txt`. No C++ code changes needed.

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `routers` | int | 20 | Number of routers in the network |
| `topology` | string | RANDOM | Topology type: `RANDOM`, `RING`, `MESH`, `TREE` |
| `packets` | int | 100 | Number of packets in batch simulation |
| `source` | int | 0 | Source router for single-packet test |
| `destination` | int | -1 | Destination router (-1 = last router) |
| `simulate_latency` | ON/OFF | OFF | Enable simulated link latency |
| `latency_min_ms` | float | 1.0 | Minimum link latency (ms) |
| `latency_max_ms` | float | 10.0 | Maximum link latency (ms) |
| `log_level` | string | INFO | `SILENT`, `INFO`, or `DEBUG` |
| `seed` | int | 0 | RNG seed (0 = non-deterministic) |

---

## Topology Types

| Type | Description | Edges | Use Case |
|------|-------------|-------|----------|
| **RANDOM** | Random spanning tree + extra edges | ~2N | Default, realistic topology |
| **RING** | Circular chain (i connects to i+1) | N | Deterministic path testing |
| **MESH** | Every router connects to every other | N*(N-1)/2 | Best-case baseline (1 hop) |
| **TREE** | Binary tree (node i -> 2i+1, 2i+2) | N-1 | Hierarchical topology testing |

---

## Project Structure

```
routing-simulator/
|
+-- config.txt                     # USER-EDITABLE configuration
+-- CMakeLists.txt                 # Build configuration (C++17)
+-- Dockerfile                     # Multi-stage Docker build
+-- README.md                      # This file
|
+-- include/                       # Header files
|   +-- Config.h                   # SimConfig struct + loadConfig()
|   +-- Packet.h                   # Packet with TTL
|   +-- Router.h                   # Router with vector routing table
|   +-- RoutingTableGenerator.h    # Strategy pattern interface + BFS
|   +-- Network.h                  # Topology + forwarding + latency
|   +-- Simulator.h                # Timing + stats + result structs
|
+-- src/                           # Source files
    +-- Config.cpp                 # config.txt parser
    +-- Packet.cpp                 # Packet constructor
    +-- Router.cpp                 # Router lookup/init methods
    +-- RoutingTableGenerator.cpp  # BFS routing table generator
    +-- Network.cpp                # 4 topology algorithms + forwarding
    +-- Simulator.cpp              # Timing, batch sim, pretty-print
    +-- main.cpp                   # Config-driven entry point
```

---

## Architecture

```
+------------------------------------------------------------------+
|                        Simulator Layer                           |
|  - Timing (std::chrono::steady_clock)                           |
|  - Logging (SILENT / INFO / DEBUG)                              |
|  - Statistics collection (single packet + batch)                |
+------------------------------------------------------------------+
                              |
+------------------------------------------------------------------+
|                        Network Layer                             |
|  - Topology generation (Ring, Mesh, Tree, Random)               |
|  - Router management (vector<Router>)                           |
|  - Adjacency list (vector<vector<int>>)                         |
|  - Simulated link latency (map<pair, double>)                   |
|  - Packet forwarding (hop-by-hop table lookup)                  |
+------------------------------------------------------------------+
                     |                    |
+------------------------------+  +-----------------------------+
|  RoutingTableGenerator       |  |  Router + Packet            |
|  (Strategy Pattern)          |  |  - Router: ID + table       |
|  - BFSRoutingTableGenerator  |  |  - Packet: src, dst, TTL   |
|  - [Future: custom schemes]  |  |                             |
+------------------------------+  +-----------------------------+
```

### Design Patterns

- **Strategy Pattern:** Routing table generation is pluggable. Swap `BFSRoutingTableGenerator`
  for a custom implementation (e.g., privacy-preserving) without changing forwarding logic.
- **Config-Driven:** All parameters in `config.txt` — zero code changes needed for experiments.

---

## Core Classes

### Packet
| Property | Type | Description |
|----------|------|-------------|
| `sourceId` | int | Source router ID (0-based) |
| `destinationId` | int | Destination router ID |
| `payload` | string | Arbitrary payload data |
| `ttl` | int | Time-to-live, default 256 |

### Router
| Property | Type | Description |
|----------|------|-------------|
| `routerId` | int (private) | Unique identifier |
| `routingTable` | vector\<int\> | `routingTable[dest] = nextHop`; -1 = no route |

**Key:** Uses `std::vector<int>` (O(1) lookup) instead of `unordered_map` for
cache-friendly, contiguous memory access.

### Network
Owns all routers and the adjacency list. Generates topologies, populates routing
tables via the pluggable strategy, and performs hop-by-hop forwarding.

### Simulator
Wraps Network with timing, logging, and statistics. Provides `sendPacket()` for
single-packet tests and `runBatch()` for aggregate statistics.

---

## Simulated Link Latency

When `simulate_latency = ON`, each bidirectional link is assigned a random
delay uniformly sampled from `[latency_min_ms, latency_max_ms]`. During
forwarding, the simulator accumulates the latency of each link traversed.

- **Raw computation time** is always measured (sub-microsecond forwarding overhead).
- **Simulated latency** is reported separately and only when enabled.
- Latencies are generated deterministically if `seed != 0`.

---

## Extending the Routing Strategy

The routing table generation follows the Strategy pattern:

```cpp
class MyCustomGenerator : public RoutingTableGenerator {
public:
    void generate(std::vector<Router>& routers,
                  const std::vector<std::vector<int>>& adj) override {
        // Your custom routing logic here
    }
    std::string name() const override { return "CustomScheme"; }
};

// In main.cpp or your driver:
Simulator sim;
sim.setRoutingTableGenerator(std::make_unique<MyCustomGenerator>());
sim.setupNetwork(100, TopologyType::RANDOM);
```

---

## Example Output

### 20 routers, latency OFF:
```
+----------------------------------------------+
|         Simulation Configuration             |
+----------------------------------------------+
|  Routers:           20
|  Topology:          RANDOM
|  Batch Packets:     100
|  Simulate Latency:  OFF
|  Log Level:         INFO
+----------------------------------------------+

+-------------------------------------+
|        Simulation Result            |
+-------------------------------------+
|  Source:       Router 0
|  Destination:  Router 19
|  Path:         0 -> 1 -> 19
|  Hop Count:    2
|  Comp. Time:   0.600 us
|  Status:       Delivered [OK]
+-------------------------------------+

+=====================================+
|      Batch Simulation Results       |
+=====================================+
|  Total Packets:   100
|  Delivered:       100
|  Success Rate:    100.0%
|  Avg Comp. Time:  0.223 us
|  Avg Hops:        2.3
+=====================================+
```

### 1000 routers, latency ON:
```
+----------------------------------------------+
|         Simulation Configuration             |
+----------------------------------------------+
|  Routers:           1000
|  Topology:          RANDOM
|  Simulate Latency:  ON
|  Latency Range:     1 ms - 10 ms
|  Log Level:         SILENT
+----------------------------------------------+

+-------------------------------------+
|        Simulation Result            |
+-------------------------------------+
|  Source:       Router 0
|  Destination:  Router 999
|  Path:         0 -> 258 -> 12 -> 159 -> 999
|  Hop Count:    4
|  Comp. Time:   2.400 us
|  Sim. Latency: 21.811 ms
|  Status:       Delivered [OK]
+-------------------------------------+

+=====================================+
|      Batch Simulation Results       |
+=====================================+
|  Total Packets:   500
|  Delivered:       500
|  Success Rate:    100.0%
|  Avg Comp. Time:  1.319 us
|  --- Simulated Latency ---
|  Avg Sim. Lat.:   27.937 ms
|  Min Sim. Lat.:   5.148 ms
|  Max Sim. Lat.:   56.397 ms
|  Avg Hops:        5.1
+=====================================+
```

---

## Performance

| Environment | Routers | Avg Forwarding Time | Avg Hops |
|-------------|---------|---------------------|----------|
| Windows/MSVC | 20 | 0.223 us | 2.3 |
| Windows/MSVC | 1000 | 1.319 us | 5.1 |
| Linux/GCC (Docker) | 20 | 0.095 us | 2.1 |
| Linux/GCC (Docker) | 50 | 0.101 us | 2.8 |

---

## Requirements

- **C++17** compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake 3.16+**
- **Docker** (optional, for containerized builds)

---

## Future Research Extensions

| Extension | Impact |
|-----------|--------|
| Secret sharing routing | New `RoutingTableGenerator` subclass |
| Privacy-preserving forwarding | Modify `Network::forwardPacket()` |
| Distributed routing protocols | Router-to-router message passing |
| Benchmark comparisons | Side-by-side batch results |
| Topology visualization | Export adjacency list to DOT/GraphML |

---

## License

Research project. All rights reserved.
