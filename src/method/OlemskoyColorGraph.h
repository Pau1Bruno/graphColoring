#ifndef OLEMSKOY_COLOR_GRAPH_H
#define OLEMSKOY_COLOR_GRAPH_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>      // std::pair
#include <algorithm>

#include "Graph.h"
#include "GPair.h"

/*---------------------------------------------------------------*
 |  общий ключ (j,s) = (blockIndex, level)                       |
 *---------------------------------------------------------------*/
using Key = std::pair<int,int>;

struct PairHash {
    std::size_t operator()(const Key& k) const noexcept
    {
        return (static_cast<std::size_t>(k.first) << 32) ^
                static_cast<std::size_t>(k.second);
    }
};

/*================================================================*/
/*                     OlemskoyColorGraph                         */
/*================================================================*/
class OlemskoyColorGraph
{
private:
    /*------------- входные данные -------------*/
    Graph g;
    int   n = 0;

    /*------------- найденное лучшее решение ---*/
    int bestColorCount           = 0;    // минимальное χ
    int bestColorBottomLineColor = -1;   // нижняя оценка из проверки B
    std::vector<std::vector<int>> bestPartition;

    /*------------- текущее состояние поиска ---*/
    std::vector<bool>             used;              // вершина уже «закрыта»?
    std::vector<std::vector<int>> currentPartition;  // построенные блоки
    std::unordered_set<long long> firstBlockSeen;    // симметр-кэш 1-го блока

    /*----------- хранение ω, Q, F, G по (j,s) -------------*/
    std::unordered_map<Key, std::vector<int>, PairHash> omega_;
    std::unordered_map<Key, std::vector<std::vector<int>>, PairHash> Q_;
    std::unordered_map<Key, std::vector<int>, PairHash> F_;
    std::unordered_map<Key, std::vector<GPair>, PairHash> G_;
    std::unordered_map<Key, std::vector<GPair>, PairHash> Z_;

    /*------------- служебные методы -------------*/
    void setLevelData(int j, int s,
                      const std::vector<int>&   omega,
                      const std::vector<GPair>& G);   // сохранить ω и G, обнулить Q,F
    void setOmega(int j,int s, const std::vector<int>& omega);              // F^{j,s} ← … ∪ {q, r}
    void addQ(int j,int s,int q, int r);              // Q^{j,s} ← … ∪ {(q,r)}
    void addF(int j,int s,int q, int r);              // F^{j,s} ← … ∪ {q, r}
    void addF(int j,int s,int v);                     // F^{j,s} ← … ∪ {v}
    void eraseLevel(int j,int s);                     // удалить все 4 контейнера

    /* получение ω,Q,F,G,Z – безопасно, без исключений */
    const std::vector<int>&                     getOmega(int j,int s) const;
    const std::vector<std::vector<int>>&        getQ    (int j,int s) const;
    const std::vector<int>&                     getF    (int j,int s) const;
    const std::vector<GPair>&                   getG    (int j,int s) const;
    const std::vector<GPair>&                   getZ    (int j,int s) const;

    /*------------- рекурсивные процедуры -------*/
    void searchBlocks(int currentBlockIndex);
    void buildBlock  (int blockIndex, int level,
                      std::vector<int>&       currentBlock,
                      const std::vector<int>& omega,
                    std::vector<int>& r_j
                    );

    /*------------- Ψ/Z-прореживание ------------*/
    std::vector<int> computePsi(int j,
                            int s,
                            const std::vector<int>& J) const;

    std::vector<int> pruneOmega(int j, int s, const std::vector<int>& J) const;

public:
    explicit OlemskoyColorGraph(const Graph& matrix);

    // результат — список цветовых классов
    std::vector<std::vector<int>> resultColorNodes();
};

#endif  // OLEMSKOY_COLOR_GRAPH_H
