#ifndef OLEMSKOY_GRAPH_H
#define OLEMSKOY_GRAPH_H

#include <Eigen/Dense>
#include <iostream>
#include <vector>

#include "Utils.h"

using DenseMatrix = Eigen::Matrix<int,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor>;

class Graph {
private:
    int n;
    std::vector<std::vector<bool>> adj;
    std::vector<std::vector<int>>  Hsets_, Vsets_;
public:
    // Construct Graph from a symmetric [0-1] matrix (Eigen)
    Graph(const DenseMatrix& matrix) : n(matrix.rows()),
    adj(n, std::vector<bool>(n)),
    Hsets_(n),        
    Vsets_(n) {
        n = matrix.rows();
        adj.assign(n, std::vector<bool>(n));

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                bool edge = (matrix(i, j) != 0);
                // Treat nonzero as adjacent (assuming 1 for edges, 0 for no edge)
                // Ensure diagonal is treated as no self-loop (0) for safety
                adj[i][j] = edge;

                if (!edge) {                 
                    Hsets_[i].push_back(j + 1);
                    Vsets_[j].push_back(i + 1);
                }
            }
        }

        std::cout << Hsets_;
    }

    // Number of vertices
    int size() const {
        return n;
    }

    // Check if two vertices are adjacent (matrix entry == 1)
    bool areAdjacent(int i, int j) const {
        return adj[i][j];
    }

    const std::vector<std::vector<bool>>& adjacency() const { return adj; }
    const std::vector<std::vector<int>>& Hsets() const { return Hsets_; }
    const std::vector<std::vector<int>>& Vsets() const { return Vsets_; }
};

#endif // OLEMSKOY_GRAPH_H
