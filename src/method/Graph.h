#ifndef OLEMSKOY_GRAPH_H
#define OLEMSKOY_GRAPH_H

#include <Eigen/Dense>
#include <vector>

class Graph {
private:
    int n;
    std::vector<std::vector<bool>> adj;
public:
    // Construct Graph from a symmetric [0-1] matrix (Eigen)
    Graph(const Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic>& matrix) {
        n = matrix.rows();
        adj.assign(n, std::vector<bool>(n));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                // Treat nonzero as adjacent (assuming 1 for edges, 0 for no edge)
                // Ensure diagonal is treated as no self-loop (0) for safety
                adj[i][j] = (matrix(i, j) != 0);
            }
        }
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
};

#endif // OLEMSKOY_GRAPH_H
