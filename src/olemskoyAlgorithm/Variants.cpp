#include "Variants.h"
#include "Utils.h"
#include <algorithm>
#include <iostream>

static bool contains(const std::vector<int> &vec, int x) {
    return std::find(vec.begin(), vec.end(), x) != vec.end();
}

Variants Variants::createFromMatrix(const Eigen::MatrixXi &mat, const std::vector<int> &omega) {
    int n = mat.rows();

    std::vector<std::vector<int>> hor(n), ver(n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (mat(i, j) == 0) {
                hor[i].push_back(j + 1);
                ver[j].push_back(i + 1);
            }

    std::cout << "[Variants] Horizontal sets H:\n";
    for (int i = 0; i < n; ++i) {
        std::cout << "  H" << (i + 1) << " = { ";
        for (int x : hor[i]) std::cout << x << " ";
        std::cout << "}\n";
    }

    std::cout << "[Variants] Vertical sets V:\n";
    for (int j = 0; j < n; ++j) {
        std::cout << "  V" << (j + 1) << " = { ";
        for (int x : ver[j]) std::cout << x << " ";
        std::cout << "}\n";
    }

    std::vector<DSet> dsets;
    dsets.reserve(n * n);

    for (int q = 0; q < n; ++q) {
        for (int r = q + 1; r < n; ++r) {
            int Q = q + 1, R = r + 1;
            if (!contains(omega, Q) || !contains(omega, R)) continue;

            std::vector<int> d_qr, d_rq;
            for (int el : hor[q])
                if (contains(ver[r], el) && contains(omega, el)) d_qr.push_back(el);

            for (int el : hor[r])
                if (contains(ver[q], el) && contains(omega, el)) d_rq.push_back(el);

            std::vector<int> Dqr;
            for (int el : d_qr)
                if (contains(d_rq, el)) Dqr.push_back(el);

            if (!contains(Dqr, Q) || !contains(Dqr, R)) continue;

            dsets.push_back({Q, R, std::move(Dqr)});
        }
    }

    std::sort(dsets.begin(), dsets.end(), [](const DSet &a, const DSet &b) {
        if (a.set.size() != b.set.size()) return a.set.size() > b.set.size();
        if (a.i != b.i) return a.i < b.i;
        return a.j < b.j;
    });

    std::cout << dsets; // Util function to print

    return Variants{dsets};
}

Variants Variants::sieve(int i, int j) const {
    Variants out;
    for (auto dSet : setOfPairs) {
        if (dSet.i == i && dSet.j == j) continue;
        dSet.set.erase(std::remove(dSet.set.begin(), dSet.set.end(), i), dSet.set.end());
        dSet.set.erase(std::remove(dSet.set.begin(), dSet.set.end(), j), dSet.set.end());
        out.setOfPairs.push_back(std::move(dSet));
    }
    return out;
}

Variants Variants::sieve(const std::vector<int> &supSet) const {
    Variants out;
    for (auto dSet : setOfPairs) {
        if (!contains(supSet, dSet.i) || !contains(supSet, dSet.j)) continue;
        std::vector<int> newSup;
        for (int x : dSet.set)
            if (contains(supSet, x)) newSup.push_back(x);
        dSet.set = std::move(newSup);
        out.setOfPairs.push_back(std::move(dSet));
    }
    return out;
}

void Variants::sift(const std::vector<int> &supSet) {
    auto it = setOfPairs.begin();
    while (it != setOfPairs.end()) {
        bool allContained = std::all_of(it->set.begin(), it->set.end(), [&](int x) {
            return contains(supSet, x);
        });
        if (allContained)
            it = setOfPairs.erase(it);
        else
            ++it;
    }
}
