#pragma once

#include "DSet.h"
#include <Eigen/Dense>
#include <vector>

// Handles a collection of non-edge Pairs and operations to filter/prune them.
class Variants
{
public:
    std::vector<DSet> setOfPairs;

    // Generate all D-sets from adjacency matrix 'mat' and support set 'omega'.
    // Each DSet corresponds to a non-edge (i<j) with its support vertices.
    static Variants createFromMatrix(
        const Eigen::MatrixXi &mat,
        const std::vector<int> &omega);

    // Sieve: remove the specific (left,right) pair, prune other supports by removing left/right.
    Variants sieve(int left, int right) const;

    // Sieve by support set: keep only pairs whose endpoints are in supSet,
    // and intersect their support with supSet.
    Variants sieve(const std::vector<int> &supSet) const;

    // Sift: remove pairs whose support set is entirely contained in supSet.
    void sift(const std::vector<int> &supSet);
};