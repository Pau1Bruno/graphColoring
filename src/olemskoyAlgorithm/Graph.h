#pragma once

#include <Eigen/Dense>
#include <vector>
#include <cassert>

// Graph wraps an integer adjacency matrix (non-zero => edge)
class Graph {
public:
    using Matrix = Eigen::MatrixXi;

    // Construct from a square adjacency matrix
    explicit Graph(const Matrix& adj);

    // Number of vertices
    int numVertices() const;

    // Access adjacency matrix
    const Matrix& adjacency() const;

    // Compute the maximum degree Δ(G)
    int maxDegree() const;

    // Depth-first search from 'root', returns sorted list of visited vertices
    std::vector<int> deepSearch(int root) const;

    // Extract connected components as subgraphs
    std::vector<Graph> components() const;

private:
    int     n_;   // number of vertices
    Matrix  adj_; // adjacency matrix (n x n)
};
