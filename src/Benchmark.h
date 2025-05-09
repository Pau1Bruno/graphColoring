// Benchmark.h
#pragma once

#include "algorithms/GreedyColoring.h"
#include "algorithms/DSaturColoring.h"

#include "method/Graph.h"
#include "method/OlemskoyColorGraph.h"

#include <Eigen/Dense>
#include <random>
#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

using DenseMatrix = Eigen::Matrix<int,Eigen::Dynamic,Eigen::Dynamic>;
using Clock = std::chrono::high_resolution_clock;

// –– Utility: generate 'count' symmetric dense matrices of size n×n with given density
static std::vector<DenseMatrix>
generateDenseMatrices(int n, double density, int count)
{
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::vector<DenseMatrix> out;
    out.reserve(count);

    for (int m = 0; m < count; ++m)
    {
        DenseMatrix A = DenseMatrix::Zero(n, n);
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                if (dist(rng) < density)
                {
                    A(i, j) = A(j, i) = 1;
                }
            }
        }
        out.push_back(std::move(A));
    }
    return out;
}

// –– Compute maximum degree Δ(G)
template <typename Mat>
static int computeMaxDegree(const Mat &A)
{
    int n = A.rows(), maxDeg = 0;
    for (int i = 0; i < n; ++i)
    {
        int deg = 0;
        if constexpr (std::is_same_v<Mat, DenseMatrix>)
        {
            for (int j = 0; j < n; ++j)
                if (A(i, j) != 0)
                    ++deg;
        }
        else
        {
            for (typename Mat::InnerIterator it(A, i); it; ++it)
                ++deg;
        }
        maxDeg = std::max(maxDeg, deg);
    }
    return maxDeg;
}

// –– Run all three algorithms on a given dense matrix
static void runOnDense(const DenseMatrix &M,
                       double density,
                       int idx,
                       const std::string &tag)
{
    std::cout << tag
              << " | density=" << density
              << " | mat#=" << idx << "\n"
              << "Adjacency matrix:\n"
              << M << "\n\n";

    // Time‐helper
    auto timeit = [&](auto &&fn)
    {
        auto t0 = Clock::now();
        auto res = fn();
        auto t1 = Clock::now();
        return std::pair{std::move(res),
                         std::chrono::duration<double>(t1 - t0).count()};
    };

    auto printFlatColoring = [&](const std::vector<int> &sol, int numColors)
    {
        for (int c = 0; c < numColors; ++c)
        {
            std::cout << "    Color " << c << ":";
            for (int v = 0; v < (int)sol.size(); ++v)
            {
                if (sol[v] == c)
                    std::cout << " " << v;
            }
            std::cout << "\n";
        }
    };

    // 1) Greedy
    auto [greedSol, tG] = timeit([&]
                                 { return GreedyColoring::color(M); });
    int colorsG = *std::max_element(greedSol.begin(), greedSol.end()) + 1;

    // 2) DSATUR
    auto [dsatSol, tD] = timeit([&]
                                { return DSaturColoring::color(M); });
    int colorsD = *std::max_element(dsatSol.begin(), dsatSol.end()) + 1;

    // 3) Olemskoy
    auto [olemSol, tO] = timeit([&]
                                {
                                    Graph G(M);
                                    OlemskoyColorGraph ocg(G);
                                    return ocg.resultColorNodes(); // now returns by value
                                });
    int colorsO = (int)olemSol.size();

    // Print summary header
    std::cout
        << tag << " | density=" << density << " | mat#=" << idx << "\n"
        << "  Greedy  : colors=" << colorsG << "  time=" << tG << " s\n";
    // Print greedy groups
    printFlatColoring(greedSol, colorsG);

    std::cout
        << "  DSATUR  : colors=" << colorsD << "  time=" << tD << " s\n";
    // Print dsatur groups
    printFlatColoring(dsatSol, colorsD);

    std::cout
        << "  Olemskoy: colors=" << colorsO << "  time=" << tO << " s\n";
    // Print olem groups
    for (int c = 0; c < colorsO; ++c)
    {
        std::cout << "    Color " << c << ":";
        for (int v : olemSol[c])
        {
            std::cout << " " << v;
        }
        std::cout << "\n";
    }
}

// –– Master routine: generates, runs, and reports
inline void runBenchmarks(int n,
                          const std::vector<double> &densities,
                          int perDensity)
{
    DenseMatrix TEST_MATRIX {
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
        {0, 1, 0, 1, 1, 1, 0, 0, 0, 0},
        {1, 0, 1, 0, 0, 0, 1, 1, 0, 0},
        {0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
        {1, 0, 1, 0, 0, 0, 0, 1, 1, 0},
        {0, 1, 0, 0, 1, 1, 0, 0, 0, 1},
        {0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    };
    runOnDense(TEST_MATRIX, 1, 1, "Dense");
    // for (double d : densities)
    // {
    //     auto denseList = generateDenseMatrices(n, d, perDensity);
    //     for (int i = 0; i < perDensity; ++i)
    //     {
    //         runOnDense(denseList[i], d, i, "Dense");
    //     }
    // }
}
