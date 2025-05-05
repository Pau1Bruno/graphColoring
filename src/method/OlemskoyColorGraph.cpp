#include "OlemskoyColorGraph.h"
// #include "Graph.h"
#include <iostream>
#include <cmath>
#include <functional>
#include <algorithm>

OlemskoyColorGraph::OlemskoyColorGraph(const Graph& matrix): g(matrix) {
    n = g.size();
    bestColorCount = n;           // initial record (upper bound) = n (worst-case each vertex its own block)
    used.assign(n, false);
}

std::vector<std::vector<int>> OlemskoyColorGraph::resultColorNodes() {
    // Clear any previous data
    bestPartition.clear();
    currentPartition.clear();
    firstBlockSeen.clear();

    // Start recursive search from block 1
    searchBlocks(1);
    return bestPartition;
}

void OlemskoyColorGraph::searchBlocks(int currentBlockIndex) {
    // Check if all vertices are colored (used)
    bool allColored = true;
    for (int v = 0; v < n; ++v) {
        if (!used[v]) {
            allColored = false;
            break;
        }
    }
    if (allColored) {
        // All vertices covered with (currentBlockIndex - 1) blocks
        int blocksUsed = currentBlockIndex - 1;
        std::cout << "All vertices colored using " << blocksUsed << " blocks.\n";
        if (blocksUsed < bestColorCount) {
            bestColorCount = blocksUsed;
            bestPartition = currentPartition;
            std::cout << "New best coloring found with " << blocksUsed << " blocks.\n";
        }
        return;
    }

    // Prune if we've already used equal or more blocks than the best found
    if (currentBlockIndex > bestColorCount) {
        std::cout << "[Prune A] Already using " << (currentBlockIndex - 1) 
                  << " blocks (>= best " << bestColorCount << "). Backtracking.\n";
        return;
    }

    // Compute a lower bound on total blocks needed (using clique number of remaining graph)
    std::vector<int> remain;
    remain.reserve(n);
    for (int v = 0; v < n; ++v) {
        if (!used[v]) remain.push_back(v);
    }
    // Find size of largest clique in subgraph induced by 'remain'
    int maxCliqueSize = 0;
    std::vector<int> clique;
    std::function<void(int)> dfsClique = [&](int start) {
        for (int i = start; i < (int)remain.size(); ++i) {
            int v = remain[i];
            bool canExtend = true;
            // check adjacency with current clique members
            for (int u : clique) {
                if (!g.areAdjacent(u, v)) { // if not adjacent, cannot form a clique
                    canExtend = false;
                    break;
                }
            }
            if (!canExtend) continue;
            clique.push_back(v);
            maxCliqueSize = std::max(maxCliqueSize, (int)clique.size());
            dfsClique(i + 1);
            clique.pop_back();
        }
    };
    dfsClique(0);
    int lowerBoundBlocks = (currentBlockIndex - 1) + maxCliqueSize;
    if (lowerBoundBlocks > bestColorCount) {
        std::cout << "[Prune A] Lower bound = " << lowerBoundBlocks 
                  << " blocks (>" << bestColorCount << " best). Backtracking.\n";
        return;
    }

    // Begin constructing the current block (block 'currentBlockIndex')
    std::vector<int> support = remain;  // candidates for this block (initially all remaining vertices)
    // (support is inherently sorted by vertex index ascending due to how 'remain' was constructed)
    std::vector<int> currentBlock;      // vertices currently in this block (being built)

    std::cout << "Starting Block " << currentBlockIndex << " (Level 1) with initial support: {";
    for (size_t k = 0; k < support.size(); ++k) {
        std::cout << support[k] << ((k + 1 < support.size()) ? "," : "");
    }
    std::cout << "}\n";

    // Recursively build the block (level=1, record initial support size for check C)
    buildBlock(currentBlockIndex, currentBlock, support, 1, support.size());
}

void OlemskoyColorGraph::buildBlock(int blockIndex, std::vector<int>& currentBlock, 
                                    std::vector<int>& support, int level, int initialSize) {
    // If support is empty, we've completed this block with an even number of vertices (no leftover)
    if (support.empty()) {
        // Close the block and move to the next block in the coloring
        std::cout << "Block " << blockIndex << " completed (even size: " 
                  << currentBlock.size() << " vertices).\n";
        // Prepare to recurse for the next block
        // Check symmetry pruning D for first block:
        if (blockIndex == 1) {
            // Compute bit mask of this block's vertices for identification
            long long mask = 0;
            for (int v : currentBlock) mask |= (1LL << v);
            if (firstBlockSeen.find(mask) != firstBlockSeen.end()) {
                std::cout << "[Prune D] Block 1 configuration already considered, skipping further search.\n";
            } else {
                firstBlockSeen.insert(mask);
                // Add this block to current partition and recurse
                currentPartition.push_back(currentBlock);
                std::cout << "Proceeding to Block " << (blockIndex + 1) << " after closing Block " 
                          << blockIndex << ".\n";
                searchBlocks(blockIndex + 1);
                currentPartition.pop_back();
            }
        } else {
            // No symmetry check for later blocks
            currentPartition.push_back(currentBlock);
            std::cout << "Proceeding to Block " << (blockIndex + 1) << " after closing Block " 
                      << blockIndex << ".\n";
            searchBlocks(blockIndex + 1);
            currentPartition.pop_back();
        }
        return; // backtrack (block finished)
    }

    // Form list of all candidate pivot pairs (q,r) in 'support' that are non-adjacent
    struct PairInfo { int u; int v; int commonCount; };
    std::vector<PairInfo> pivotPairs;
    pivotPairs.reserve(support.size() * (support.size() - 1) / 2);
    for (size_t i = 0; i < support.size(); ++i) {
        for (size_t j = i + 1; j < support.size(); ++j) {
            int u = support[i];
            int v = support[j];
            if (!g.areAdjacent(u, v)) {
                // Compute how many vertices in support (excluding u,v) are common non-neighbors of u and v
                int commonCount = 0;
                for (int w : support) {
                    if (w == u || w == v) continue;
                    if (!g.areAdjacent(u, w) && !g.areAdjacent(v, w)) {
                        commonCount++;
                    }
                }
                pivotPairs.push_back({u, v, commonCount});
            }
        }
    }
    // Sort pivot pairs by descending commonCount (heuristic: choose pair with max intersection first)
    std::sort(pivotPairs.begin(), pivotPairs.end(), [](const PairInfo& a, const PairInfo& b){
        if (a.commonCount != b.commonCount) return a.commonCount > b.commonCount;
        // tie-break: maybe by smallest vertex to have deterministic order
        if (a.u != b.u) return a.u < b.u;
        return a.v < b.v;
    });

    // Memory for "used" pivot pairs at this level is implicit by iterating pivotPairs vector (no duplicates)
    // Try each pivot pair as a branching option
    for (auto& pair : pivotPairs) {
        int p = pair.u;
        int q = pair.v;
        int intersectionSize = pair.commonCount;
        // Pruning Check B: first block, ensure potential block size >= ceil(n / bestColorCount)
        if (blockIndex == 1) {
            // Already have (level-1)*2 vertices in currentBlock (from higher levels)
            // plus potentially adding this pair and its intersection
            int potentialSize = 2 * (level - 1) + (int)intersectionSize + 2; 
            // We add 2 for the current pair itself, and intersectionSize is how many more could possibly join after
            // Actually, check B formula from description: 2(s-1) + ρ^{1,s} < ceil(n / v0)
            // They define ρ^{1,s} as |D_{(p,q)}| (common nonneighbors count) if pivot exists.
            // D_{(p,q)} here is essentially intersection plus the pair? Or just intersection excluding the pair?
            // The text suggests ρ = |D|, likely excluding the pair itself. We computed commonCount excluding the pair.
            // So 2(s-1) + ρ^{1,s} = 2*(level-1) + intersectionSize.
            // However, since we consider adding this pair, a more intuitive potential block size = currentBlock.size() + 2 + intersectionSize.
            // We'll use the formula as given: 2*(level-1) + (intersectionSize) < ceil(n / bestColorCount).
            int avgTarget = (int)std::ceil(n * 1.0 / bestColorCount);
            if (2 * (level - 1) + intersectionSize < avgTarget) {
                std::cout << "[Prune B] Block 1 potential too small (" 
                          << (2 * (level - 1) + intersectionSize) << " < ceil(" << n << "/" 
                          << bestColorCount << ")=" << avgTarget << "). Skipping pivot (" 
                          << p << "," << q << ").\n";
                continue;
            }
        }
        // Pruning Check C: if building the last allowed block (equals current best), avoid branches that only achieve tie
        if (blockIndex == bestColorCount) {
            // If this pivot's common intersection plus already in block equals initial support, it will use up all remaining vertices exactly
            // meaning we'll end up with 'bestColorCount' blocks in total (tie, not improvement).
            // Check if 2*(level-1) + (intersection + 2) equals initial size.
            if (2 * (level - 1) + (int)currentBlock.size() + 2 + intersectionSize == initialSize) {
                std::cout << "[Prune C] Branch would complete coloring with " << bestColorCount 
                          << " blocks (no improvement). Skipping pivot (" << p << "," << q << ").\n";
                continue;
            }
        }
        // Choose this pivot pair (p,q) for current level
        std::cout << "Level " << level << ": choose pivot (" << p << "," << q 
                  << "), common candidates = " << intersectionSize << ".\n";

        // Mark p and q as used (colored in this block)
        used[p] = true;
        used[q] = true;
        // Add them to current block
        currentBlock.push_back(p);
        currentBlock.push_back(q);
        // Ensure the block's vertex list remains sorted (for consistency) 
        // (Not strictly necessary for correctness, but good for output and mask consistency)
        std::sort(currentBlock.begin(), currentBlock.end());

        // Compute new support for next level = old support ∩ nonneighbors(p) ∩ nonneighbors(q) \ {p,q}
        std::vector<int> newSupport;
        newSupport.reserve(support.size());
        for (int w : support) {
            if (w == p || w == q) continue;
            if (!g.areAdjacent(p, w) && !g.areAdjacent(q, w)) {
                newSupport.push_back(w);
            }
        }
        // newSupport is naturally sorted because we iterated sorted 'support'

        std::cout << "Level " << level << ": new support after adding (" << p << "," << q << "): {";
        for (size_t k = 0; k < newSupport.size(); ++k) {
            std::cout << newSupport[k] << ((k + 1 < newSupport.size()) ? "," : "");
        }
        std::cout << "}.\n";

        // Recurse to next level within this same block
        buildBlock(blockIndex, currentBlock, newSupport, level + 1, initialSize);

        // Backtrack: remove p,q from current block and mark them unused
        currentBlock.erase(std::remove(currentBlock.begin(), currentBlock.end(), p), currentBlock.end());
        currentBlock.erase(std::remove(currentBlock.begin(), currentBlock.end(), q), currentBlock.end());
        used[p] = false;
        used[q] = false;
        std::cout << "Level " << level << ": backtrack from pivot (" << p << "," << q << ").\n";
    }

    // After exploring all pivot pairs at this level, or if none was available, consider closing block with a leftover vertex
    if (!support.empty()) {
        // If no pivot pairs at this level or all pivot branches explored, try adding one leftover vertex to finish this block
        std::cout << "Level " << level << ": ";
        if (!pivotPairs.empty()) {
            std::cout << "exhausted pivot options, closing block with a leftover.\n";
        } else {
            std::cout << "no pivot pair available, closing block with a leftover.\n";
        }
        // Loop through possible leftover vertices in support (prefer highest index first for consistency)
        for (int idx = support.size() - 1; idx >= 0; --idx) {
            int v = support[idx];
            // Prune Check C also applies here: if block is last allowed and taking this leftover uses exactly all initial vertices...
            if (blockIndex == bestColorCount) {
                if (2 * (level - 1) + (int)currentBlock.size() + 1 == initialSize) {
                    std::cout << "[Prune C] Branch would complete coloring with " << bestColorCount 
                              << " blocks (no improvement). Skipping leftover " << v << ".\n";
                    // We still mark it as considered and continue to next leftover
                    continue;
                }
            }
            // Take v as leftover to close the block
            used[v] = true;
            currentBlock.push_back(v);
            std::sort(currentBlock.begin(), currentBlock.end());
            std::cout << "Level " << level << ": add leftover vertex " << v << " to block and close it (odd block size).\n";
            // Close block with odd number of vertices
            if (blockIndex == 1) {
                long long mask = 0;
                for (int u : currentBlock) mask |= (1LL << u);
                if (firstBlockSeen.find(mask) != firstBlockSeen.end()) {
                    std::cout << "[Prune D] Block 1 configuration already considered, skipping further search.\n";
                } else {
                    firstBlockSeen.insert(mask);
                    currentPartition.push_back(currentBlock);
                    std::cout << "Proceeding to Block " << (blockIndex + 1) 
                              << " after closing Block " << blockIndex << " with leftover.\n";
                    searchBlocks(blockIndex + 1);
                    currentPartition.pop_back();
                }
            } else {
                currentPartition.push_back(currentBlock);
                std::cout << "Proceeding to Block " << (blockIndex + 1) 
                          << " after closing Block " << blockIndex << " with leftover.\n";
                searchBlocks(blockIndex + 1);
                currentPartition.pop_back();
            }
            // Backtrack: remove leftover
            currentBlock.erase(std::remove(currentBlock.begin(), currentBlock.end(), v), currentBlock.end());
            used[v] = false;
            std::cout << "Level " << level << ": backtrack from leftover " << v << ".\n";
        }
    }
}
