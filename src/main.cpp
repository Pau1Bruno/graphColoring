#include "Benchmark.h"

int main() {
    int                             n = 5;                   // number of vertices
    std::vector<double>             densities = {0.4,0.6,0.8};
    int                             perDensity = 3;          // matrices per density

    runBenchmarks(n, densities, perDensity);

    return 0;
}