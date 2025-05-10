#ifndef OLEMSKOY_COLOR_GRAPH_H
#define OLEMSKOY_COLOR_GRAPH_H

#include <vector>
#include <unordered_set>
#include "Graph.h"

class OlemskoyColorGraph {
private:
    Graph g;
    int n;
    int bestColorCount;
    std::vector<std::vector<int>> bestPartition;

    // Tracking for search state
    std::vector<bool> used;                        // which vertices are already colored (used in previous blocks)
    std::vector<int> currentNodes;                 // which vertices in current block are already colored (used in previous blocks)
    std::vector<std::vector<int>> currentPartition; // current partial coloring (blocks built so far)
    std::unordered_set<long long> firstBlockSeen;   // memory of first-block configurations (for symmetry pruning D)

    // Recursive search functions
    void searchBlocks(int currentBlockIndex);
    void buildBlock(int blockIndex, std::vector<int>& currentBlock,const std::vector<int>& omega, int level, int initialSize);

public:
    // Constructor
    OlemskoyColorGraph(const Graph& matrix);

    // Returns groups of vertex indices for the color classes (solution partition)
    std::vector<std::vector<int>> resultColorNodes();
};

#endif // OLEMSKOY_COLOR_GRAPH_H
