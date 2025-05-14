#ifndef OLEMSKOY_COLOR_GRAPH_H
#define OLEMSKOY_COLOR_GRAPH_H

#include <vector>
#include <unordered_set>
#include <algorithm>              
#include "Graph.h"
#include "GPair.h"

/*  состояние одного уровня  ω^j,s , Q^j,s , F^j,s */
struct LevelState {
    std::vector<int> omega;        // ω^{j,s}
    std::vector<int> Q;            // Q^{j,s}
    std::vector<int> F;            // F^{j,s}
    std::vector<GPair> G;          // G^{j,s}  (варианты продолжения)
};

class OlemskoyColorGraph {
private:
    /*------------- входные данные -------------*/
    Graph g;
    int   n;

    /*------------- найденное лучшее решение ---*/
    int bestColorCount;              // минимальное число блоков (цветов)
    int bestColorBottomLineColor;    // оценка снизу (из проверки B)
    std::vector<std::vector<int>> bestPartition;

    /*--------------  новый «стек уровней» ---------------*/
    std::vector<LevelState> lvl;   // стэк ω/Q/F/G по уровням

    /* вспомогательные функции */
    void pushLevel(const std::vector<int>& omega,
                   const std::vector<GPair>& G);   // создать новый LevelState
    void popLevel();                               // удалить s-й уровень

    /* обновление Q  и  F  */
    void markAsNode (int v);   // Q^{j,s}  ←  Q^{j,s} ∪ {v}
    void markAsTail (int v);   // F^{j,s}  ←  F^{j,s} ∪ {v}

    /*------------- текущее состояние поиска ---*/
    std::vector<bool>               used;              // какие вершины уже «закрыты»
    std::vector<std::vector<int>>   currentPartition;  // построенные блоки
    std::unordered_set<long long>   firstBlockSeen;    // симметр.-память для 1-го блока

    /*------------- рекурсивные процедуры -------*/
    void searchBlocks(int currentBlockIndex);
    void buildBlock(int blockIndex, int level,
                    std::vector<int>&       currentBlock,
                    const std::vector<int>& omega);

    /*------------- Ψ/Z-прореживание ------------*/
    static std::vector<int>  computePsi(const std::vector<int>& omega,
                                    const std::vector<int>& currentBlock,
                                    int                     level);

    static std::vector<int>  computeZ  (const std::vector<GPair>& gPairs,
                                    const std::vector<int>&   psi);

    static std::vector<int>  pruneOmega(const std::vector<int>& omega,
                                    const std::vector<GPair>& gPairs,
                                    const std::vector<int>& currentBlock,
                                    int  level);

public:
    explicit OlemskoyColorGraph(const Graph& matrix);

    // результат — список цветовых классов
    std::vector<std::vector<int>> resultColorNodes();
};

#endif // OLEMSKOY_COLOR_GRAPH_H
