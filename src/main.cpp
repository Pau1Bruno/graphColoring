#include "Benchmark.h"

int main() {
    int                             n = 14;                   // number of vertices
    std::vector<double>             densities = {0.25, 0.35};
    int                             perDensity = 1;          // matrices per density

    runBenchmarks(n, densities, perDensity);
    return 0;
}