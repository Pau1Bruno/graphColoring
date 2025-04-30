#include "Variants.h"
#include "Utils.h"
#include <algorithm>
#include <iostream>

Variants Variants::createFromMatrix(const Eigen::MatrixXi &mat,
                                    const std::vector<int> &w)
{
    int n = static_cast<int>(mat.rows());
    Variants res;
    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            if (mat(i, j) != 0)
                continue; // skip edges, only non‐edges
            Pair p;
            p.left = i;
            p.right = j;
            // build support: vertices k that are non‐adjacent to both i and j and in w
            for (int k : w)
            {
                if (mat(i, k) == 0 && mat(k, j) == 0)
                {
                    p.sup.push_back(k);
                }
            }
            res.setOfPairs.push_back(std::move(p));
        }
    }
    // sort by increasing support size
    std::sort(res.setOfPairs.begin(), res.setOfPairs.end(),
              [](auto &a, auto &b)
              { return a.sup.size() < b.sup.size(); });

    // *** DEBUG LOG ***
    std::cout << "[createFromMatrix] generated Variants: " << res << std::endl;

    return res;
}

Variants Variants::sieve(int left, int right) const
{
    Variants out;
    for (auto p : setOfPairs)
    {
        if (p.left == left && p.right == right)
        {
            // skip this pair entirely
            continue;
        }
        // for other pairs, prune support of left/right
        p.sup.erase(std::remove(p.sup.begin(), p.sup.end(), left), p.sup.end());
        p.sup.erase(std::remove(p.sup.begin(), p.sup.end(), right), p.sup.end());
        out.setOfPairs.push_back(std::move(p));
    }
    return out;
}

Variants Variants::sieve(const std::vector<int> &supSet) const
{
    Variants out;
    for (auto p : setOfPairs)
    {
        // keep only if both endpoints in supSet
        if (std::find(supSet.begin(), supSet.end(), p.left) == supSet.end() ||
            std::find(supSet.begin(), supSet.end(), p.right) == supSet.end())
        {
            continue;
        }
        // intersect p.sup with supSet
        std::vector<int> newSup;
        for (int x : p.sup)
        {
            if (std::find(supSet.begin(), supSet.end(), x) != supSet.end())
                newSup.push_back(x);
        }
        p.sup = std::move(newSup);
        out.setOfPairs.push_back(std::move(p));
    }
    return out;
}

void Variants::sift(const std::vector<int> &supSet)
{
    // Remove pairs whose support set is entirely within supSet
    for (auto it = setOfPairs.begin(); it != setOfPairs.end();)
    {
        // Check if every element of it->sup is in supSet
        bool allContained = std::all_of(it->sup.begin(), it->sup.end(),
                                        [&](int x)
                                        { return std::find(supSet.begin(), supSet.end(), x) != supSet.end(); });
        if (allContained)
            it = setOfPairs.erase(it);
        else
            ++it;
    }
}