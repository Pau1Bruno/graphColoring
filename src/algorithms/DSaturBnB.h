#pragma once
#include <Eigen/Dense>
#include <vector>
#include <algorithm>
#include <numeric>
#include <unordered_set>

namespace DSaturBnB 
{
    using DenseMatrix =
        Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    // === Вспомогательная «быстрая» жадная раскраска (Upper Bound) ============
    static int greedyUB(const DenseMatrix& A, std::vector<int>& col) {
        const int n = A.rows();
        col.assign(n, -1);
        std::vector<int> deg(n);
        std::iota(deg.begin(), deg.end(), 0);
        std::sort(deg.begin(), deg.end(), [&](int a, int b) {
            int da = A.row(a).sum();
            int db = A.row(b).sum();
            return da > db;
        });
        int used = 0;
        for (int v : deg) {
            std::vector<bool> forbid(n, false);
            for (int u = 0; u < n; ++u)
                if (A(v, u) && col[u] != -1) forbid[col[u]] = true;
            int c = 0;
            while (forbid[c]) ++c;
            col[v] = c;
            used = std::max(used, c + 1);
        }
        return used;
    }

    // === Основная функция: точная DSATUR + Branch&Bound =======================
    static std::vector<int> color(const DenseMatrix& A)
    {
        const int n = A.rows();
        std::vector<int> best(n, -1);       // окончательное решение
        std::vector<int> cur (n, -1);       // текущая частичная раскраска
        int UB = n;                         // верхняя граница χ

        // получаем приличную начальную UB за O(n²)
        std::vector<int> tmp;
        UB = greedyUB(A, tmp);
        best = tmp;

        // пред-вычислим степенной массив (нужно только один раз)
        std::vector<int> degree(n);
        for (int v = 0; v < n; ++v)
            degree[v] = static_cast<int>(A.row(v).sum());

        // Рекурсивное B&B c DSATUR-выбором вершины
        std::function<void(int)> dfs = [&](int colored) {
            if (colored == n) {                    // все вершины раскрашены
                int used = 0;
                for (int c : cur) used = std::max(used, c + 1);
                if (used < UB) {
                    UB   = used;
                    best = cur;
                }
                return;
            }
            // --- выбираем вершину по DSATUR ---
            int sel = -1, maxSat = -1, maxDeg = -1;
            for (int v = 0; v < n; ++v) if (cur[v] == -1) {
                std::unordered_set<int> neighColors;
                for (int u = 0; u < n; ++u)
                    if (A(v,u) && cur[u] != -1) neighColors.insert(cur[u]);
                int sat = static_cast<int>(neighColors.size());
                if (sat > maxSat || (sat == maxSat && degree[v] > maxDeg)) {
                    sel    = v;
                    maxSat = sat;
                    maxDeg = degree[v];
                }
            }

            // --- запрещённые цвета для sel ---
            std::vector<bool> forbid(UB, false);
            for (int u = 0; u < n; ++u)
                if (A(sel,u) && cur[u] != -1) forbid[cur[u]] = true;

            // --- пробуем существующие цвета (0..UB-1) ---
            for (int c = 0; c < UB; ++c) {
                if (forbid[c]) continue;
                cur[sel] = c;
                dfs(colored + 1);
                cur[sel] = -1;
            }

            // --- новая ветка: создаём новый цвет (если не превысим UB-1) ---
            int newColor = 0;
            while (newColor < UB && forbid[newColor]) ++newColor;
            if (newColor == UB - 1) {              // можем добавить ровно UB-1?
                cur[sel] = newColor;
                dfs(colored + 1);
                cur[sel] = -1;
            }
        };

        dfs(0);

        return best;
    }
}
