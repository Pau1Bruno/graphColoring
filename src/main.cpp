#include "Benchmark.h"

int main() {
    int                             n = 5;                   // number of vertices
    std::vector<double>             densities = {0.1,0.3,0.5};
    int                             perDensity = 3;          // matrices per density
    int                             gaPopulation = 100;
    int                             gaMaxGen    = 500;
    double                          gaMutation  = 0.01;

    runBenchmarks(n, densities, perDensity,
                  gaPopulation, gaMaxGen, gaMutation);

    return 0;
}