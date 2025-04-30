#pragma once

#include "Pair.h"
#include <Eigen/Dense>
#include <vector>
#include <algorithm>

// Handles a collection of non-edge Pairs and operations to filter/prune them.
class Variants {
public:
    std::vector<Pair> setOfPairs;

    Variants() = default;
    Variants(const Variants&) = default;

    // Create initial variants from adjacency matrix 'mat' and support set 'w'.
    // Pairs correspond to non-edges (i<j where mat(i,j)==0).
    static Variants createFromMatrix(const Eigen::MatrixXi& mat,
                                     const std::vector<int>& w);

    // Sieve: remove the specific (left,right) pair and prune other supports by removing left/right.
    Variants sieve(int left, int right) const;

    // Sieve by support set: keep only pairs whose endpoints are in supSet,
    // and intersect their support with supSet.
    Variants sieve(const std::vector<int>& supSet) const;

    // Sift: remove pairs whose support set is entirely contained in supSet.
    void sift(const std::vector<int>& supSet);
};