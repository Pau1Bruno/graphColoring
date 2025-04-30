#include "Graph.h"
#include <stack>
#include <algorithm>

// Construct from adjacency matrix; matrix must be square
Graph::Graph(const Matrix& adj)
    : n_(static_cast<int>(adj.rows())), adj_(adj) {
    assert(adj.rows() == adj.cols() && "Adjacency matrix must be square");
}

int Graph::numVertices() const {
    return n_;
}

const Graph::Matrix& Graph::adjacency() const {
    return adj_;
}

int Graph::maxDegree() const {
    int maxD = 0;
    for (int i = 0; i < n_; ++i) {
        int d = 0;
        for (int j = 0; j < n_; ++j) {
            if (adj_(i, j) != 0) ++d;
        }
        maxD = std::max(maxD, d);
    }
    return maxD;
}

std::vector<int> Graph::deepSearch(int root) const {
    std::vector<bool> visited(n_, false);
    std::vector<int> result;
    std::stack<int> st;
    st.push(root);
    visited[root] = true;

    while (!st.empty()) {
        int u = st.top(); st.pop();
        result.push_back(u);
        for (int v = 0; v < n_; ++v) {
            if (adj_(u, v) != 0 && !visited[v]) {
                visited[v] = true;
                st.push(v);
            }
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::vector<Graph> Graph::components() const {
    std::vector<bool> seen(n_, false);
    std::vector<Graph> comps;

    for (int i = 0; i < n_; ++i) {
        if (!seen[i]) {
            auto verts = deepSearch(i);
            for (int v : verts) seen[v] = true;

            int k = static_cast<int>(verts.size());
            Matrix sub(k, k);
            for (int a = 0; a < k; ++a) {
                for (int b = 0; b < k; ++b) {
                    sub(a, b) = adj_(verts[a], verts[b]);
                }
            }
            comps.emplace_back(sub);
        }
    }
    return comps;
}
