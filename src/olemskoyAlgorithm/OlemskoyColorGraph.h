#pragma once

#include <vector>
#include <Eigen/Dense>
#include "Variants.h"

class OlemskoyColorGraph {
public:
    using Matrix   = Eigen::MatrixXi;
    using Coloring = std::vector<std::vector<int>>;

    // Constructor: takes adjacency matrix (n x n int, non-zero=edge)
    explicit OlemskoyColorGraph(const Matrix& adj);

    // Results after construction:
    // - resultColorNodes(): color-classes (each vector<int> is a color set)
    // - resultNumColors(): number of colors used
    Coloring resultColorNodes() const { return bestColors_; }
    int resultNumColors()    const { return static_cast<int>(bestColors_.size()); }

private:
    int    N_;                // number of vertices (sqrt of input length)
    Matrix graph_;            // adjacency matrix
    Coloring bestColors_;     // best coloring found

    // Working state
    std::vector<std::vector<int>> tmpColors_;           // current partial coloring (list of color-classes)
    std::vector<std::vector<int>> watchedFirstBlocks_;  // for first-block pruning
    std::vector<Variants>         lvlVariants_;         // support-set variants per level
    std::vector<int>              mainSupSet_;          // initial support set [0..N-1]
    int                            maxColors_;          // current upper bound on #colors
    bool                           isOver_;             // termination flag

    // Initialize/reset working state
    void init();
    // After build(), copy bestColors_ to results
    void initResult();

    // Create initial support set [0..N-1]
    std::vector<int> createStartSupport() const;
    // Recompute support = mainSupSet \ union of tmpColors_
    std::vector<int> createSupportSet() const;

    // Block checks correspond to C# BlockCheckA/B/C/D
    bool blockCheckA(int lenMax, int uniLen) const;
    bool blockCheckB(int curLvl, int ro) const;
    bool blockCheckC(int curLvl, int uniLen, int ro) const;
    bool blockCheckD(int ro) const;

    // Ro = size of support for a given Pair (at least 1)
    int getRo(const Variants& vs, const Pair& p) const;

    // Recursive branching:
    void build(const std::vector<int>& uni,
               std::vector<int>&       curTempSet);

    // Case when variants non-empty
    void buildNotNullVariants(const Variants& variants,
                              const std::vector<int>& uni,
                              std::vector<int>& curTempSet);
    // Case when no variants, but uni non-empty
    void buildEndByCenter(const std::vector<int>& uni,
                          std::vector<int>&       curTempSet);

    // Thinning (Phi) and pruning
    std::vector<int> getPhi(int s) const;
    void thinning();
    bool coloringIsOver() const;

    // First-block check (avoid duplicate blocks)
    bool checkFirstBlock(const std::vector<int>& block) const;

    // Create or sieve variants: C# CreateVariants
    void createVariants(const std::vector<int>& uni, bool newColor);
};