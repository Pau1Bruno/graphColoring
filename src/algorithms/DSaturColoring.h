#pragma once
#include <vector>
#include <algorithm>
#include <numeric>

class DSaturColoring
{
private:                          
    static std::vector<int>
    colorAdj(const std::vector<std::vector<int>>& g)
    {
        const int n = static_cast<int>(g.size());
        std::vector<int> degree(n), satDeg(n), color(n, -1);
        for (int v = 0; v < n; ++v) degree[v] = static_cast<int>(g[v].size());

        /* ---------- бакет-куча по сатурации ---------- */
        std::vector<std::vector<int>> bucket(n + 1);
        for (int v = 0; v < n; ++v) bucket[0].push_back(v);
        int curMaxSat = 0;

        auto pop_max = [&]() -> int {
            while (curMaxSat >= 0 && bucket[curMaxSat].empty()) --curMaxSat;
            auto &vec = bucket[curMaxSat];
            /*   tie-break: берём вершину с наибольшей степенью   */
            auto it = std::max_element(vec.begin(), vec.end(),
                       [&](int a,int b){ return degree[a] < degree[b]; });
            int v = *it;
            vec.erase(it);
            return v;
        };

        /* ---------- служебные массивы ---------- */
        std::vector<int> mark(n, 0);      // занятые цвета
        int stamp = 1, colored = 0;

        /* ---------- DSatur ---------- */
        while (colored < n)
        {
            int v = pop_max();
            if (color[v] != -1) continue; // мог быть перекрашен

            /* помечаем цвета соседей */
            for (int u : g[v])
                if (color[u] != -1) mark[color[u]] = stamp;

            int c = 0; while (mark[c] == stamp) ++c;
            color[v] = c;                     // красим
            ++stamp; ++colored;

            /* обновляем сатурации соседей */
            for (int u : g[v]) if (color[u] == -1)
            {
                bool newColor = true;
                for (int w : g[u]) if (color[w] == c) { newColor = false; break; }
                if (!newColor) continue;

                int& sd = satDeg[u];
                auto& vec = bucket[sd];                     // убираем из старого
                vec.erase(std::remove(vec.begin(), vec.end(), u), vec.end());

                ++sd;                                       // ↑ сатурация
                bucket[sd].push_back(u);                    // кладём в новый
                curMaxSat = std::max(curMaxSat, sd);
            }
        }
        return color;
    }

public:        

    /* --- списки смежности --- */
    static std::vector<int> color(const std::vector<std::vector<int>>& g)
    { return colorAdj(g); }

    /* --- 0/1-матрица (Eigen, std::vector, ...) --- */
    template<class Matrix>
    static auto color(const Matrix& M) -> decltype(M.rows(), std::vector<int>())
    {
        const int n = M.rows();
        std::vector<std::vector<int>> adj(n);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (M(i,j)) adj[i].push_back(j);
        return colorAdj(adj);
    }
};
