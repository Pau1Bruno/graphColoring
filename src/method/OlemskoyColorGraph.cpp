#include "OlemskoyColorGraph.h"
#include "GPair.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <vector>

/*---------------------------------------------------------------
   Единый поток-лог →  olemskoy_steps.txt
 ----------------------------------------------------------------*/
static std::ofstream LOG("olemskoy_steps.txt");

/*================================================================*/
/*                    OlemskoyColorGraph                          */
/*================================================================*/
OlemskoyColorGraph::OlemskoyColorGraph(const Graph& matrix) : g(matrix)
{
    n              = g.size();
    bestColorCount = n;
    used.assign(n, false);
    LOG << "Graph n = " << n << '\n';
}

std::vector<std::vector<int>> OlemskoyColorGraph::resultColorNodes()
{
    bestPartition.clear();
    currentPartition.clear();
    firstBlockSeen.clear();

    LOG << "\n=== start search ===\n";
    searchBlocks(0);
    LOG << "=== finished;  bestColorCount = "
        << bestColorCount << " ===\n";

    return bestPartition;
}

/*───────────────────────────────────────────────────────────────*/
void OlemskoyColorGraph::searchBlocks(int currentBlockIndex)
{
    /* все ли вершины уже раскрашены? */
    bool allColored = true;
    for (int v = 0; v < n; ++v)
        if (!used[v]) { allColored = false; break; }

    if (allColored) {
        int blocksUsed = currentBlockIndex;     
        LOG << "All vertices colored with " << blocksUsed
            << " blocks\n";
        if (blocksUsed < bestColorCount) {
            bestColorCount = blocksUsed;
            bestPartition  = currentPartition;
            LOG << "  new optimum!\n";
        }
        return;
    }

    /* Ω */
    std::vector<int> omega;
    for (int v = 0; v < n; ++v)
        if (!used[v]) omega.push_back(v + 1);

    LOG << "\n Current Ω " << omega << '\n';

    /* запускаем построение блока */
    std::vector<int> curBlock;     
    buildBlock(currentBlockIndex,
               curBlock,
               omega,                 
               1,                    
               (int)omega.size());
}

/*───────────────────────────────────────────────────────────────*/
void OlemskoyColorGraph::buildBlock(int                     blockIdx,
                                    std::vector<int>&       curBlock,
                                    const std::vector<int>& omega, // 1-idx
                                    int                     level,
                                    int                     initialΩ)
{
    /*— блок закрыт —*/
    if (omega.empty()) {
        LOG << "current block end: " << curBlock << "\n";
        currentPartition.push_back(curBlock);
        LOG << "current partition: " << currentPartition << "\n";
        searchBlocks(blockIdx + 1);
        currentPartition.pop_back();
        return;
    }

    /*— пары  (i,j)  по H/V —*/
    auto gPairs = buildGPairsHV(g, omega);
    LOG << "GPairs " << gPairs << '\n';


    /* проверка типа A */
    if (!gPairs.empty() && (blockIdx + omega.size() / (int)gPairs[0].set.size() >= bestColorCount)) {
        LOG << "Prune A " << "[" << blockIdx << "]: " << " ≥ best\n";

        return;
    }

    if (bestColorCount == 2) return;
  

    /*— основной перебор пар —*/
    for (const auto& pr : gPairs)
    {
        int i  = pr.i - 1;              // перевод в 0-индекс
        int j  = pr.j - 1;
        int ro  = (int)pr.set.size();
        if (ro == 0) ro = 1;              // правило GetRo

        /* Prune B — только для первого блока */
        if (blockIdx == 0) {
            int potential = 2 * (level - 1) + ro;
            int avgTarget = (n + bestColorCount - 1) / bestColorCount;
            LOG << "  test pair ("<<i+1<<","<<j+1<<")  ro="<<ro
                << "  potential="<<potential
                << "  avg="<<avgTarget << '\n';
            if (potential < avgTarget) { LOG << "  Prune B\n"; continue; }

            /* Prune D (симметрия) */
            int ceilDiv = (n + ro - 1) / ro;
            if (ceilDiv == bestColorCount
                || ceilDiv == bestColorCount - 1) {
                LOG << "  Prune D\n"; continue;
            }
        }

        /* Prune C — “ничего лучше не выйдет” */
        if (blockIdx + 1 == bestColorCount &&
            2*(level-1) + (int)curBlock.size() + 2 + ro == initialΩ) {
            LOG << "  Prune C\n"; continue;
        }

        /*—— добавляем (i,j) в текущий блок ——*/
        used[i] = used[j] = true;
        curBlock.push_back(i);
        curBlock.push_back(j);
        std::sort(curBlock.begin(), curBlock.end());

        /*—— новое Ω ——*/
        std::vector<int> nextΩ;
        for (int w1 : omega) {
            int w = w1 - 1;
            if (w == i || w == j) continue;
            if (!g.areAdjacent(i,w) && !g.areAdjacent(j,w))
                nextΩ.push_back(w1);           // 1-index!
        }

        buildBlock(blockIdx,
                   curBlock,
                   nextΩ,
                   level + 1,
                   initialΩ);

        /* откат */
        curBlock.erase(std::remove(curBlock.begin(),curBlock.end(),i),
                       curBlock.end());
        curBlock.erase(std::remove(curBlock.begin(),curBlock.end(),j),
                       curBlock.end());
        used[i] = used[j] = false;
    }

    /*———— ветка-“одиночка” ————*/
    for (int k = (int)omega.size() - 1; k >= 0; --k) {
        int v1 = omega[k];           // 1-idx
        int v  = v1 - 1;              // 0-idx

        if (blockIdx + 1 == bestColorCount &&
            2*(level-1) + (int)curBlock.size() + 1 == initialΩ)
            continue;

        LOG << "  single " << v+1 << '\n';

        used[v] = true;
        curBlock.push_back(v);

        buildBlock(blockIdx,
                   curBlock,
                   {},                 // пустое Ω
                   level + 1,
                   initialΩ);

        curBlock.pop_back();
        used[v] = false;
    }
}
