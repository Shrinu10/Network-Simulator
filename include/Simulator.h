#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Network.h"

// ─────────────────────────────────────────────────────────────────
// Per-packet result
// ─────────────────────────────────────────────────────────────────

struct SimulationResult {
    int              source;
    int              destination;
    std::vector<int> path;
    int              hopCount;
    double           latencyMicroseconds;
    bool             delivered;
    std::string      failureReason;
};

// ─────────────────────────────────────────────────────────────────
// Aggregate batch result
// ─────────────────────────────────────────────────────────────────

struct BatchResult {
    int    totalPackets;
    int    delivered;
    int    dropped;
    double avgLatencyUs;
    double minLatencyUs;
    double maxLatencyUs;
    double avgHops;
};

// ─────────────────────────────────────────────────────────────────
// Log verbosity
// ─────────────────────────────────────────────────────────────────

enum class LogLevel { SILENT, INFO, DEBUG };

// ─────────────────────────────────────────────────────────────────
// Simulator
// ─────────────────────────────────────────────────────────────────

/**
 * @brief Orchestrates simulations: topology setup, packet sending,
 *        timing, logging, and statistics collection.
 *
 * Timing uses std::chrono::steady_clock (guaranteed monotonic).
 * Measures raw computation time only — no simulated link latency.
 */
class Simulator {
private:
    Network  network;
    LogLevel logLevel;

    void logInfo(const std::string& msg) const;
    void logDebug(const std::string& msg) const;

public:
    explicit Simulator(LogLevel level = LogLevel::INFO);

    /**
     * @brief Build a network with the given size and topology.
     * @param seed  RNG seed for reproducibility (0 = non-deterministic).
     */
    void setupNetwork(int numRouters, TopologyType type, unsigned int seed = 0);

    /**
     * @brief Replace the routing-table generation strategy.
     *        Must be called *before* setupNetwork().
     */
    void setRoutingTableGenerator(std::unique_ptr<RoutingTableGenerator> gen);

    /**
     * @brief Send a single packet and measure forwarding time.
     */
    SimulationResult sendPacket(int source, int destination,
                                const std::string& payload = "test_payload");

    /**
     * @brief Send numPackets packets between random source/destination pairs
     *        and collect aggregate statistics.
     */
    BatchResult runBatch(int numPackets, unsigned int seed = 0);

    void printResult(const SimulationResult& result) const;
    void printBatchResult(const BatchResult& result) const;

    int getRouterCount() const;
};
