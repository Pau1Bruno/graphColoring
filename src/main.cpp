#include "Benchmark.h"

int main() {
    int                             n = 5000;                   // number of vertices
    std::vector<double>             densities = {0.15, 0.35, 0.55, 0.65, 0.75, 0.85};
    int                             perDensity = 2;          // matrices per density

    runBenchmarks(n, densities, perDensity);
    return 0;
}