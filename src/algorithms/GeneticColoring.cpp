// GeneticColoring.cpp
#include "GeneticColoring.h"
#include <algorithm>
#include <numeric>

GeneticColoring::GeneticColoring(int popSize,
                                 int maxCols,
                                 int maxGen,
                                 double mutRate)
  : vertexCount(0),
    populationSize(popSize),
    maxColors(maxCols),
    maxGenerations(maxGen),
    mutationRate(mutRate),
    rng(std::random_device{}())
{}

GeneticColoring::Coloring
GeneticColoring::solveColoring(const Eigen::MatrixXd& adjMatrix) {
    vertexCount = adjMatrix.rows();
    if (adjMatrix.rows() != adjMatrix.cols())
        throw std::invalid_argument("Adjacency matrix must be square");

    // 1) build initial population
    Population pop = initialPopulation();
    Coloring best = pop[0];
    int bestScore = evaluate(adjMatrix, best);

    // 2) main GA loop
    for (int gen = 0; gen < maxGenerations; ++gen) {
        // 2a) evaluate
        std::vector<int> score(populationSize);
        for (int i = 0; i < populationSize; ++i) {
            score[i] = evaluate(adjMatrix, pop[i]);
            if (score[i] < bestScore) {
                bestScore = score[i];
                best = pop[i];
            }
        }

        // 2b) create next generation (elitism + tournament + crossover + mutate)
        Population next;
        next.reserve(populationSize);
        next.push_back(best);

        std::uniform_int_distribution<int> pick(0, populationSize - 1);
        while ((int)next.size() < populationSize) {
            // tournament select
            int a = pick(rng), b = pick(rng);
            int p1 = (score[a] < score[b] ? a : b);
            a = pick(rng); b = pick(rng);
            int p2 = (score[a] < score[b] ? a : b);

            // crossover + mutate
            Coloring child = crossover(pop[p1], pop[p2]);
            mutate(child);
            next.push_back(std::move(child));
        }
        pop = std::move(next);
    }

    return best;
}

// random initial population
GeneticColoring::Population GeneticColoring::initialPopulation() {
    Population out;
    out.reserve(populationSize);
    std::uniform_int_distribution<int> colorDist(1, maxColors);

    for (int i = 0; i < populationSize; ++i) {
        Coloring sol(vertexCount);
        for (int v = 0; v < vertexCount; ++v)
            sol[v] = colorDist(rng);
        out.push_back(std::move(sol));
    }
    return out;
}

// single‐point crossover
GeneticColoring::Coloring
GeneticColoring::crossover(const Coloring& p1,
                           const Coloring& p2)
{
    std::uniform_int_distribution<int> crossPt(1, vertexCount-1);
    int cp = crossPt(rng);

    Coloring child(vertexCount);
    std::copy(p1.begin(), p1.begin()+cp, child.begin());
    std::copy(p2.begin()+cp, p2.end(),   child.begin()+cp);
    return child;
}

// per‐gene mutation
void GeneticColoring::mutate(Coloring& indiv) {
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    std::uniform_int_distribution<int>     colorDist(1, maxColors);

    for (int i = 0; i < vertexCount; ++i) {
        if (prob(rng) < mutationRate) {
            indiv[i] = colorDist(rng);
        }
    }
}

// score = (# conflicts) + (max color used)
// lower is better
int GeneticColoring::evaluate(const Eigen::MatrixXd& adj,
                              const Coloring& sol)
{
    int conflicts = 0;
    for (int i = 0; i < vertexCount; ++i) {
        for (int j = i+1; j < vertexCount; ++j) {
            if (adj(i,j) != 0 && sol[i] == sol[j])
                ++conflicts;
        }
    }
    int maxCol = *std::max_element(sol.begin(), sol.end());
    return conflicts + maxCol;
}
