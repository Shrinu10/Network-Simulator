#include "Simulator.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>

// ═══════════════════════════════════════════════════════════════════
// Logging helpers
// ═══════════════════════════════════════════════════════════════════

void Simulator::logInfo(const std::string& msg) const {
    if (logLevel >= LogLevel::INFO) {
        std::cout << "[INFO]  " << msg << "\n";
    }
}

void Simulator::logDebug(const std::string& msg) const {
    if (logLevel >= LogLevel::DEBUG) {
        std::cout << "[DEBUG] " << msg << "\n";
    }
}

// ═══════════════════════════════════════════════════════════════════
// Construction & setup
// ═══════════════════════════════════════════════════════════════════

Simulator::Simulator(LogLevel level)
    : logLevel(level) {}

void Simulator::setupNetwork(int numRouters, TopologyType type,
                              unsigned int seed)
{
    network.generateTopology(numRouters, type, seed);
}

void Simulator::setRoutingTableGenerator(
    std::unique_ptr<RoutingTableGenerator> gen)
{
    network.setRoutingTableGenerator(std::move(gen));
}

void Simulator::enableLatency(double minMs, double maxMs) {
    network.enableLatency(minMs, maxMs);
}

int Simulator::getRouterCount() const {
    return network.getRouterCount();
}

// ═══════════════════════════════════════════════════════════════════
// Single packet transmission with timing
// ═══════════════════════════════════════════════════════════════════

SimulationResult Simulator::sendPacket(int source, int destination,
                                        const std::string& payload)
{
    Packet packet(source, destination, payload);

    logInfo("Sending packet: Router " + std::to_string(source)
            + " -> Router " + std::to_string(destination));

    // ── Measure raw forwarding time (steady_clock = monotonic) ──
    auto start = std::chrono::steady_clock::now();
    auto fwdResult = network.forwardPacket(packet);
    auto end   = std::chrono::steady_clock::now();

    std::chrono::duration<double, std::micro> elapsed = end - start;

    // ── Log the hop-by-hop path ─────────────────────────────────
    if (logLevel >= LogLevel::INFO && fwdResult.path.size() > 1) {
        for (size_t i = 0; i < fwdResult.path.size() - 1; ++i) {
            logInfo("Router " + std::to_string(fwdResult.path[i])
                    + " forwarded packet to Router "
                    + std::to_string(fwdResult.path[i + 1]));
        }
    }

    // ── Build result ────────────────────────────────────────────
    SimulationResult result;
    result.source              = source;
    result.destination         = destination;
    result.path                = std::move(fwdResult.path);
    result.hopCount            = static_cast<int>(result.path.size()) - 1;
    result.latencyMicroseconds = elapsed.count();
    result.simulatedLatencyMs  = fwdResult.simulatedLatencyMs;
    result.delivered           = fwdResult.delivered;
    result.failureReason       = std::move(fwdResult.failureReason);

    if (result.delivered) {
        logInfo("Packet delivered successfully");
    } else {
        logInfo("Packet DROPPED: " + result.failureReason);
    }

    return result;
}

// ═══════════════════════════════════════════════════════════════════
// Batch simulation
// ═══════════════════════════════════════════════════════════════════

BatchResult Simulator::runBatch(int numPackets, unsigned int seed) {
    const int n = network.getRouterCount();
    if (n < 2) {
        return {numPackets, 0, numPackets, 0, 0, 0, 0, 0, 0, 0};
    }

    unsigned int effectiveSeed = seed;
    if (effectiveSeed == 0) {
        effectiveSeed = std::random_device{}();
    }
    std::mt19937 rng(effectiveSeed);
    std::uniform_int_distribution<int> nodeDist(0, n - 1);

    BatchResult batch{};
    batch.totalPackets = numPackets;
    batch.delivered    = 0;
    batch.dropped      = 0;
    batch.avgLatencyUs = 0.0;
    batch.minLatencyUs = std::numeric_limits<double>::max();
    batch.maxLatencyUs = 0.0;
    batch.avgHops      = 0.0;
    batch.avgSimLatencyMs = 0.0;
    batch.minSimLatencyMs = std::numeric_limits<double>::max();
    batch.maxSimLatencyMs = 0.0;

    double totalLatency    = 0.0;
    double totalHops       = 0.0;
    double totalSimLatency = 0.0;

    // Suppress per-hop logs during batch (use SILENT internally)
    LogLevel savedLevel = logLevel;
    logLevel = LogLevel::SILENT;

    for (int i = 0; i < numPackets; ++i) {
        int src = nodeDist(rng);
        int dst = nodeDist(rng);
        while (dst == src) {
            dst = nodeDist(rng);   // ensure src != dst
        }

        auto result = sendPacket(src, dst);

        if (result.delivered) {
            ++batch.delivered;
            totalLatency    += result.latencyMicroseconds;
            totalHops       += result.hopCount;
            totalSimLatency += result.simulatedLatencyMs;
            batch.minLatencyUs = std::min(batch.minLatencyUs,
                                          result.latencyMicroseconds);
            batch.maxLatencyUs = std::max(batch.maxLatencyUs,
                                          result.latencyMicroseconds);
            batch.minSimLatencyMs = std::min(batch.minSimLatencyMs,
                                             result.simulatedLatencyMs);
            batch.maxSimLatencyMs = std::max(batch.maxSimLatencyMs,
                                             result.simulatedLatencyMs);
        } else {
            ++batch.dropped;
        }
    }

    logLevel = savedLevel;  // restore original log level

    if (batch.delivered > 0) {
        batch.avgLatencyUs    = totalLatency    / batch.delivered;
        batch.avgHops         = totalHops       / batch.delivered;
        batch.avgSimLatencyMs = totalSimLatency / batch.delivered;
    } else {
        batch.minLatencyUs    = 0.0;
        batch.minSimLatencyMs = 0.0;
    }

    return batch;
}

// ═══════════════════════════════════════════════════════════════════
// Pretty-printing
// ═══════════════════════════════════════════════════════════════════

void Simulator::printResult(const SimulationResult& r) const {
    std::cout << "\n+-------------------------------------+\n";
    std::cout << "|        Simulation Result            |\n";
    std::cout << "+-------------------------------------+\n";

    std::cout << "|  Source:       Router " << r.source << "\n";
    std::cout << "|  Destination:  Router " << r.destination << "\n";
    std::cout << "|  Path:         ";
    for (size_t i = 0; i < r.path.size(); ++i) {
        if (i > 0) std::cout << " -> ";
        std::cout << r.path[i];
    }
    std::cout << "\n";

    std::cout << "|  Hop Count:    " << r.hopCount << "\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "|  Comp. Time:   " << r.latencyMicroseconds << " us\n";
    if (r.simulatedLatencyMs > 0.0) {
        std::cout << "|  Sim. Latency: " << r.simulatedLatencyMs << " ms\n";
    }
    std::cout << "|  Status:       "
              << (r.delivered ? "Delivered [OK]" : "DROPPED [X]") << "\n";
    if (!r.delivered) {
        std::cout << "|  Reason:       " << r.failureReason << "\n";
    }

    std::cout << "+-------------------------------------+\n";
}

void Simulator::printBatchResult(const BatchResult& b) const {
    std::cout << "\n+=====================================+\n";
    std::cout << "|      Batch Simulation Results       |\n";
    std::cout << "+=====================================+\n";

    std::cout << "|  Total Packets:   " << b.totalPackets << "\n";
    std::cout << "|  Delivered:       " << b.delivered << "\n";
    std::cout << "|  Dropped:         " << b.dropped << "\n";

    double successRate = (b.totalPackets > 0)
        ? (100.0 * b.delivered / b.totalPackets) : 0.0;
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "|  Success Rate:    " << successRate << "%\n";

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "|  Avg Comp. Time:  " << b.avgLatencyUs << " us\n";
    std::cout << "|  Min Comp. Time:  " << b.minLatencyUs << " us\n";
    std::cout << "|  Max Comp. Time:  " << b.maxLatencyUs << " us\n";

    if (b.avgSimLatencyMs > 0.0) {
        std::cout << "|  --- Simulated Latency ---\n";
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "|  Avg Sim. Lat.:   " << b.avgSimLatencyMs << " ms\n";
        std::cout << "|  Min Sim. Lat.:   " << b.minSimLatencyMs << " ms\n";
        std::cout << "|  Max Sim. Lat.:   " << b.maxSimLatencyMs << " ms\n";
    }

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "|  Avg Hops:        " << b.avgHops << "\n";

    std::cout << "+=====================================+\n";
}
