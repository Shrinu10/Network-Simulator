#include "Packet.h"

Packet::Packet(int src, int dst, const std::string& data, int maxHops)
    : sourceId(src)
    , destinationId(dst)
    , payload(data)
    , ttl(maxHops) {}
