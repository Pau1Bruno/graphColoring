#pragma once

#include <Eigen/Dense>
#include <vector>
#include <random>
#include <stdexcept>

class GeneticColoring {
public:
    using Coloring = std::vector<int>;

    // popSize: GA population size
    // maxColors: Δ(G)+1, the range of initial/random colors
    // maxGen:   max number of generations
    // mutRate: per-gene mutation probability in [0..1]
    GeneticColoring(int popSize,
                    int maxColors,
                    int maxGen,
                    double mutRate);

    // Solve on a dense adjacency matrix (square, 0/1 entries).
    // Returns a vector of size n with colors in [1..maxColors].
    Coloring solveColoring(const Eigen::MatrixXd& adjMatrix);

private:
    int vertexCount;
    int populationSize;
    int maxColors;
    int maxGenerations;
    double mutationRate;

    std::mt19937 rng;    // must be non-const so RNG calls compile

    using Population = std::vector<Coloring>;

    // These methods are non-const so they can use rng
    Population     initialPopulation();
    Coloring       crossover(const Coloring& p1, const Coloring& p2);
    void           mutate(Coloring& indiv);
    int            evaluate(const Eigen::MatrixXd& adj,
                             const Coloring& indiv);
};