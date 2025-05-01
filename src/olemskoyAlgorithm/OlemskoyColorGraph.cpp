#include "OlemskoyColorGraph.h"
#include "Utils.h"
#include <numeric>
#include <algorithm>
#include <cassert>
#include <iostream>

OlemskoyColorGraph::OlemskoyColorGraph(const Matrix &adj)
    : N_(adj.rows()), graph_(adj), maxColors_(N_), isOver_(false)
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
    std::iota(mainSupSet_.begin(), mainSupSet_.end(), 1);
    maxColors_ = N_;
    isOver_ = false;
}

void OlemskoyColorGraph::initResult()
{
    // bestColors_ already set
}

int OlemskoyColorGraph::getRo(const Variants &vs, const DSet &d) const
{
    return vs.setOfPairs.empty()
               ? 1
               : std::max(1, static_cast<int>(d.set.size()));
}

std::vector<int> OlemskoyColorGraph::createSupportSet() const
{
    std::vector<bool> used(N_ + 1, false);
    for (auto &cls : tmpColors_)
        for (int v : cls)
            used[v] = true;
    std::vector<int> sup;
    for (int i = 1; i <= N_; ++i)
        if (!used[i])
            sup.push_back(i);

    std::cout << sup;
    return sup;
}

bool OlemskoyColorGraph::blockCheckA(int lenMax, int uniLen) const
{
    return tmpColors_.size() + static_cast<double>(uniLen) / lenMax >= maxColors_;
}
bool OlemskoyColorGraph::blockCheckB(int lvl, int ro) const
{
    return 2 * (lvl - 1) + ro < N_ / maxColors_;
}
bool OlemskoyColorGraph::blockCheckC(int lvl, int uniLen, int ro) const
{
    return 2 * (lvl - 1) + ro == uniLen;
}
bool OlemskoyColorGraph::blockCheckD(int ro) const
{
    int t1 = N_ / ro;
    double t2 = double(N_) / ro;
    return (t1 == t2 ? t1 == maxColors_ : t1 == maxColors_ - 1);
}

void OlemskoyColorGraph::createVariants(const std::vector<int> &uni, bool newC)
{
    Variants v;
    if (!newC)
        v = lvlVariants_.back().sieve(uni);
    else if (!lvlVariants_.empty())
        v = lvlVariants_.front().sieve(uni);
    else
        v = Variants::createFromMatrix(graph_, mainSupSet_);
    lvlVariants_.push_back(std::move(v));
}

void OlemskoyColorGraph::buildNotNullVariants(
    const Variants &variants,
    const std::vector<int> &uni,
    std::vector<int> &curTemp)
{
    int lvl = curTemp.size() / 2 + 1;
    bool isFirst = curTemp.empty();
    Variants var = variants;

    while (!var.setOfPairs.empty())
    {
        DSet d = var.setOfPairs.front();
        int ro = getRo(var, d);
        if ((isFirst && blockCheckA(ro, uni.size())) ||
            (tmpColors_.empty() && (uni.size() / ro >= maxColors_ || blockCheckB(lvl, ro))) ||
            (tmpColors_.size() + 1 == maxColors_ && !blockCheckC(lvl, uni.size(), ro)) ||
            (isFirst && tmpColors_.empty() && blockCheckD(ro)))
            return;

        auto nextTemp = curTemp;
        nextTemp.push_back(d.i);
        nextTemp.push_back(d.j);

        std::vector<int> nextUni;
        for (int x : uni)
            if (x != d.i && x != d.j && std::find(d.set.begin(), d.set.end(), x) != d.set.end())
                nextUni.push_back(x);

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
    std::vector<int> &curTemp)
{
    if (tmpColors_.size() + uni.size() >= maxColors_ ||
        tmpColors_.size() + 1 == maxColors_ - 1)
        return;
    for (int v : uni)
    {
        curTemp.push_back(v);
        tmpColors_.push_back(curTemp);
        build({}, curTemp);
        tmpColors_.pop_back();
        curTemp.pop_back();
        if (isOver_)
            return;
    }
}

std::vector<int> OlemskoyColorGraph::getPhi(int s) const
{
    assert(!tmpColors_.empty());
    auto phi = tmpColors_.back();
    int iters = phi.size() - s - 1;
    if (phi.size() == s - 1)
    {
        for (int i = 0; i < iters; ++i)
            phi.erase(phi.begin(), phi.begin() + 2);
        if (phi.size() % 2)
            phi.erase(phi.begin());
    }
    return phi;
}

void OlemskoyColorGraph::thinning()
{
    if (tmpColors_.empty())
        return;
    int lastSz = tmpColors_.back().size(),
        lvls = lvlVariants_.size(),
        lastLen = lastSz / 2 + lastSz % 2;
    for (int i = 1; i < lastLen; ++i)
        lvlVariants_[lvls - 1 - i].sift(getPhi(i));
}

bool OlemskoyColorGraph::coloringIsOver() const
{
    int sum = 0;
    for (auto &c : tmpColors_)
        sum += c.size();
    return sum == N_;
}

bool OlemskoyColorGraph::checkFirstBlock(const std::vector<int> &b) const
{
    for (auto &wb : watchedFirstBlocks_)
        if (std::all_of(wb.begin(), wb.end(),
                        [&](int x)
                        { return std::find(b.begin(), b.end(), x) != b.end(); }))
            return true;
    return false;
}

void OlemskoyColorGraph::build(const std::vector<int> &uniIn,
                               std::vector<int> &curTemp)
{
    // guard infinite recursion
    if (!tmpColors_.empty() && uniIn.empty() && curTemp.empty())
        return;

    bool first = curTemp.empty();
    auto uni = uniIn;
    if (first)
    {
        if (tmpColors_.size() == 1 && checkFirstBlock(tmpColors_[0]))
            return;
        watchedFirstBlocks_.push_back(tmpColors_.empty() ? std::vector<int>{} : tmpColors_[0]);
        uni = createSupportSet();
    }

    if (uni.empty())
    {
        tmpColors_.push_back(curTemp);
        thinning();
        if (!coloringIsOver())
        {
            auto next = createSupportSet();
            std::vector<int> empty;
            build(next, empty);
        }
        else if (bestColors_.empty() || tmpColors_.size() < bestColors_.size())
        {
            bestColors_ = tmpColors_;
            maxColors_ = bestColors_.size();
        }
        tmpColors_.pop_back();
        return;
    }

    createVariants(uni, first);
    auto var = lvlVariants_.back();
    if (!var.setOfPairs.empty())
        buildNotNullVariants(var, uni, curTemp);
    else
        buildEndByCenter(uni, curTemp);
    lvlVariants_.pop_back();
}