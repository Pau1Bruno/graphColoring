// src/Benchmark.cpp

#include "algorithms/GreedyColoring.h"
#include "algorithms/DSaturColoring.h"
#include "algorithms/GeneticColoring.h"
// #include "algorithms/GeneticColoring.h"

#include <Eigen/Dense>
// #include <Eigen/Sparse>

#include <random>
#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

using DenseMatrix  = Eigen::MatrixXi;
// using SparseMatrix = Eigen::SparseMatrix<int, Eigen::RowMajor>;
using Clock        = std::chrono::high_resolution_clock;

//–– Utility: generate 'count' symmetric dense matrices of size n×n with given density
std::vector<DenseMatrix>
generateDenseMatrices(int n, double density, int count) {
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::vector<DenseMatrix> out;
    out.reserve(count);

    for (int m = 0; m < count; ++m) {
        DenseMatrix A = DenseMatrix::Zero(n,n);
        for (int i = 0; i < n; ++i) {
            for (int j = i+1; j < n; ++j) {
                if (dist(rng) < density) {
                    A(i,j) = A(j,i) = 1;
                }
            }
        }
        out.push_back(std::move(A));
    }
    return out;
}

//–– Compute maximum degree of a graph, works for both dense & sparse
template<typename Mat>
int computeMaxDegree(const Mat& A) {
    int n = A.rows(), maxDeg = 0;
    for (int i = 0; i < n; ++i) {
        int deg = 0;
        if constexpr (std::is_same_v<Mat, DenseMatrix>) {
            for (int j = 0; j < n; ++j)
                if (A(i,j) != 0) ++deg;
        } else {
            for (typename Mat::InnerIterator it(A,i); it; ++it)
                ++deg;
        }
        maxDeg = std::max(maxDeg, deg);
    }
    return maxDeg;
}

//–– Run all three algorithms on a given dense matrix (whether originally dense or converted)
void runOnDense(const DenseMatrix& M,
                double density,
                int idx,
                const std::string& tag)
{
    std::cout << tag
    << " | density=" << density
    << " | mat#="   << idx   << "\n"
    << "Adjacency matrix:\n"
    << M << "\n\n";

    // 1) Determine k = Δ(G) + 1 for GA
    int k = computeMaxDegree(M) + 1;

    // Helper to time a function and return (result, elapsed_seconds)
    auto timeit = [&](auto&& fn) {
        auto t0 = Clock::now();
        auto result = fn();
        auto t1 = Clock::now();
        return std::pair{ std::move(result),
            std::chrono::duration<double>(t1-t0).count() };
    };

    // Greedy
    auto [greedSol, tG] = timeit([&]{ return GreedyColoring::color(M); });
    int colorsG = *std::max_element(greedSol.begin(), greedSol.end()) + 1;

    // DSATUR
    auto [dsatSol, tD] = timeit([&]{ return DSaturColoring::color(M); });
    int colorsD = *std::max_element(dsatSol.begin(), dsatSol.end()) + 1;

    // Print summary
    std::cout
      << tag
      << " | density="<< density
      << " | mat#="<< idx << "\n"
      << "  Greedy : colors="<<colorsG<<"  time="<<tG<<" s\n"
      << "  DSATUR : colors="<<colorsD<<"  time="<<tD<<" s\n";
}

//–– Master routine: generates, runs, and reports
void runBenchmarks(int n,
                   const std::vector<double>& densities,
                   int perDensity)
{
    for (double d : densities) {
        // generate 3 dense + 3 sparse
        auto denseList  = generateDenseMatrices(n, d, perDensity);
        // auto sparseList = generateSparseMatrices(n, d, perDensity);

        for (int i = 0; i < perDensity; ++i) {
            // run on the dense copy
            runOnDense(denseList[i], d, i,  "Dense");
            // convert sparse→dense for the same algorithms
            // runOnDense(sparseList[i].toDense(),
            //            d, i, "Sparse);
        }
    }
}
