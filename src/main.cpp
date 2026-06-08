/**
 * @file main.cpp
 * @brief Network Routing Simulator — demonstration entry point.
 *
 * Runs two demos:
 *   1. 20-router random connected graph  — single packet + batch of 100
 *   2. 50-router random connected graph  — single packet + batch of 500
 *
 * All routers exist as in-process objects.  Routing tables are generated
 * via BFS (pluggable strategy).  Timing measures raw C++ forwarding
 * overhead using std::chrono::steady_clock.
 */

#include <iostream>
#include "Simulator.h"

int main() {
    std::cout << "========================================\n";
    std::cout << "  Network Routing Simulator v1.0\n";
    std::cout << "  Baseline Public Routing Network\n";
    std::cout << "========================================\n\n";

    // ─────────────────────────────────────────────────────────
    // Demo 1: Small random topology (20 routers)
    // ─────────────────────────────────────────────────────────
    std::cout << "--- Demo 1: Random Topology (20 routers) ---\n\n";

    Simulator sim1(LogLevel::INFO);
    sim1.setupNetwork(20, TopologyType::RANDOM);

    // Single packet: Router 0 -> Router 19
    auto result1 = sim1.sendPacket(0, 19, "Hello Router 19!");
    sim1.printResult(result1);

    // Batch test: 100 random source/destination pairs
    std::cout << "\n--- Batch Simulation (100 packets, 20 routers) ---\n";
    auto batch1 = sim1.runBatch(100);
    sim1.printBatchResult(batch1);

    // ─────────────────────────────────────────────────────────
    // Demo 2: Medium random topology (50 routers)
    // ─────────────────────────────────────────────────────────
    std::cout << "\n--- Demo 2: Random Topology (50 routers) ---\n\n";

    Simulator sim2(LogLevel::SILENT);   // suppress per-hop logs at scale
    sim2.setupNetwork(50, TopologyType::RANDOM);

    // Single packet: Router 0 -> Router 49
    auto result2 = sim2.sendPacket(0, 49);
    sim2.printResult(result2);

    // Batch test: 500 random source/destination pairs
    std::cout << "\n--- Batch Simulation (500 packets, 50 routers) ---\n";
    auto batch2 = sim2.runBatch(500);
    sim2.printBatchResult(batch2);

    // ─────────────────────────────────────────────────────────
    // Demo 3: Ring topology (10 routers) — deterministic path
    // ─────────────────────────────────────────────────────────
    std::cout << "\n--- Demo 3: Ring Topology (10 routers) ---\n\n";

    Simulator sim3(LogLevel::INFO);
    sim3.setupNetwork(10, TopologyType::RING);

    auto result3 = sim3.sendPacket(0, 5, "Ring test");
    sim3.printResult(result3);

    std::cout << "\n========================================\n";
    std::cout << "  All demos completed successfully.\n";
    std::cout << "========================================\n";

    return 0;
}
