#include "Graph.h"
#include <algorithm>
#include <stack>

Graph::Graph(const Matrix& adj)
  : n_(static_cast<int>(adj.rows()))
  , adj_(adj)
{
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
        for (int j = 0; j < n_; ++j)
            if (adj_(i, j) != 0) ++d;
        maxD = std::max(maxD, d);
    }
    return maxD;
}

std::vector<int> Graph::deepSearch(int root) const {
    assert(root >= 0 && root < n_);
    std::vector<bool> seen(n_, false);
    std::vector<int>  order;
    std::stack<int>   st;
    st.push(root);
    seen[root] = true;

    while (!st.empty()) {
        int u = st.top(); st.pop();
        order.push_back(u);
        for (int v = 0; v < n_; ++v) {
            if (adj_(u, v) != 0 && !seen[v]) {
                seen[v] = true;
                st.push(v);
            }
        }
    }
    std::sort(order.begin(), order.end());
    return order;
}

std::vector<Graph> Graph::components() const {
    std::vector<bool> seen(n_, false);
    std::vector<Graph> out;
    for (int i = 0; i < n_; ++i) {
        if (!seen[i]) {
            auto comp = deepSearch(i);
            for (int v : comp) seen[v] = true;
            int k = (int)comp.size();
            Matrix sub(k, k);
            for (int a = 0; a < k; ++a)
                for (int b = 0; b < k; ++b)
                    sub(a, b) = adj_(comp[a], comp[b]);
            out.emplace_back(sub);
        }
    }
    return out;
}

Graph Graph::removeVertex(int v) const {
    assert(v >= 0 && v < n_);
    int m = n_ - 1;
    Matrix sub(m, m);
    // map old→new index: skip v
    for (int i = 0, ii = 0; i < n_; ++i) {
        if (i == v) continue;
        for (int j = 0, jj = 0; j < n_; ++j) {
            if (j == v) continue;
            sub(ii, jj) = adj_(i, j);
            ++jj;
        }
        ++ii;
    }
    return Graph(sub);
}

Graph Graph::removeVertices(const std::vector<int>& verts) const {
    // Build a mask of which to keep
    std::vector<bool> keep(n_, true);
    for (int x : verts) {
        assert(x >= 0 && x < n_);
        keep[x] = false;
    }
    // Count kept
    int m = std::count(keep.begin(), keep.end(), true);
    Matrix sub(m, m);
    for (int i = 0, ii = 0; i < n_; ++i) {
        if (!keep[i]) continue;
        for (int j = 0, jj = 0; j < n_; ++j) {
            if (!keep[j]) continue;
            sub(ii, jj) = adj_(i, j);
            ++jj;
        }
        ++ii;
    }
    return Graph(sub);
}

bool Graph::isEdge(int a, int b) const {
    assert(a >= 0 && a < n_ && b >= 0 && b < n_);
    return adj_(a, b) != 0;
}

void Graph::removeEdge(int a, int b) {
    assert(a >= 0 && a < n_ && b >= 0 && b < n_);
    adj_(a, b) = adj_(b, a) = 0;
}

void Graph::complement() {
    for (int i = 0; i < n_; ++i) {
        for (int j = i+1; j < n_; ++j) {
            bool e = adj_(i, j) != 0;
            adj_(i, j) = adj_(j, i) = !e;
        }
    }
}

Graph Graph::graphUnion(const Graph& other) const {
    assert(other.n_ == n_);
    Matrix m = adj_.cwiseMax(other.adj_);  // OR for ints
    return Graph(m);
}

Graph Graph::graphIntersect(const Graph& other) const {
    assert(other.n_ == n_);
    Matrix m(n_, n_);
    for (int i = 0; i < n_; ++i)
        for (int j = 0; j < n_; ++j)
            m(i,j) = (adj_(i,j) != 0 && other.adj_(i,j) != 0) ? 1 : 0;
    return Graph(m);
}

std::vector<int> Graph::neighbors(int v) const {
    assert(v >= 0 && v < n_);
    std::vector<int> out;
    for (int u = 0; u < n_; ++u)
        if (adj_(v, u) != 0)
            out.push_back(u);
    return out;
}

bool Graph::operator==(const Graph& other) const {
    return n_ == other.n_ && adj_ == other.adj_;
}