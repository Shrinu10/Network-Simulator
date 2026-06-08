#pragma once

#include <string>
#include <vector>
#include "Router.h"

/**
 * @brief Abstract base class for routing table generation strategies.
 *
 * Routing table generation is decoupled from the Network class so that
 * the strategy can be swapped without touching forwarding logic.
 *
 * Current implementation:  BFS (guarantees valid forwarding paths)
 * Future implementations:  privacy-preserving, secret-sharing-based,
 *                          random-with-validation, etc.
 */
class RoutingTableGenerator {
public:
    virtual ~RoutingTableGenerator() = default;

    /**
     * @brief Populate routing tables for all routers.
     * @param routers        Vector of routers whose tables will be filled.
     * @param adjacencyList  The network topology as an adjacency list.
     */
    virtual void generate(std::vector<Router>& routers,
                          const std::vector<std::vector<int>>& adjacencyList) = 0;

    /// Human-readable name of the strategy (for logging).
    virtual std::string name() const = 0;
};

// ─────────────────────────────────────────────────────────────────
// Concrete strategy: BFS-based routing table generation
// ─────────────────────────────────────────────────────────────────

/**
 * @brief Generates routing tables using Breadth-First Search.
 *
 * For each router, runs BFS over the adjacency list to discover the
 * first-hop neighbour on the shortest path to every other router.
 * This guarantees loop-free, valid forwarding for any connected graph.
 *
 * Note: BFS is used here *only* for table initialization — packet
 * forwarding itself is a pure table lookup with no path computation.
 */
class BFSRoutingTableGenerator : public RoutingTableGenerator {
public:
    void generate(std::vector<Router>& routers,
                  const std::vector<std::vector<int>>& adjacencyList) override;

    std::string name() const override { return "BFS"; }
};
