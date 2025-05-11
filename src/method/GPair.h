#pragma once
#include <vector>
#include <algorithm>
#include <iostream>

#include "Graph.h"
#include "Utils.h"

/*  -----  GPair и утилита contains  -----  */
struct GPair { 
    int i;
    int j;
    std::vector<int> set;
};

inline bool contains(const std::vector<int>& vec, int x)
{
    return std::find(vec.begin(), vec.end(), x) != vec.end();
}

/*  ---------  buildGPairsHV  -------------------------------------------
    adjency      – 0/1-матрица (std::vector<std::vector<bool>>), диагональ = 0
    support  – текущий Ω (0-индексированные вершины)

    возвращает отсортированный vector<GPair>
------------------------------------------------------------------------ */
inline std::vector<GPair>
buildGPairsHV(const Graph& g, const std::vector<int>& omega)
{
    const auto& adj = g.adjacency();
    const int n = static_cast<int>(adj.size());

    /* 2. GPair */
    std::vector<GPair> out;
    out.reserve(omega.size()*omega.size()/2);

    const auto& Hsets = g.Hsets();
    const auto& Vsets = g.Vsets();

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (!contains(omega, i) || !contains(omega, j)) continue;

            std::vector<int> d_qr, d_rq;
            for (int el : Hsets[i])
                if (contains(Vsets[j], el) && contains(omega, el)) d_qr.push_back(el);

            for (int el : Hsets[j])
                if (contains(Vsets[i], el) && contains(omega, el)) d_rq.push_back(el);

            std::vector<int> Dij;
            for (int el : d_qr)
                if (contains(d_rq, el)) Dij.push_back(el);

            if (!contains(Dij, i) || !contains(Dij, j)) continue;

            out.push_back({i, j, std::move(Dij)});
        }
    }
    std::sort(out.begin(), out.end(),
        [](const GPair& a,const GPair& b){
            if (a.set.size() != b.set.size()) return a.set.size() > b.set.size();
            if (a.i != b.i) return a.i < b.i;
            return a.j < b.j;
        });

    return out;
}
