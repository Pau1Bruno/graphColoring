#include "Benchmark.h"

int main() {
    int                             n = 12;                   // number of vertices
    std::vector<double>             densities = {0.45};
    int                             perDensity = 1;          // matrices per density

    runBenchmarks(n, densities, perDensity);
    return 0;
}