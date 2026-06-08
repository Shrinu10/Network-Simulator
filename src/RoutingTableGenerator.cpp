#include "RoutingTableGenerator.h"

#include <queue>

void BFSRoutingTableGenerator::generate(
    std::vector<Router>& routers,
    const std::vector<std::vector<int>>& adjacencyList)
{
    const int n = static_cast<int>(routers.size());

    for (int source = 0; source < n; ++source) {
        routers[source].initRoutingTable(n);

        // BFS from `source` to discover the first hop toward every destination
        std::vector<int>  parent(n, -1);
        std::vector<bool> visited(n, false);
        std::queue<int>   bfsQueue;

        visited[source] = true;
        bfsQueue.push(source);

        while (!bfsQueue.empty()) {
            int current = bfsQueue.front();
            bfsQueue.pop();

            for (int neighbour : adjacencyList[current]) {
                if (!visited[neighbour]) {
                    visited[neighbour] = true;
                    parent[neighbour]  = current;
                    bfsQueue.push(neighbour);
                }
            }
        }

        // Trace the parent chain from each destination back to `source`
        // to find the immediate next-hop neighbour.
        for (int dest = 0; dest < n; ++dest) {
            if (dest == source)   continue;   // already set to self
            if (!visited[dest])   continue;   // unreachable (shouldn't happen in connected graph)

            int node = dest;
            while (parent[node] != source && parent[node] != -1) {
                node = parent[node];
            }

            if (parent[node] == source) {
                routers[source].setRoute(dest, node);
            }
        }
    }
}
