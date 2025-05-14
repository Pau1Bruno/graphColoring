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
    bestColorCount = -1;
    bestColorBottomLineColor = n;
    used.assign(n, false);
    LOG << "Graph n = " << n << '\n';
}

std::vector<std::vector<int>> OlemskoyColorGraph::resultColorNodes()
{
    bestPartition.clear();
    currentPartition.clear();
    firstBlockSeen.clear();

    LOG << "--- Начало алгоритма --- \n";
    searchBlocks(0);
    LOG << "--- Алгоритм закончен, минимальное количество цветов: " << bestColorCount << " ---\n";

    return bestPartition;
}

/*───────────────────────────────────────────────────────────────*/
void OlemskoyColorGraph::searchBlocks(int currentBlockIndex)
{
    bool allColored = true;
    for (int v = 0; v < n; ++v)
        if (!used[v]) { allColored = false; break; }

    if (allColored) {
        int blocksUsed = currentBlockIndex;     
        LOG << "Все вершины покрашены в " << blocksUsed
            << " блоков/блока\n";

        if (bestColorBottomLineColor == currentBlockIndex) {
            LOG << "Раскраска найдена \n";
            return;
        }
        
        if (blocksUsed < bestColorCount) {
            bestColorCount = blocksUsed;
            bestPartition  = currentPartition;
            LOG << "Найдено меньшее хроматическое число! Оно равно " << bestColorCount << " \n";
        }
        return;
    }

    LOG << "WTF \n";

    /* Ω */
    std::vector<int> omega;
    for (int v = 0; v < n; ++v)
        if (!used[v]) omega.push_back(v);

    /* запускаем построение блока */
    std::vector<int> currentBlock;     
    buildBlock(currentBlockIndex,
                0,
               currentBlock,
               omega);
}

/*───────────────────────────────────────────────────────────────*/
void OlemskoyColorGraph::buildBlock(int                     blockIndex,
                                    int                     level,
                                    std::vector<int>&       currentBlock,
                                    const std::vector<int>& omega)
{
    /*— блок закрыт —*/
    if (omega.empty()) {
        LOG << "Опорное множество пусто \n";
        LOG << "Блок(" << blockIndex << ", " << level << "): " << currentBlock << "\n";
        currentPartition.push_back(currentBlock);
        LOG << "Текущий набор блоков: " << currentPartition << "\n";
        searchBlocks(blockIndex + 1);
        currentPartition.pop_back();
        return;
    }

    LOG << "Опорное множество(" << blockIndex << ", " << level << "): " << omega << "\n";

    auto gPairs = buildGPairsHV(g, omega);
    LOG << "Возможные варианты продолжений " << gPairs << '\n';

    LOG << "Номер текущего блока: " << blockIndex << ", уровень: " << level << '\n';

    /* проверка типа A */
    if (blockIndex != 0 && !gPairs.empty()) {
        int ro  = (int)gPairs[0].set.size();
        if (ro == 0) ro = 1;
        LOG << "Проверка A [" << blockIndex << "] " << blockIndex << " + " << (int)omega.size() / ro << " < " << bestColorCount << "\n";
        if (blockIndex + (int)omega.size() / ro > bestColorCount) {
            LOG << "проверка A провалена, возврат к построению предыдущего блока blockIndex:= blockIndex - 1 \n";
            std::vector<int> empty = {};
            return buildBlock(blockIndex-1, 0, empty, omega);
        } else {
            LOG << "проверка A пройдена \n";
        }
    } 

    /* Проверка типа B — только для первого блока */
    if (blockIndex == 0 && !gPairs.empty()) {
        int ro  = (int)gPairs[0].set.size();
        if (ro == 0) ro = 1;
        int potential = 2 * (level) + ro;
        int flooredDiv = n / bestColorCount;
        LOG << "Проверка В [" << blockIndex << "] " << 2 * (level) << " + " << ro << " > " << flooredDiv << "\n";
        if (potential > flooredDiv) {
            LOG << "проверка В успешна, продолжаем построение \n";
            int theBestCount = (n + ro - 1) / ro;
            bestColorBottomLineColor = theBestCount;
            LOG << "Получена оценка снизу хроматического числа, оно равно: " << bestColorBottomLineColor << "\n";
        } else {
            LOG << "проверка В провалена, возврат к предыдущему уровню level:= level - 1 \n";
            std::vector<int> empty = {};
             return buildBlock(blockIndex-1, 0, empty, omega);
        }
    }

    /* Проверка типа С */
    if (blockIndex + 2 == bestColorCount && !gPairs.empty()) {
        int ro  = (int)gPairs[0].set.size();
        if (ro == 0) ro = 1;
        LOG << "Проверка С [" << blockIndex << "] " << blockIndex + 2 << " ?= " << bestColorCount << "\n";
        LOG << "Проверка С [" << blockIndex << "] " << 2*(level) + ro << " ?= " << omega.size() << "\n";
        if (blockIndex + 2 == bestColorCount && 2*(level) + ro == omega.size()) {
            LOG << "проверка C провалена, возврат к построению предыдущего блока blockIndex:= blockIndex - 1 \n";
            std::vector<int> empty = {};
            return buildBlock(blockIndex-1, 0, empty, omega);
        } else {
            LOG << "проверка C провалена \n";
        }   
    }

    /*— Перебор возможных продолжений —*/
    for (const auto& pr : gPairs)
    {
        int ro  = (int)pr.set.size();
        if (ro == 0) ro = 1;

        /*—— Добавляем (i,j) в текущий блок ——*/
        used[pr.i] = used[pr.j] = true;
        currentBlock.push_back(pr.i);
        currentBlock.push_back(pr.j);
        std::sort(currentBlock.begin(), currentBlock.end());

        /*—— Новое опорное множество ——*/
        std::vector<int> updatedOmega;
        for (int node : omega) {
            std::cout << node << std::endl;
            if (node == pr.i || node == pr.j) continue;
            if (!g.areAdjacent(pr.i,node) && !g.areAdjacent(pr.j,node)) updatedOmega.push_back(node);  
        }

        buildBlock(blockIndex,
                    level + 1,
                   currentBlock,
                   updatedOmega);

        /* откат */
        currentBlock.erase(std::remove(currentBlock.begin(),currentBlock.end(),pr.i),
                       currentBlock.end());
        currentBlock.erase(std::remove(currentBlock.begin(),currentBlock.end(),pr.j),
                       currentBlock.end());
        used[pr.i] = used[pr.j] = false;
    }

    for (int k = (int)omega.size() - 1; k >= 0; --k) {
        int v = omega[k];                

        if (blockIndex + 2 == bestColorCount &&
            2*(level) + (int)currentBlock.size() + 1 == bestColorCount)
            continue;

        LOG << "  single " << v << '\n';

        used[v] = true;
        currentBlock.push_back(v);

        buildBlock(blockIndex,
                    level + 1,
                   currentBlock,
                   {});

        currentBlock.pop_back();
        used[v] = false;
    }
}
