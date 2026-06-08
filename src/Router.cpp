#include "Router.h"

Router::Router(int id)
    : routerId(id) {}

int Router::getId() const {
    return routerId;
}

int Router::getNextHop(int destinationId) const {
    if (destinationId < 0 ||
        destinationId >= static_cast<int>(routingTable.size())) {
        return -1;
    }
    return routingTable[destinationId];
}

void Router::setRoute(int destination, int nextHop) {
    if (destination >= 0 &&
        destination < static_cast<int>(routingTable.size())) {
        routingTable[destination] = nextHop;
    }
}

void Router::initRoutingTable(int numRouters) {
    routingTable.assign(numRouters, -1);

    // A router can always reach itself directly
    if (routerId >= 0 && routerId < numRouters) {
        routingTable[routerId] = routerId;
    }
}
