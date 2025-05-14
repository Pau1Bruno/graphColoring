// Benchmark.h
#pragma once

#include "algorithms/GreedyColoring.h"
#include "algorithms/DSaturColoring.h"
#include "algorithms/GreedyHeuristicsColoring.h"
#include "algorithms/DSaturBnB.h"

#include "method/Graph.h"
#include "method/OlemskoyColorGraph.h"

#include "MatrixIO.h"
#include "BenchmarkUtils.h"

#include <Eigen/Dense>
#include <random>
#include <chrono>
#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <string>

// –– Run all three algorithms on a given dense matrix
static void runOnDense(const DenseMatrix &M,
                       double density,
                       int idx,
                       const std::string &tag)
{
    std::cout << tag
              << " | density=" << density
              << " | mat#=" << idx << "\n"
              << "Adjacency matrix:\n";
            //   << M << "\n\n";

    // 1) Greedy (vector<int>)
    auto [greedSolVec, tG] = timeit([&]{ return GreedyColoring::color(M); });
    std::cout << "Жадный метод: \n";
    printColoring(greedSolVec);
    std::cout << "Время: " << tG << " c\n\n";
    std::cout << (isProperColoring(M, greedSolVec, false) ? "✔ корректно\n\n"
                                                      : "✖ конфликт!\n\n");

    // 2) DSATUR (vector<int>)
    auto [dsatSolVec, tD] = timeit([&]{ return DSaturColoring::color(M); });
    std::cout << "DSatur метод: \n";
    printColoring(dsatSolVec);
    std::cout << "Время: " << tD << " c\n\n";
    std::cout << (isProperColoring(M, dsatSolVec, false) ? "✔ корректно\n\n"
                                                     : "✖ конфликт!\n\n");

    // 3) Расширенная эвристика (greedy::Coloring)
    auto [greedHObj, tGH] = timeit([&]{ return greedy::Coloring{M}; });
    std::cout << "Жадный метод с эвристиками: \n";
    printColoring(greedHObj);
    std::cout << "Время: " << tGH << " c\n\n";
    std::cout << (isProperColoring(M, greedHObj) ? "✔ корректно\n\n"
                                             : "✖ конфликт!\n\n");

    // // 4) DSATUR-BnB — гарантировано минимальное χ
    // auto [bnbColorVec, tDBnB] = timeit([&]{ return DSaturBnB::color(M); });
    // std::cout << "DSATUR-BnB (точный):\n";
    // printColoring(bnbColorVec);
    // std::cout << "Время: " << tDBnB << " c\n\n";
    // std::cout << (isProperColoring(M, bnbColorVec, false) ? "✔ корректно\n\n"
    //                                                  : "✖ конфликт!\n\n");
    // 5) Метод Олемского
    // auto [olemSol, tO] = timeit([&]{ 
    //     Graph G(M);
    //     OlemskoyColorGraph ocg(G);
    //     return ocg.resultColorNodes();
    // });
    // printColoring(olemSol);
    // std::cout << "Время: " << tO << " c\n\n";
    // std::cout << (isProperColoring(M, olemSol, false) ? "✔ корректно\n\n"
    //                                                  : "✖ конфликт!\n\n");
}



  

// Запуск алгоритмов по начальным данным
inline void runBenchmarks(int n,
                          const std::vector<double> &densities,
                          int perDensity)
{
    auto graphs = loadGraphs("graphs.txt");
    for (size_t idx=0; idx<graphs.size(); ++idx)
    {
        const auto& G = graphs[idx];
        writeMatrix("start_matrix_" + std::to_string(idx+1) + ".txt", G.A);
        std::cout << "=== Graph #" << idx+1
                  << "  (n="<<G.n<<", d≈"<<G.density<<") ===\n";
        runOnDense(G.A, G.density, idx, "Dense");
    }
    // DenseMatrix TEST_MATRIX {
    //     {0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
    //     {0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    //     {0, 1, 0, 1, 1, 1, 0, 0, 0, 0},
    //     {1, 0, 1, 0, 0, 0, 1, 1, 0, 0},
    //     {0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    //     {1, 0, 1, 0, 0, 0, 0, 1, 1, 0},
    //     {0, 1, 0, 0, 1, 1, 0, 0, 0, 1},
    //     {0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
    //     {0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
    //     {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    // };
    // runOnDense(TEST_MATRIX, 1, 1, "Dense");


    // for (double d : densities)
    // {
    //     auto denseList = generateDenseMatrices(n, d, perDensity);
    //     for (int i = 0; i < perDensity; ++i)
    //     {
    //         runOnDense(denseList[i], d, i, "Dense");
    //     }
    // }
}
