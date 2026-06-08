#pragma once

#include <string>

/**
 * @brief Represents a network packet traveling through the simulated network.
 *
 * Carries source/destination identifiers, a payload string, and a TTL
 * (time-to-live) counter that is decremented at each hop to prevent
 * infinite forwarding loops.
 */
class Packet {
public:
    int sourceId;
    int destinationId;
    std::string payload;
    int ttl;  // Time-to-live: max hops before the packet is dropped

    /**
     * @param src      Source router ID
     * @param dst      Destination router ID
     * @param data     Payload string
     * @param maxHops  Maximum number of hops (default 256)
     */
    Packet(int src, int dst, const std::string& data, int maxHops = 256);
};
