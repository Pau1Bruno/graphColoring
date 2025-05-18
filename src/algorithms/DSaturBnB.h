#ifndef DSATUR_BNB_H
#define DSATUR_BNB_H

#include <Eigen/Dense>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cstdint>
#include <functional>

namespace DSaturBnB
{
/*--------------------------------------------------------------*/
/*  Тип матрицы (0/1, RowMajor удобно для A(i,j)-доступа)       */
/*--------------------------------------------------------------*/
using DenseMatrix =
    Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

/*--------------------------------------------------------------*/
/*  Быстрый жадный Welsh–Powell  ⇒  начальная верхняя граница   */
/*--------------------------------------------------------------*/
inline int greedyUB(const DenseMatrix& A, std::vector<int>& col)
{
    const int n = A.rows();
    col.assign(n, -1);

    /* — упорядочиваем вершины по убыванию степеней — */
    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b)
              { return A.row(a).sum() > A.row(b).sum(); });

    int used = 0;
    std::vector<int> forbid(n, 0), stamp(n, 0);
    int curStamp = 1;

    for (int v : order)
    {
        /* отмечаем занятые цвета */
        for (int u = 0; u < n; ++u)
            if (A(v, u) && col[u] != -1)
                stamp[col[u]] = curStamp;

        int c = 0;
        while (stamp[c] == curStamp) ++c;
        col[v] = c;
        used   = std::max(used, c + 1);
        ++curStamp;
    }
    return used;                       /* χᴳ */
}

/*--------------------------------------------------------------*/
/*  Основная функция: точная DSATUR + B&B                       */
/*--------------------------------------------------------------*/
inline std::vector<int> color(const DenseMatrix& A)
{
    const int n = A.rows();

    /* ---------- списки смежности + степени ---------- */
    std::vector<std::vector<int>> adj(n);
    std::vector<int> degree(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (A(i, j))
            {
                adj[i].push_back(j);
                ++degree[i];
            }

    /* ---------- начальная greedy-граница χᴳ ---------- */
    std::vector<int> best;                 // сюда greedyUB запишет раскраску
    int UB = greedyUB(A, best);            // UB = χᴳ

    /* ---------- рабочие структуры DFS  --------------- */
    std::vector<int> colour(n, -1);        // текущая раскраска
    std::vector<int> sat   (n,  0);        // насыщенность (|разн. цветов|)
    std::vector<std::vector<uint8_t>>      // forbidCnt[v][c] : счётчик
        forbidCnt(n, std::vector<uint8_t>(UB, 0));

    int maxUsed = 0;                       // max(colour)+1 в текущем пути

    /* ---------- рекурсивный поиск  ------------------- */
    std::function<void(int)> dfs = [&](int colored)
    {
        /* нижняя граница χ  (Brooks-like) */
        int low = maxUsed;
        for (int v = 0; v < n; ++v)
            if (colour[v] == -1)
                low = std::max(low, sat[v] + 1);
        if (low >= UB) return;             // отсечение

        if (colored == n)                  // нашли полную раскраску
        {
            UB   = maxUsed;
            best = colour;
            return;
        }

        /* ---- выбираем вершину по DSATUR ---- */
        int v = -1, bestSat = -1, bestDeg = -1;
        for (int i = 0; i < n; ++i)
            if (colour[i] == -1 &&
               (sat[i] > bestSat || (sat[i] == bestSat && degree[i] > bestDeg)))
            {
                v       = i;
                bestSat = sat[i];
                bestDeg = degree[i];
            }

        /* ---- перебираем уже используемые цвета ---- */
        for (int c = 0; c < maxUsed; ++c)
        {
            if (forbidCnt[v][c]) continue;         // цвет запрещён

            /* assign colour c */
            colour[v] = c;
            for (int u : adj[v])
                if (colour[u] == -1 && forbidCnt[u][c]++ == 0) ++sat[u];

            dfs(colored + 1);

            /* undo */
            for (int u : adj[v])
                if (colour[u] == -1 && --forbidCnt[u][c] == 0) --sat[u];
            colour[v] = -1;
        }

        /* ---- пытаемся завести НОВЫЙ цвет ---- */
        if (maxUsed + 1 < UB)
        {
            int c = maxUsed;
            colour[v] = c;
            for (int u : adj[v])
                if (colour[u] == -1 && forbidCnt[u][c]++ == 0) ++sat[u];

            ++maxUsed;
            dfs(colored + 1);
            --maxUsed;

            for (int u : adj[v])
                if (colour[u] == -1 && --forbidCnt[u][c] == 0) --sat[u];
            colour[v] = -1;
        }
    };

    dfs(/*colored=*/0);
    return best;                           // 0-based цвета
}

} // namespace DSaturBnB
#endif /* DSATUR_BNB_H */