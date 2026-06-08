#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Packet.h"
#include "Router.h"
#include "RoutingTableGenerator.h"

/// Supported built-in topology shapes.
enum class TopologyType { RING, MESH, TREE, RANDOM };

/**
 * @brief Represents the complete network topology and manages packet forwarding.
 *
 * Owns all Router objects and the adjacency list.  Topology is generated once
 * via generateTopology() and remains fixed for the duration of a simulation run.
 *
 * Routing tables are populated by a pluggable RoutingTableGenerator strategy
 * (default: BFS).  The generator can be swapped at any time before calling
 * generateTopology() — e.g., to use a privacy-preserving scheme later.
 */
class Network {
private:
    std::vector<Router>              routers;
    std::vector<std::vector<int>>    adjacencyList;
    std::unique_ptr<RoutingTableGenerator> routingGenerator;

    /// Add a bidirectional link between routers a and b.
    void addLink(int a, int b);

    /// Verify that the graph is fully connected (BFS from node 0).
    bool isConnected() const;

    // ── Topology builders ──────────────────────────────────────
    void generateRing(int n);
    void generateMesh(int n);
    void generateTree(int n);
    void generateRandom(int n, unsigned int seed);

public:
    /// Result of forwarding a single packet through the network.
    struct ForwardResult {
        std::vector<int> path;           ///< Ordered list of router IDs visited
        bool             delivered;      ///< true if packet reached destination
        std::string      failureReason;  ///< non-empty only on failure
    };

    Network();

    /**
     * @brief Swap in a custom routing-table generation strategy.
     *        Must be called *before* generateTopology().
     */
    void setRoutingTableGenerator(std::unique_ptr<RoutingTableGenerator> gen);

    /**
     * @brief Build the network: create routers, generate links, populate routing tables.
     * @param numRouters  Number of routers (nodes) in the network.
     * @param type        Topology shape.
     * @param seed        RNG seed for random topologies (0 = non-deterministic).
     */
    void generateTopology(int numRouters, TopologyType type, unsigned int seed = 0);

    /**
     * @brief Forward a packet hop-by-hop through the network using routing tables.
     *
     * Pure table-lookup forwarding — no path computation at runtime.
     * The packet's TTL is decremented at each hop.
     */
    ForwardResult forwardPacket(Packet& packet) const;

    int getRouterCount() const;
    const Router& getRouter(int id) const;

    /// Human-readable name for a topology type.
    static std::string topologyName(TopologyType type);
};
