#pragma once

#include <vector>

/**
 * @brief Represents a network router in the simulated network.
 *
 * Each router has a unique integer ID and a routing table that maps
 * destination router IDs to next-hop router IDs.
 *
 * The routing table is implemented as a std::vector<int> indexed by
 * destination ID, giving O(1) lookups with cache-friendly memory layout.
 * This works because router IDs are dense sequential integers (0..N-1).
 *
 * A value of -1 in the routing table means "no route to that destination."
 */
class Router {
private:
    int routerId;

public:
    /// routingTable[destination] = next_hop;  -1 = no route
    std::vector<int> routingTable;

    /**
     * @param id  Unique router identifier (0-based)
     */
    explicit Router(int id = -1);

    int getId() const;

    /**
     * @brief Look up the next hop for a given destination.
     * @return Next-hop router ID, or -1 if no route exists.
     */
    int getNextHop(int destinationId) const;

    /**
     * @brief Set the next-hop for a specific destination.
     */
    void setRoute(int destination, int nextHop);

    /**
     * @brief Allocate the routing table for a network of numRouters routers.
     *        Fills all entries with -1 (no route), except self -> self.
     */
    void initRoutingTable(int numRouters);
};
