#pragma once
#include <vector>
#include <algorithm>
#include <numeric>

class GreedyColoring
{
public:
    template<class Matrix>
    static std::vector<int> color(const Matrix& adj) {
        const int n = adj.rows();

        /* ---------- 1. строим списки смежности и степени ------------------ */
        std::vector<std::vector<int>> g(n);
        std::vector<int> degree(n,0);
        for (int i=0;i<n;++i)
            for (int j=0;j<n;++j)
                if (adj(i,j)) { g[i].push_back(j); ++degree[i]; }

        /* ---------- 2. Welsh–Powell порядок (степень ↓) ------------------- */
        std::vector<int> order(n);
        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(),
                  [&](int a,int b){ return degree[a] > degree[b]; });

        /* ---------- 3. greedy coloring ------------------------------------ */
        std::vector<int> color(n, -1);
        std::vector<int> mark(n, 0);   // mark[c] == stamp → цвет c занят
        int stamp = 1;

        for (int v : order) {
            for (int u : g[v])
                if (color[u] != -1) mark[color[u]] = stamp;

            int c = 0;
            while (c < n && mark[c] == stamp) ++c;
            color[v] = c;

            ++stamp;                   // «очищаем» mark без fill
        }
        return color;                  // χ = 1 + *max(color)*
    }
};
