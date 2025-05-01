#include "OlemskoyColorGraph.h"
#include "Utils.h"
#include <numeric>   // std::iota
#include <algorithm> // std::sort, std::all_of
#include <cassert>
#include <iostream>

OlemskoyColorGraph::OlemskoyColorGraph(const Matrix &adj)
    : N_(static_cast<int>(adj.rows())), graph_(adj), maxColors_(N_), isOver_(false)
{
    assert(adj.rows() == adj.cols());
    init();

    std::vector<int> emptyUni;
    std::vector<int> emptyTemp;
    build(emptyUni, emptyTemp);
    initResult();
}

void OlemskoyColorGraph::init()
{
    bestColors_.clear();
    tmpColors_.clear();
    watchedFirstBlocks_.clear();
    lvlVariants_.clear();
    mainSupSet_.resize(N_);
    // fill with 1..N (instead of 0..N-1)
    std::iota(mainSupSet_.begin(), mainSupSet_.end(), 1);
    maxColors_ = N_;
    isOver_ = false;
}

void OlemskoyColorGraph::initResult()
{
    // bestColors_ now holds the final coloring
}

int OlemskoyColorGraph::getRo(const Variants &vs, const DSet &dSet) const
{
    if (vs.setOfPairs.empty())
        return 1;
    int s = static_cast<int>(dSet.set.size());
    return s > 0 ? s : 1;
}

std::vector<int> OlemskoyColorGraph::createSupportSet() const
{
    std::vector<bool> used(N_, false);
    for (auto &cls : tmpColors_)
        for (int v : cls)
            used[v] = true;
    std::vector<int> sup;
    for (int i = 0; i < N_; ++i)
        if (!used[i])
            sup.push_back(i);
    return sup;
}

bool OlemskoyColorGraph::blockCheckA(int lenMax, int uniLen) const
{
    double val = tmpColors_.size() + static_cast<double>(uniLen) / lenMax;
    return val >= maxColors_;
}

bool OlemskoyColorGraph::blockCheckB(int curLvl, int ro) const
{
    return 2 * (curLvl - 1) + ro < N_ / maxColors_;
}

bool OlemskoyColorGraph::blockCheckC(int curLvl, int uniLen, int ro) const
{
    return 2 * (curLvl - 1) + ro == uniLen;
}

bool OlemskoyColorGraph::blockCheckD(int ro) const
{
    int t1 = N_ / ro;
    double t2 = static_cast<double>(N_) / ro;
    return (t1 == t2) ? (t1 == maxColors_) : (t1 == maxColors_ - 1);
}

void OlemskoyColorGraph::createVariants(const std::vector<int> &uni, bool newColor)
{
    Variants res;
    if (!newColor)
    {
        res = lvlVariants_.back().sieve(uni);
    }
    else if (!lvlVariants_.empty())
    {
        res = lvlVariants_.front().sieve(uni);
    }
    else
    {
        res = Variants::createFromMatrix(graph_, mainSupSet_);
    }

    lvlVariants_.push_back(std::move(res));
}

void OlemskoyColorGraph::buildNotNullVariants(
    const Variants &variants,
    const std::vector<int> &uni,
    std::vector<int> &curTempSet)
{
    int lvl = static_cast<int>(curTempSet.size()) / 2 + 1;
    bool isFirst = curTempSet.empty();
    Variants var = variants;
    while (!var.setOfPairs.empty())
    {
        DSet node = var.setOfPairs.front();
        int ro = getRo(var, node);
        if ((isFirst && blockCheckA(ro, uni.size())) ||
            (tmpColors_.empty() && (static_cast<int>(uni.size()) / ro >= maxColors_ || blockCheckB(lvl, static_cast<int>(node.set.size())))) ||
            (tmpColors_.size() + 1 == maxColors_ && !blockCheckC(lvl, uni.size(), ro)) ||
            (isFirst && tmpColors_.empty() && blockCheckD(ro)))
        {
            return;
        }
        std::vector<int> nextTemp = curTempSet;
        nextTemp.push_back(node.i);
        nextTemp.push_back(node.j);
        std::vector<int> nextUni;
        for (int x : uni)
        {
            if (x != node.i && x != node.j &&
                std::find(node.set.begin(), node.set.end(), x) != node.set.end())
            {
                nextUni.push_back(x);
            }
        }
        tmpColors_.push_back(nextTemp);
        build(nextUni, nextTemp);
        tmpColors_.pop_back();
        if (isOver_)
            return;
        var.setOfPairs.erase(var.setOfPairs.begin());
    }
}

void OlemskoyColorGraph::buildEndByCenter(
    const std::vector<int> &uni,
    std::vector<int> &curTempSet)
{
    if (tmpColors_.size() + uni.size() >= maxColors_ || tmpColors_.size() + 1 == maxColors_ - 1)
        return;
    for (int v : uni)
    {
        curTempSet.push_back(v);
        tmpColors_.push_back(curTempSet);
        build({}, curTempSet);
        tmpColors_.pop_back();
        curTempSet.pop_back();
        if (isOver_)
            return;
    }
}

std::vector<int> OlemskoyColorGraph::getPhi(int s) const
{
    assert(!tmpColors_.empty());
    auto phi = tmpColors_.back();
    int numIters = static_cast<int>(phi.size()) - s - 1;
    if (static_cast<int>(phi.size()) == s - 1)
    {
        for (int i = 0; i < numIters; ++i)
            phi.erase(phi.begin(), phi.begin() + 2);
        if (phi.size() % 2 == 1)
            phi.erase(phi.begin());
    }
    return phi;
}

void OlemskoyColorGraph::thinning()
{
    if (tmpColors_.empty())
        return;
    int lastSize = static_cast<int>(tmpColors_.back().size());
    int lastVariants = static_cast<int>(lvlVariants_.size());
    int lastColorLen = lastSize / 2 + lastSize % 2;
    for (int i = 1; i < lastColorLen; ++i)
    {
        auto phi = getPhi(i);
        lvlVariants_[lastVariants - 1 - i].sift(phi);
    }
}

bool OlemskoyColorGraph::coloringIsOver() const
{
    int cnt = 0;
    for (auto &cls : tmpColors_)
        cnt += static_cast<int>(cls.size());
    return cnt == N_;
}

bool OlemskoyColorGraph::checkFirstBlock(const std::vector<int> &block) const
{
    for (auto &wb : watchedFirstBlocks_)
    {
        if (std::all_of(wb.begin(), wb.end(),
                        [&](int x)
                        { return std::find(block.begin(), block.end(), x) != block.end(); }))
        {
            return true;
        }
    }
    return false;
}

void OlemskoyColorGraph::build(const std::vector<int> &uniIn, std::vector<int> &curTempSet)
{
    // GUARD: prevent infinite recursion when both support & temp are empty but not at root
    if (!tmpColors_.empty() && uniIn.empty() && curTempSet.empty())
    {
        std::cout << "[Color] guard: non-root empty call, returning" << std::endl;
        return;
    }
    // DEBUG: build entry
    std::cout << "[Color] depth=" << tmpColors_.size()
              << " uniIn.size=" << uniIn.size()
              << " curTempSet.size=" << curTempSet.size()
              << " maxColors=" << maxColors_ << std::endl;
    if (isOver_)
        return;
    bool firstCall = curTempSet.empty();
    // Refresh support if starting a new color
    std::vector<int> uni = uniIn;
    if (firstCall)
    {
        if (tmpColors_.size() == 1 && checkFirstBlock(tmpColors_[0]))
            return;
        watchedFirstBlocks_.push_back(tmpColors_.empty() ? std::vector<int>{} : tmpColors_[0]);
        uni = createSupportSet();
    }
    // Leaf: no vertices left
    if (uni.empty())
    {
        tmpColors_.push_back(curTempSet);
        thinning();
        if (!coloringIsOver())
        {
            std::vector<int> nextUni = createSupportSet();
            std::vector<int> emptyTemp;
            build(nextUni, emptyTemp);
        }
        else
        {
            if (bestColors_.empty() || tmpColors_.size() < bestColors_.size())
            {
                bestColors_ = tmpColors_;
                maxColors_ = static_cast<int>(bestColors_.size());
            }
        }
        tmpColors_.pop_back();
        return;
    }
    // Interior: generate variants and recurse
    createVariants(uni, firstCall);
    Variants var = lvlVariants_.back();
    if (!var.setOfPairs.empty())
        buildNotNullVariants(var, uni, curTempSet);
    else
        buildEndByCenter(uni, curTempSet);
    lvlVariants_.pop_back();
}
