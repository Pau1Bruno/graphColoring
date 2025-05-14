// OlemskoyColorGraph.cpp
#include "OlemskoyColorGraph.h"
#include "GPair.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <unordered_set>

/*---------------------------------------------------------------
   Единый поток-лог →  olemskoy_steps.txt
 ----------------------------------------------------------------*/
static std::ofstream LOG("olemskoy_steps.txt");

/*================================================================*/
/*                    OlemskoyColorGraph                          */
/*================================================================*/
OlemskoyColorGraph::OlemskoyColorGraph(const Graph &matrix) : g(matrix)
{
    n                        = g.size();
    bestColorCount           = n;   // стартовая оценка χ
    bestColorBottomLineColor = -1;  // нижняя оценка пока не получена
    used.assign(n, false);
    LOG << "Graph n = " << n << '\n';
}

std::vector<std::vector<int>> OlemskoyColorGraph::resultColorNodes()
{
    bestPartition.clear();
    currentPartition.clear();
    firstBlockSeen.clear();
    lvl.clear();

    LOG << "--- Начало алгоритма --- \n";
    searchBlocks(0);
    LOG << "--- Алгоритм закончен, минимальное количество цветов: "
        << bestColorCount << " ---\n";

    return bestPartition;
}

/*================================================================*/
/*                 ВСПОМОГАТЕЛЬНЫЕ ПРОЦЕДУРЫ                      */
/*================================================================*/
std::vector<int>
OlemskoyColorGraph::computePsi(const std::vector<int>& omega,
                               const std::vector<int>& currentBlock,
                               int level)
{
    std::unordered_set<int> skip(currentBlock.begin(),
                                 currentBlock.begin() + 2 * level);

    std::vector<int> psi;
    psi.reserve(omega.size());
    for (int v : omega)
        if (!skip.count(v)) psi.push_back(v);
    return psi;
}

std::vector<int>
OlemskoyColorGraph::computeZ(const std::vector<GPair>& gPairs,
                             const std::vector<int>&   psi)
{
    std::vector<int> Z;
    for (const auto& gp : gPairs)
        if (gp.set == psi) {           // D_{α}^{j,s} == Ψ
            Z.push_back(gp.i);
            Z.push_back(gp.j);
        }
    return Z;
}

std::vector<int>
OlemskoyColorGraph::pruneOmega(const std::vector<int>& omega,
                               const std::vector<GPair>& gPairs,
                               const std::vector<int>& currentBlock,
                               int level)
{
    auto psi = computePsi(omega, currentBlock, level);
    auto Z   = computeZ  (gPairs, psi);

    std::vector<int> pruned;
    for (int v : psi)
        if (std::find(Z.begin(), Z.end(), v) == Z.end())
            pruned.push_back(v);

    LOG << "Ψ   = " << psi    << '\n';
    LOG << "Z   = " << Z      << '\n';
    LOG << "Ψ\\Z = " << pruned << '\n';
    return pruned;
}

/*================================================================*/
/*                          РЕКУРСИЯ                              */
/*================================================================*/
void OlemskoyColorGraph::searchBlocks(int currentBlockIndex)
{
    bool allColored = true;
    for (int v = 0; v < n; ++v)
        if (!used[v]) { allColored = false; break; }

    if (allColored) {
        int blocksUsed = currentBlockIndex;
        LOG << "Все вершины покрашены в " << blocksUsed << " блоков/блока\n";

        if (bestColorBottomLineColor == currentBlockIndex) {
            LOG << "Раскраска найдена \n";
            return;
        }

        if (blocksUsed < bestColorCount) {
            bestColorCount = blocksUsed;
            bestPartition  = currentPartition;
            LOG << "Найдено меньшее хроматическое число! Оно равно "
                << bestColorCount << " \n";
        }
        return;
    }

    /* Ω */
    std::vector<int> omega;
    for (int v = 0; v < n; ++v)
        if (!used[v]) omega.push_back(v);

    /* запуск построения блока */
    std::vector<int> currentBlock;
    buildBlock(currentBlockIndex, /*s=*/0, currentBlock, omega);
}

/*───────────────────────────────────────────────────────────────*/
void OlemskoyColorGraph::buildBlock(int blockIndex, int level,
                                    std::vector<int> &currentBlock,
                                    const std::vector<int> &omega)
{
    /*— блок закрыт —*/
    if (omega.empty()) {
        LOG << "Опорное множество пусто \n";
        LOG << "Блок(" << blockIndex << ", " << level << "): "
            << currentBlock << "\n";

        /* Ψ\Z-прореживание ПЕРЕД переходом к следующему блоку */
        if (!lvl.empty()) {
            const auto& last = lvl.back();
            std::vector<int> singles =
                pruneOmega(last.omega, last.G, currentBlock, level);

            /* добавляем все одиночки к F^{j,s} текущего уровня */
            for (int v : singles) {
                markAsTail(v);
                LOG << "   F-tail added: " << v << '\n';
            }

            /*— лог текущих Q,F —*/
            LOG << "Q(level " << level << ") = " << last.Q << '\n';
            LOG << "F(level " << level << ") = " << last.F << '\n';
        }

        currentPartition.push_back(currentBlock);
        LOG << "Текущий набор блоков: " << currentPartition << "\n";

        searchBlocks(blockIndex + 1);

        currentPartition.pop_back();
        return;
    }

    LOG << "Опорное множество(" << blockIndex << ", " << level
        << "): " << omega << "\n";

    auto gPairs = buildGPairsHV(g, omega);
    LOG << "Возможные варианты продолжений " << gPairs << '\n';

    LOG << "Номер текущего блока: " << blockIndex << ", уровень: "
        << level << '\n';

    /*-------------            проверки A/B/C               --------*/
    if (blockIndex != 0 && !gPairs.empty()) {                  // A
        int ro = std::max<int>(1, gPairs[0].set.size());
        LOG << "Проверка A [" << blockIndex << "] " << blockIndex
            << " + " << (int)omega.size() / ro << " < "
            << bestColorCount << "\n";
        if (blockIndex + (int)omega.size() / ro > bestColorCount) {
            LOG << "проверка A провалена, возврат к построению "
                   "предыдущего блока blockIndex:= blockIndex - 1 \n";
            return;
        } else {
            LOG << "проверка A пройдена \n";
        }
    }

    if (blockIndex == 0 && !gPairs.empty()) {                  // B
        int ro         = std::max<int>(1, gPairs[0].set.size());
        int potential  = 2 * (level) + ro;
        int flooredDiv = n / bestColorCount;
        LOG << "Проверка В [" << blockIndex << "] " << 2 * (level)
            << " + " << ro << " > " << flooredDiv << "\n";
        if (potential > flooredDiv) {
            LOG << "проверка В успешна, продолжаем построение \n";
            int theBest = (n + ro - 1) / ro;
            bestColorBottomLineColor = theBest;
            LOG << "Получена оценка снизу хроматического числа, "
                   "оно равно: "
                << bestColorBottomLineColor << "\n";
        } else {
            LOG << "проверка В провалена, возврат к предыдущему "
                   "уровню level:= level - 1 \n";
            return;
        }
    }

    if (blockIndex + 2 == bestColorCount && !gPairs.empty()) { // C
        int ro = std::max<int>(1, gPairs[0].set.size());
        LOG << "Проверка С [" << blockIndex << "] " << blockIndex + 2
            << " ?= " << bestColorCount << "\n";
        LOG << "Проверка С [" << blockIndex << "] "
            << 2 * (level) + ro << " ?= " << omega.size() << "\n";
        if (blockIndex + 2 == bestColorCount &&
            2 * (level) + ro == (int)omega.size()) {
            LOG << "проверка C провалена, возврат к построению "
                   "предыдущего блока blockIndex:= blockIndex - 1 \n";
            return;
        } else {
            LOG << "проверка C провалена \n";
        }
    }

    /*================ pushLevel — ω/G =========================*/
    pushLevel(omega, gPairs);

    /*================ 1. перебираем пары (α) ==================*/
    for (const auto &pr : gPairs) {
        markAsNode(pr.i);
        markAsNode(pr.j);

        used[pr.i] = used[pr.j] = true;
        currentBlock.push_back(pr.i);
        currentBlock.push_back(pr.j);
        std::sort(currentBlock.begin(), currentBlock.end());

       std::vector<int> updatedOmega;
        for (int node : omega)
            if (node != pr.i && node != pr.j &&
                !g.areAdjacent(pr.i,node) && !g.areAdjacent(pr.j,node))
                updatedOmega.push_back(node);

        buildBlock(blockIndex, level + 1, currentBlock, updatedOmega);

        currentBlock.erase(std::remove(currentBlock.begin(),
                                       currentBlock.end(), pr.i),
                           currentBlock.end());
        currentBlock.erase(std::remove(currentBlock.begin(),
                                       currentBlock.end(), pr.j),
                           currentBlock.end());
        used[pr.i] = used[pr.j] = false;
    }

    /* откат уровня */
    popLevel();
}

/*------------ работа с «стеком уровней» ----------------*/
void OlemskoyColorGraph::pushLevel(const std::vector<int>& omega,
                                   const std::vector<GPair>& G)
{
    lvl.push_back(LevelState{});
    lvl.back().omega = omega;
    lvl.back().G     = G;
}

void OlemskoyColorGraph::popLevel() { lvl.pop_back(); }

void OlemskoyColorGraph::markAsNode(int v) { if(!lvl.empty()) lvl.back().Q.push_back(v); }
void OlemskoyColorGraph::markAsTail(int v) { if(!lvl.empty()) lvl.back().F.push_back(v); }