/**
 * @file main.cpp
 * @brief Network Routing Simulator -- config-driven entry point.
 *
 * All simulation parameters are read from config.txt.
 * Users edit config.txt instead of modifying this file.
 *
 * What happens:
 *   1. Load config.txt  (number of routers, topology, latency, etc.)
 *   2. Build the network with the chosen topology
 *   3. Send a single packet (source -> destination)
 *   4. Run a batch simulation (random src/dst pairs)
 *   5. Print results
 */

#include <iostream>
#include <string>

#include "Config.h"
#include "Simulator.h"

// ─── Convert config strings to enums ────────────────────────────

static TopologyType parseTopology(const std::string& s) {
    if (s == "RING")   return TopologyType::RING;
    if (s == "MESH")   return TopologyType::MESH;
    if (s == "TREE")   return TopologyType::TREE;
    return TopologyType::RANDOM;  // default
}

static LogLevel parseLogLevel(const std::string& s) {
    if (s == "SILENT") return LogLevel::SILENT;
    if (s == "DEBUG")  return LogLevel::DEBUG;
    return LogLevel::INFO;  // default
}

// ─── Main ───────────────────────────────────────────────────────

int main() {
    std::cout << "========================================\n";
    std::cout << "  Network Routing Simulator v2.0\n";
    std::cout << "  Config-Driven Simulation\n";
    std::cout << "========================================\n";

    // ── 1. Load configuration ───────────────────────────────
    SimConfig cfg = loadConfig("config.txt");
    printConfig(cfg);

    TopologyType topo     = parseTopology(cfg.topologyStr);
    LogLevel     logLevel = parseLogLevel(cfg.logLevelStr);

    // ── 2. Build the simulator ──────────────────────────────
    Simulator sim(logLevel);

    // Enable simulated link latency if requested
    if (cfg.simulateLatency) {
        sim.enableLatency(cfg.latencyMinMs, cfg.latencyMaxMs);
    }

    sim.setupNetwork(cfg.numRouters, topo, cfg.seed);

    // ── 3. Single packet test ───────────────────────────────
    std::cout << "--- Single Packet Test ---\n\n";

    auto result = sim.sendPacket(cfg.sourceRouter, cfg.destRouter,
                                 "test_payload");
    sim.printResult(result);

    // ── 4. Batch simulation ─────────────────────────────────
    std::cout << "\n--- Batch Simulation (" << cfg.numPackets
              << " packets, " << cfg.numRouters << " routers) ---\n";

    auto batch = sim.runBatch(cfg.numPackets, cfg.seed);
    sim.printBatchResult(batch);

    // ── Done ────────────────────────────────────────────────
    std::cout << "\n========================================\n";
    std::cout << "  Simulation completed successfully.\n";
    std::cout << "========================================\n";

    return 0;
}
