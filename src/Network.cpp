#include "Network.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <stdexcept>

// ═══════════════════════════════════════════════════════════════════
// Construction
// ═══════════════════════════════════════════════════════════════════

Network::Network()
    : routingGenerator(std::make_unique<BFSRoutingTableGenerator>()) {}

void Network::setRoutingTableGenerator(
    std::unique_ptr<RoutingTableGenerator> gen)
{
    routingGenerator = std::move(gen);
}

// ═══════════════════════════════════════════════════════════════════
// Link helpers
// ═══════════════════════════════════════════════════════════════════

void Network::addLink(int a, int b) {
    adjacencyList[a].push_back(b);
    adjacencyList[b].push_back(a);
}

bool Network::isConnected() const {
    const int n = static_cast<int>(routers.size());
    if (n == 0) return true;

    std::vector<bool> visited(n, false);
    std::queue<int> q;
    q.push(0);
    visited[0] = true;
    int count = 1;

    while (!q.empty()) {
        int cur = q.front();
        q.pop();
        for (int nb : adjacencyList[cur]) {
            if (!visited[nb]) {
                visited[nb] = true;
                ++count;
                q.push(nb);
            }
        }
    }
    return count == n;
}

// ═══════════════════════════════════════════════════════════════════
// Topology generators
// ═══════════════════════════════════════════════════════════════════

void Network::generateRing(int n) {
    for (int i = 0; i < n; ++i) {
        addLink(i, (i + 1) % n);
    }
}

void Network::generateMesh(int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            addLink(i, j);
        }
    }
}

void Network::generateTree(int n) {
    // Binary tree: node i connects to children 2i+1 and 2i+2
    for (int i = 0; i < n; ++i) {
        int left  = 2 * i + 1;
        int right = 2 * i + 2;
        if (left < n)  addLink(i, left);
        if (right < n) addLink(i, right);
    }
}

void Network::generateRandom(int n, unsigned int seed) {
    // Build a guaranteed-connected random graph:
    //
    // Step 1 — Random spanning tree (ensures connectivity)
    //   Shuffle node order, then connect each node to a random
    //   previously-visited node.  Result: a random tree on N nodes.
    //
    // Step 2 — Extra edges (richer connectivity)
    //   Add approximately N more random edges, roughly doubling the
    //   edge count from the spanning tree.

    std::mt19937 rng(seed);

    // ── Step 1: random spanning tree ──────────────────────────
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin() + 1, order.end(), rng);

    for (int i = 1; i < n; ++i) {
        std::uniform_int_distribution<int> dist(0, i - 1);
        addLink(order[i], order[dist(rng)]);
    }

    // ── Step 2: extra random edges ───────────────────────────
    const int extraTarget = n;  // approximately N extra edges
    std::uniform_int_distribution<int> nodeDist(0, n - 1);
    int added = 0;

    for (int attempt = 0; attempt < extraTarget * 3 && added < extraTarget;
         ++attempt) {
        int a = nodeDist(rng);
        int b = nodeDist(rng);
        if (a == b) continue;

        // Check for duplicate edge
        bool exists = false;
        for (int nb : adjacencyList[a]) {
            if (nb == b) { exists = true; break; }
        }
        if (!exists) {
            addLink(a, b);
            ++added;
        }
    }
}

// ═══════════════════════════════════════════════════════════════════
// Public: generate full topology
// ═══════════════════════════════════════════════════════════════════

void Network::generateTopology(int numRouters, TopologyType type,
                                unsigned int seed)
{
    if (numRouters <= 0) {
        throw std::invalid_argument("numRouters must be positive");
    }

    // ── Reset state ──────────────────────────────────────────
    routers.clear();
    adjacencyList.clear();
    routers.reserve(numRouters);
    adjacencyList.resize(numRouters);

    for (int i = 0; i < numRouters; ++i) {
        routers.emplace_back(i);
    }

    // ── Determine RNG seed ───────────────────────────────────
    unsigned int effectiveSeed = seed;
    if (effectiveSeed == 0) {
        effectiveSeed = std::random_device{}();
    }

    // ── Build links ──────────────────────────────────────────
    switch (type) {
        case TopologyType::RING:   generateRing(numRouters);              break;
        case TopologyType::MESH:   generateMesh(numRouters);              break;
        case TopologyType::TREE:   generateTree(numRouters);              break;
        case TopologyType::RANDOM: generateRandom(numRouters, effectiveSeed); break;
    }

    // ── Sanity check ─────────────────────────────────────────
    if (!isConnected()) {
        throw std::runtime_error(
            "Generated topology is not connected — "
            "this should not happen with the built-in generators");
    }

    // ── Populate routing tables ──────────────────────────────
    std::cout << "[SETUP] Generating routing tables ("
              << routingGenerator->name() << ") for "
              << numRouters << " routers...\n";

    routingGenerator->generate(routers, adjacencyList);

    std::cout << "[SETUP] Topology ready: " << topologyName(type)
              << " with " << numRouters << " routers\n\n";
}

// ═══════════════════════════════════════════════════════════════════
// Packet forwarding — pure table lookup, no path computation
// ═══════════════════════════════════════════════════════════════════

Network::ForwardResult Network::forwardPacket(Packet& packet) const {
    ForwardResult result;
    result.delivered = false;

    const int n = static_cast<int>(routers.size());

    // Validate source and destination
    if (packet.sourceId < 0 || packet.sourceId >= n) {
        result.failureReason = "Invalid source router ID: "
                               + std::to_string(packet.sourceId);
        return result;
    }
    if (packet.destinationId < 0 || packet.destinationId >= n) {
        result.failureReason = "Invalid destination router ID: "
                               + std::to_string(packet.destinationId);
        return result;
    }

    int current = packet.sourceId;
    result.path.push_back(current);

    while (current != packet.destinationId) {
        if (packet.ttl <= 0) {
            result.failureReason = "TTL exceeded (possible routing loop)";
            return result;
        }

        int nextHop = routers[current].getNextHop(packet.destinationId);
        if (nextHop < 0) {
            result.failureReason =
                "No route from router " + std::to_string(current)
                + " to destination " + std::to_string(packet.destinationId);
            return result;
        }

        --packet.ttl;
        current = nextHop;
        result.path.push_back(current);
    }

    result.delivered = true;
    return result;
}

// ═══════════════════════════════════════════════════════════════════
// Accessors
// ═══════════════════════════════════════════════════════════════════

int Network::getRouterCount() const {
    return static_cast<int>(routers.size());
}

const Router& Network::getRouter(int id) const {
    return routers.at(id);
}

std::string Network::topologyName(TopologyType type) {
    switch (type) {
        case TopologyType::RING:   return "Ring";
        case TopologyType::MESH:   return "Full Mesh";
        case TopologyType::TREE:   return "Binary Tree";
        case TopologyType::RANDOM: return "Random Connected Graph";
    }
    return "Unknown";
}
