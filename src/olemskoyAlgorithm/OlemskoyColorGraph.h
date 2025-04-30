#pragma once

#include <vector>
#include <Eigen/Dense>
#include "Variants.h"

// Colors result as vector of color-classes (vertex indices)
class OlemskoyColorGraph {
public:
    using Matrix = Eigen::MatrixXi;
    using Coloring = std::vector<std::vector<int>>;

    // Build and solve: input adjacency (int matrix, non-zero means edge)
    explicit OlemskoyColorGraph(const Matrix& adj);

    // Retrieve result color-classes
    const Coloring& resultColorNodes() const { return bestColors_; }

private:
    int N_;                 // number of vertices
    Matrix graph_;          // adjacency matrix
    Coloring bestColors_;   // best coloring found

    // Working state during backtracking
    std::vector<std::vector<int>> tmpColors_;
    std::vector<std::vector<int>> watchedFirstBlocks_;
    std::vector<Variants>         lvlVariants_;
    std::vector<int>              mainSupSet_;
    int                            maxColors_;
    bool                           isOver_;

    // Initialize working state
    void init();
    // Finalize bestColors_
    void initResult();

    // Core recursive builder
    void build(const std::vector<int>& uni,
               std::vector<int>&       curTempSet);

    // Helper checks
    bool blockCheckA(int lenMax, int uniLen) const;
    bool blockCheckB(int curLvl, int ro) const;
    bool blockCheckC(int curLvl, int uniLen, int ro) const;
    bool blockCheckD(int ro) const;
    int  getRo(const Variants& vs, const Pair& p) const;

    // Branch routines
    void buildNotNullVariants(const Variants& variants,
                              const std::vector<int>& uni,
                              std::vector<int>& curTempSet);
    void buildEndByCenter(const std::vector<int>& uni,
                          std::vector<int>&       curTempSet);

    // Prune and thinning
    void thinning();
    bool coloringIsOver() const;

    // Manage variants
    void createVariants(const std::vector<int>& uni, bool newColor);
};