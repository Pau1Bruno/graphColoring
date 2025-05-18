#include "OlemskoyColorGraph.h"
#include "GPair.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>

/*---------------------------------------------------------------*
 |  Единый поток-лог →  olemskoy_steps.txt                       |
 *---------------------------------------------------------------*/
static std::ofstream LOG("olemskoy_steps.txt");

/* ---------- «пустые» статические контейнеры ---------- */
static const std::vector<int>              kEmptyIntVec;
static const std::vector<std::vector<int>> kEmptyInt2d;
static const std::vector<GPair>            kEmptyGPairVec;


/*================================================================*/
/*                    OlemskoyColorGraph                          */
/*================================================================*/
OlemskoyColorGraph::OlemskoyColorGraph(const Graph& matrix) : g(matrix)
{
    n                        = g.size();
    bestColorCount           = n;       // стартовая оценка χ
    bestColorBottomLineColor = n;
    used.assign(n, false);
    LOG << "Graph n = " << n << '\n';
}

/*---------------------------------------------------------------*
 |                                                                |
 *---------------------------------------------------------------*/
std::vector<std::vector<int>> OlemskoyColorGraph::resultColorNodes()
{
    bestPartition.clear();
    currentPartition.clear();
    firstBlockSeen.clear();

    omega_.clear(); Q_.clear(); F_.clear(); G_.clear();

    LOG << "--- Начало алгоритма --- \n";
    searchBlocks(0);
    LOG << "--- Алгоритм закончен, минимальное количество цветов: "
        << bestColorCount << " ---\n";

    return bestPartition;
}

/*---------------------------------------------------------------*
 |                ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ                        |
 *---------------------------------------------------------------*/
void OlemskoyColorGraph::setLevelData(int j, int s,
                                      const std::vector<int>&   omega,
                                      const std::vector<GPair>& G)
{
    Key key{j,s};
    omega_[key] = omega;
    G_[key] = G;

    Q_[key] = kEmptyInt2d;
    F_[key] = kEmptyIntVec;
}

void OlemskoyColorGraph::setOmega(int j,int s, const std::vector<int>& omega) { 
    omega_[{j,s}] = omega;
 }

void OlemskoyColorGraph::addQ(int j,int s,int q, int r) { 
    Q_[{j,s}].push_back({q,r});
 }
 
void OlemskoyColorGraph::addF(int j,int s,int q, int r) { 
    F_[{j,s}].push_back(q);
    F_[{j,s}].push_back(r);
 }

 void OlemskoyColorGraph::addF(int j,int s,int v) { 
    F_[{j,s}].push_back(v);
 }

void OlemskoyColorGraph::eraseLevel(int j,int s)
{
    Key k{j,s};
    omega_.erase(k);
    Q_.erase(k);
    F_.erase(k);
    G_.erase(k);
}

/* ---------- безопасные геттеры ---------- */
const std::vector<int>&
OlemskoyColorGraph::getOmega(int j,int s) const
{
    auto it = omega_.find({j,s});
    return (it == omega_.end()) ? kEmptyIntVec : it->second;
}

const std::vector<std::vector<int>>&
OlemskoyColorGraph::getQ(int j,int s) const
{
    auto it = Q_.find({j,s});
    return (it == Q_.end()) ? kEmptyInt2d : it->second;
}

const std::vector<int>&
OlemskoyColorGraph::getF(int j,int s) const
{
    auto it = F_.find({j,s});
    return (it == F_.end()) ? kEmptyIntVec : it->second;
}

const std::vector<GPair>&
OlemskoyColorGraph::getG(int j,int s) const
{
    auto it = G_.find({j,s});
    return (it == G_.end()) ? kEmptyGPairVec : it->second;
}

const std::vector<GPair>&
OlemskoyColorGraph::getZ(int j,int s) const
{
    auto it = Z_.find({j,s});
    return (it == Z_.end()) ? kEmptyGPairVec : it->second;
}

/*---------------------------------------------------------------*
 |  Ψ^{j,s} = J^{j} \ ⋃_{μ=1}^{s-1} Q^{j,μ}                      |
 *---------------------------------------------------------------*/
std::vector<int>
OlemskoyColorGraph::computePsi(int j, int s,
                               const std::vector<int>& J) const
{
    /* 1. skip = ⋃_{μ=1}^{s-1} Q^{j,μ} */
    std::unordered_set<int> skip;

    for (int mu = 1; mu < s; ++mu) {                 // μ = 1 … s-1
        const auto& qLevel = getQ(j, mu);            // ← геттер, может быть пуст
        for (const auto& qpair : qLevel)             // каждая пара <α1,α2>
            for (int v : qpair) skip.insert(v);      // обе вершины → skip
    }

    /* 2. Ψ = J \ skip */
    std::vector<int> psi;
    psi.reserve(J.size());
    for (int v : J)
        if (!skip.count(v)) psi.push_back(v);

    return psi;                                      // Ψ^{j,s}
}

/*---------------------------------------------------------------*
 |  Ψ\Z-прореживание                                             |
 *---------------------------------------------------------------*/
std::vector<int>
OlemskoyColorGraph::pruneOmega(int j, int s,
                               const std::vector<int>& J) const
{
    auto psi = computePsi(j, s, J);          // Ψ^{j,s}

    /* Z^{j,s} формируем из сохранённых G^{j,s} */
    const auto& gPairs = getG(j, s);         // может быть пуст
    std::vector<int> Z;
    for (const auto& gp : gPairs)
        if (gp.set == psi) {                 // D_{α}^{j,s} == Ψ ?
            Z.push_back(gp.i);
            Z.push_back(gp.j);
        }

    /* Ψ \ Z */
    std::vector<int> pruned;
    for (int v : psi)
        if (std::find(Z.begin(), Z.end(), v) == Z.end())
            pruned.push_back(v);

    LOG << "Ψ (" << j << ", " << s << ")" <<psi<< '\n';
    LOG << "Z (" << j << ", " << s << ")" <<Z<< '\n';
    LOG << "Ψ\\Z (" << j << ", " << s << ")" <<pruned<< '\n';
    return pruned;
}

/*---------------------------------------------------------------*
 |                 ОСНОВНАЯ РЕКУРСИЯ                             |
 *---------------------------------------------------------------*/
void OlemskoyColorGraph::searchBlocks(int currentBlockIndex)
{
    bool allColored = true;
    for (int v = 0; v < n; ++v)
        if (!used[v]) { allColored = false; break; }

    if (allColored) {
        int blocksUsed = currentBlockIndex;
        LOG << "Все вершины покрашены в " << blocksUsed << " блоков/блока\n";

        if (blocksUsed < bestColorCount) {
            bestColorCount = blocksUsed;
            bestPartition  = currentPartition;
            LOG << "Найдено меньшее хроматическое число! Оно равно "
                << bestColorCount << " \n";
        }

        return;
    }

    /* Ω — множество ещё не закрашенных */
    std::vector<int> omega;
    for (int v = 0; v < n; ++v)
        if (!used[v]) omega.push_back(v);

    /* старт построения блока */
    std::vector<int> currentBlock;
    std::vector<int> r_j(n, -1);

    buildBlock(currentBlockIndex, 0, currentBlock, omega, r_j);
}

/*───────────────────────────────────────────────────────────────*/
void OlemskoyColorGraph::buildBlock(int blockIndex, int level,
                                    std::vector<int>&       currentBlock,
                                    const std::vector<int>& omega,
                                std::vector<int>&       r_j
                                )
{
    r_j[blockIndex] = level;
    setOmega(blockIndex, level, omega);
    /*-------------------- БАЗА: ω пусто ------------------------*/
    if (omega.empty()) {
        LOG << "Опорное множество(" << blockIndex << ", " << level
        << "): " << omega << "пусто \n";
        LOG << "Блок(" << blockIndex << ", " << level << "): "
            << currentBlock << "\n";

        /* Ψ\Z-прореживание перед переходом к следующему блоку */
        if (level > 0)
        {
            std::vector<int> singles = pruneOmega(blockIndex, level, currentBlock);
            
            for (int v : singles) {
                addF(blockIndex, level, v);                  // F^{j,s-1} ← … ∪ {v}
                LOG << "Добавлена единичная вершина: " << v << '\n';
            }
        
            LOG << "Q("<< blockIndex << ", " << level << ") = "
                << getQ(blockIndex, level) << '\n';
            LOG << "F("<< blockIndex << ", " << level << ") = "
                << getF(blockIndex, level) << '\n';
        }
        currentPartition.push_back(currentBlock);
        LOG << "Текущий набор блоков: " << currentPartition << "\n";

        searchBlocks(blockIndex + 1);

        currentPartition.pop_back();
        return;
    }

    /*-------------------- ОБЫЧНЫЙ СЛУЧАЙ -----------------------*/
    LOG << "Опорное множество(" << blockIndex << ", " << level
        << "): " << omega << "\n";

    LOG << "Q: (" << blockIndex << ", " << level
    << "): " << getQ(blockIndex, level) << '\n';

    auto gPairs = buildGPairsHV(g, omega, getQ(blockIndex, level));
    LOG << "Возможные варианты продолжений G\\Q " << gPairs << '\n';

    LOG << "Номер текущего блока: " << blockIndex
        << ", уровень: " << level << '\n';

    /*-------- проверки A/B/C                            --------*/
    if (blockIndex != 0 && !gPairs.empty()) {                  // A
        int ro = std::max<int>(1, gPairs[0].set.size());
        LOG << "Проверка A [" << blockIndex << "] "
            << blockIndex << " + " << (int)omega.size() / ro
            << " < " << bestColorCount << "\n";
        if (blockIndex + (int)omega.size() / ro > bestColorCount) {
            LOG << "проверка A провалена\n";
            blockIndex--;
            level= r_j[blockIndex];
            return;
        }
    }

    if (blockIndex == 0 && !gPairs.empty()) {                  // B
        int ro         = std::max<int>(1, gPairs[0].set.size());
        int potential  = 2 * level + ro;
        int flooredDiv = n / bestColorCount;
        LOG << "Проверка В [" << blockIndex << "] "
            << 2 * level << " + " << ro << " > "
            << flooredDiv << "\n";
        if (potential > flooredDiv) {
            LOG << "проверка В успешна\n";
            if (bestColorBottomLineColor > (n + ro - 1) / ro) {
                bestColorBottomLineColor = (n + ro - 1) / ro;
                LOG << "Получена нижняя оценка χ: "
                    << bestColorBottomLineColor << "\n";
            }
        } else {
            LOG << "проверка В провалена\n";
            return;
        }
    }

    if (blockIndex + 2 == bestColorCount && !gPairs.empty()) { // C
        int ro = std::max<int>(1, gPairs[0].set.size());
        if (2 * level + ro == (int)omega.size()) {
            LOG << "проверка C провалена\n";
            return;
        }
    }

    /*---------------- сохраняем данные уровня ------------------*/
    setLevelData(blockIndex, level, omega, gPairs);

    /*---------------- перебираем пары (α) ----------------------*/
    for (const auto& pr : gPairs) {
        addQ(blockIndex, level, pr.i, pr.j);

        LOG << "Q: (" << blockIndex << ", " << level
        << "): " << getQ(blockIndex, level) << '\n';

        used[pr.i] = used[pr.j] = true;
        currentBlock.push_back(pr.i);
        currentBlock.push_back(pr.j);
        std::sort(currentBlock.begin(), currentBlock.end());

        /* ω  ←  ω  \ { i,j }  \ N(i)  \ N(j) */
        std::vector<int> updatedOmega;
        for (int node : omega)
            if (node != pr.i && node != pr.j &&
                !g.areAdjacent(pr.i,node) && !g.areAdjacent(pr.j,node))
                updatedOmega.push_back(node);

        buildBlock(blockIndex, level + 1, currentBlock, updatedOmega, r_j);

        currentBlock.erase(std::remove(currentBlock.begin(),
                                       currentBlock.end(), pr.i),
                           currentBlock.end());
        currentBlock.erase(std::remove(currentBlock.begin(),
                                       currentBlock.end(), pr.j),
                           currentBlock.end());
        used[pr.i] = used[pr.j] = false;
    }

    /*--- очистка данных уровня (как popLevel) ---*/
    eraseLevel(blockIndex, level);
}
