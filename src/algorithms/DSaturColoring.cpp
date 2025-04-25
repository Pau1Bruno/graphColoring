#include "DSaturColoring.h"
#include <vector>
#include <algorithm>

std::vector<int> DSaturColoring::color(const MatrixType& adj) {
    const int n = adj.rows();
    std::vector<int> color(n, -1);
    std::vector<int> saturation(n, 0);
    std::vector<int> degree(n, 0);

    // 1) Compute initial degrees
    for (int i = 0; i < n; ++i) {
        int d = 0;
        for (int j = 0; j < n; ++j)
            if (adj(i, j) != 0) ++d;
        degree[i] = d;
    }

    // 2) Color n vertices
    for (int step = 0; step < n; ++step) {
        // a) Select uncolored vertex with max saturation (tie-break by degree)
        int bestV = -1, bestSat = -1, bestDeg = -1;
        for (int v = 0; v < n; ++v) {
            if (color[v] != -1) continue;
            if (saturation[v] > bestSat ||
               (saturation[v] == bestSat && degree[v] > bestDeg)) {
                bestSat = saturation[v];
                bestDeg = degree[v];
                bestV = v;
            }
        }

        // b) Find smallest available color for bestV
        std::vector<bool> used(n, false);
        for (int u = 0; u < n; ++u)
            if (adj(bestV, u) != 0 && color[u] != -1)
                used[color[u]] = true;

        int c = 0;
        while (c < n && used[c]) ++c;
        color[bestV] = c;

        // c) Update saturation of its uncolored neighbors
        for (int u = 0; u < n; ++u) {
            if (adj(bestV, u) != 0 && color[u] == -1) {
                // Check if color c is new for neighbor u
                bool seen = false;
                for (int k = 0; k < n; ++k) {
                    if (adj(u, k) != 0 && color[k] == c) {
                        seen = true;
                        break;
                    }
                }
                if (!seen) {
                    ++saturation[u];
                }
            }
        }
    }

    return color;
}