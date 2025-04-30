#pragma once

#include <Eigen/Dense>
#include <vector>
#include <cassert>

// Graph wraps an integer adjacency matrix (non-zero ⇒ edge).
class Graph {
public:
    using Matrix = Eigen::MatrixXi;

    // Construct from a square adjacency matrix
    explicit Graph(const Matrix& adj);

    // Number of vertices
    int numVertices() const;

    // Access adjacency
    const Matrix& adjacency() const;

    // Maximum degree Δ(G)
    int maxDegree() const;

    // Depth-first search from root (0 ≤ root < n), returns sorted vertex list
    std::vector<int> deepSearch(int root) const;

    // Connected components as subgraphs
    std::vector<Graph> components() const;

    // Remove a single vertex (by index), returning the induced subgraph
    Graph removeVertex(int v) const;

    // Remove multiple vertices (by indices), returning the induced subgraph
    Graph removeVertices(const std::vector<int>& verts) const;

    // Edge query
    bool isEdge(int a, int b) const;

    // Delete an edge
    void removeEdge(int a, int b);

    // Complement the graph (invert all off-diagonal entries)
    void complement();

    // Union (edge-wise OR) with another graph of equal size
    Graph graphUnion(const Graph& other) const;

    // Intersection (edge-wise AND) with another graph of equal size
    Graph graphIntersect(const Graph& other) const;

    // List neighbors of v
    std::vector<int> neighbors(int v) const;

    // Equality test
    bool operator==(const Graph& other) const;

private:
    int     n_;   // number of vertices
    Matrix  adj_; // adjacency matrix (n×n)
};