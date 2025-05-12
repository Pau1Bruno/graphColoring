#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <random>
#include <Eigen/Dense>


#include <type_traits>

#include "algorithms/GreedyHeuristicsColoring.h" 
#include "method/Graph.h"; 

using Clock = std::chrono::high_resolution_clock;

/*------------------------------------------------------------------*
 |  printColoring overloads:
 |   - greedy::Coloring
 |   - vector<int>
 |   - vector<vector<int>>
 *------------------------------------------------------------------*/

template<class Matrix>
inline void printColoring(const greedy::Coloring<Matrix>& solver)
{
    std::map<int, std::vector<int>> byColor;
    for (const auto& [v,c] : solver.colors()) byColor[c].push_back(v);

    std::cout << "χ = " << solver.chromaticNumber() << '\n';
    for (auto& [c, vs] : byColor) {
        std::cout << "Цвет " << c << ": ";
        for (size_t i = 0; i < vs.size(); ++i)
            std::cout << vs[i] + 1        // +1 если хотите 1-based вершины
                      << (i + 1 == vs.size() ? "" : ", ");
        std::cout << '\n';
    }
}

inline void printColoring(const std::vector<int>& col)
{
    std::map<int, std::vector<int>> byColor;
    for (size_t v = 0; v < col.size(); ++v)
        byColor[col[v] + 1].push_back(static_cast<int>(v));

    int chi = byColor.empty() ? 0 : byColor.rbegin()->first;
    std::cout << "χ = " << chi << '\n';
    for (auto& [c, vs] : byColor) {
        std::cout << "Цвет " << c << ": ";
        for (size_t i = 0; i < vs.size(); ++i)
            std::cout << vs[i] + 1 << (i + 1 == vs.size() ? "" : ", ");
        std::cout << '\n';
    }
}

inline void printColoring(const std::vector<std::vector<int>>& groups)
{
    int chi = static_cast<int>(groups.size());
    std::cout << "χ = " << chi << '\n';
    for (int c = 0; c < chi; ++c) {
        std::cout << "Цвет " << (c + 1) << ": ";
        const auto& verts = groups[c];
        for (size_t i = 0; i < verts.size(); ++i)
            std::cout << verts[i] << (i + 1 == verts.size() ? "" : ", ");
        std::cout << '\n';
    }
}

/*------------------------------------------------------------------*
 |  generateDenseMatrices: produce symmetric random 0/1 matrices
 *------------------------------------------------------------------*/

static std::vector<DenseMatrix>
generateDenseMatrices(int n, double density, int count)
{
    std::mt19937_64 rng{std::random_device{}()};
    std::bernoulli_distribution coin(density);
    auto gen = [&](Eigen::Index, Eigen::Index){ return static_cast<int>(coin(rng)); };

    std::vector<DenseMatrix> out;
    out.reserve(count);
    for (int k = 0; k < count; ++k) {
        DenseMatrix A = DenseMatrix::Zero(n, n);
        A.template triangularView<Eigen::StrictlyUpper>() =
            DenseMatrix::NullaryExpr(n, n, gen)
            .template triangularView<Eigen::StrictlyUpper>();
        A.template triangularView<Eigen::StrictlyLower>() =
            A.transpose().template triangularView<Eigen::StrictlyLower>();
        out.emplace_back(std::move(A));
    }
    return out;
}

/*------------------------------------------------------------------*
 |  timeit: measure function returning any T, returns pair<T,double>
 *------------------------------------------------------------------*/

template<typename F>
auto timeit(F&& fn)
{
    auto t0 = Clock::now();
    auto res = fn();
    auto t1 = Clock::now();
    return std::pair{ std::move(res), std::chrono::duration<double>(t1 - t0).count() };
}


/*---------------------------------------------------------------*
 |  1. проверка (adjacency-list  +  vector<int>)                 |
 *---------------------------------------------------------------*/
inline bool isProperColoring(
        const std::vector<std::vector<int>>& adj,
        const std::vector<int>& color,
        bool oneBased = true)               // true → цвета 1,2,… ; false → 0,1,…
{
    const int n = static_cast<int>(adj.size());
    if ((int)color.size() != n) return false;

    for (int v = 0; v < n; ++v)
        for (int u : adj[v])
            if (u > v && color[u] == color[v]) {      // проверяем каждое ребро 1 раз
                std::cerr << "Conflict: ("<<v<<","<<u<<") both color "
                          << color[v] << "\n";
                return false;
            }

    /* необязательно, но можно проверить, что цвета ≥ стартового значения */
    if (oneBased)
        for (int c : color) if (c < 1) return false;

    return true;
}

/*---------------------------------------------------------------*
 |  2. проверка (0/1-матрица  +  vector<int>)                    |
 *---------------------------------------------------------------*/
template<class Matrix>
auto isProperColoring(
        const Matrix& M,
        const std::vector<int>& color,
        bool oneBased = true)
        -> decltype(M.rows(), true)
{
    const int n = M.rows();
    if ((int)color.size() != n) return false;

    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (M(i,j) && color[i] == color[j]) {
                std::cerr << "Conflict: ("<<i<<","<<j<<") both color "
                          << color[i] << "\n";
                return false;
            }
    if (oneBased)
        for (int c : color) if (c < 1) return false;
    return true;
}

/*---------------------------------------------------------------*
 |  3. обёртки для greedy::Coloring<Matrix> и vector<vector<int>>|
 *---------------------------------------------------------------*/
template<class Matrix>
bool isProperColoring(const Matrix& M,
                      const greedy::Coloring<Matrix>& solver)
{
    /* формируем массив цветов 0-based */
    std::vector<int> col(M.rows(), 0);
    for (auto& [v,c] : solver.colors()) col[v] = c;   // цвета 1-based
    return isProperColoring(M, col, true);
}

inline bool isProperColoring(const std::vector<std::vector<int>>& adj,
                             const std::vector<std::vector<int>>& groups)
{
    /* раскладка groups → vector<int> (1-based) */
    std::vector<int> col(adj.size(), 0);
    for (size_t c = 0; c < groups.size(); ++c)
        for (int v : groups[c]) col[v] = static_cast<int>(c) + 1;
    return isProperColoring(adj, col, true);
}
