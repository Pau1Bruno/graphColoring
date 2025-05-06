#include "Benchmark.h"

int main() {
    int                             n = 13;                   // number of vertices
    std::vector<double>             densities = {0.15,0.25,0.35, 0.45};
    int                             perDensity = 5;          // matrices per density

    runBenchmarks(n, densities, perDensity);

    return 0;
}