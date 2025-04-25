#include "GreedyColoring.h"
#include <algorithm>
#include <numeric>

std::vector<int> GreedyColoring::color(const MatrixType& adj) {
    int n = adj.rows();
    std::vector<int> result(n, -1);
    std::vector<char> used(n);

    // 1) Welsh–Powell: sort vertices by descending degree
    std::vector<std::pair<int,int>> deg; deg.reserve(n);
    for (int i = 0; i < n; ++i) {
        deg.emplace_back((adj.row(i).array() != 0).count(), i);
    }
    std::sort(deg.begin(), deg.end(),
              [](auto &a, auto &b){ return a.first > b.first; });
       
    // 2) Color in that order
    for (auto [d,v] : deg) {
        std::fill(used.begin(), used.end(), 0);
        for (int u = 0; u < n; ++u) {
            if (adj(v,u) && result[u] != -1)
                used[result[u]] = 1;
        }
        int c = 0;
        while (used[c]) ++c;
        result[v] = c;
    }
    return result;
}