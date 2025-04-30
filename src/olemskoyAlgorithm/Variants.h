#pragma once

#include "Pair.h"
#include <Eigen/Dense>
#include <vector>

// Encapsulates a collection of Pair (left,right) with associated support sets
class Variants {
public:
    std::vector<Pair> setOfPairs;

    Variants() = default;
    Variants(const Variants&) = default;

    // Create initial variants from adjacency matrix 'mat' and support set 'w'
    // Non-edges (i<j where mat(i,j)==0) become Pairs
    static Variants createFromMatrix(const Eigen::MatrixXi& mat,
                                     const std::vector<int>& w);

    // Sieve: keep only pairs involving (left,right), remove others but prune their sup by removing left/right
    Variants sieve(int left, int right) const;

    // Sieve by support set: remove pairs whose endpoints not both in supSet; otherwise intersect their sup
    Variants sieve(const std::vector<int>& supSet) const;

    // Remove pairs whose sup is disjoint from supSet
    void sift(const std::vector<int>& supSet);
};

