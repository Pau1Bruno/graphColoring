#pragma once

#include <Eigen/Dense>
#include <vector>

class GreedyColoring {
public:
    // Use a dense integer adjacency matrix
    using MatrixType = Eigen::MatrixXi;

    // Returns a vector of size n (number of vertices),
    // where each entry is the color index [0..].
    static std::vector<int> color(const MatrixType& adj);
};