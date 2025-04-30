#include "OlemskoyColorGraph.h"
#include <algorithm>
#include <numeric>

// Constructor: set up and run
OlemskoyColorGraph::OlemskoyColorGraph(const Matrix& adj)
    : N_(static_cast<int>(adj.rows())), graph_(adj), maxColors_(N_), isOver_(false)
{
    init();
    if (!mainSupSet_.empty()) {
        std::vector<int> emptySet;
        build(mainSupSet_, emptySet);
        initResult();
    }
}

void OlemskoyColorGraph::init() {
    bestColors_.clear();
    tmpColors_.clear();
    watchedFirstBlocks_.clear();
    lvlVariants_.clear();
    mainSupSet_.resize(N_);
    std::iota(mainSupSet_.begin(), mainSupSet_.end(), 0);
    maxColors_ = N_;
    isOver_ = false;
}

void OlemskoyColorGraph::initResult() {
    // bestColors_ already set during build
}

bool OlemskoyColorGraph::coloringIsOver() const {
    int sum = 0;
    for (auto &c : tmpColors_) sum += (int)c.size();
    return sum == N_;
}

bool OlemskoyColorGraph::blockCheckA(int lenMax, int uniLen) const {
    double val = tmpColors_.size() + double(uniLen) / lenMax;
    return val >= maxColors_;
}

bool OlemskoyColorGraph::blockCheckB(int curLvl, int ro) const {
    return 2*(curLvl-1) + ro < N_ / maxColors_;
}

bool OlemskoyColorGraph::blockCheckC(int curLvl, int uniLen, int ro) const {
    return 2*(curLvl-1) + ro == uniLen;
}

bool OlemskoyColorGraph::blockCheckD(int ro) const {
    int tmp1 = N_ / ro;
    double tmp2 = double(N_) / ro;
    if (tmp1 == tmp2) return tmp1 == maxColors_;
    return tmp1 == maxColors_ - 1;
}

int OlemskoyColorGraph::getRo(const Variants& vs, const Pair& p) const {
    if (vs.setOfPairs.empty()) return 1;
    int tmp = (int)p.sup.size();
    return tmp != 0 ? tmp : 1;
}

void OlemskoyColorGraph::createVariants(const std::vector<int>& uni, bool newColor) {
    Variants result = newColor
        ? (lvlVariants_.empty()
            ? Variants::createFromMatrix(graph_, mainSupSet_)
            : lvlVariants_.front().sieve(uni))
        : lvlVariants_.back().sieve(uni);
    // order descending support size
    std::sort(result.setOfPairs.begin(), result.setOfPairs.end(),
              [](auto &a, auto &b){ return a.sup.size() > b.sup.size(); });
    lvlVariants_.push_back(std::move(result));
}

void OlemskoyColorGraph::buildNotNullVariants(const Variants& variants,
                                              const std::vector<int>& uni,
                                              std::vector<int>& curTempSet)
{
    int lvl = tmpColors_.size();
    bool isFirst = (lvl == 0);
    Variants var = variants;
    while (!var.setOfPairs.empty()) {
        const Pair node = var.setOfPairs.front();
        int ro = getRo(var, node);
        if ((isFirst && blockCheckA(ro, uni.size())) ||
            (!isFirst && tmpColors_.empty() && (uni.size()/ro >= maxColors_ || blockCheckB(lvl, (int)node.sup.size()))) ||
            (tmpColors_.size()+1 == maxColors_-1 && !blockCheckC(lvl, uni.size(), ro)) ||
            (isFirst && tmpColors_.empty() && blockCheckD(ro))) {
            return;
        }
        // branch with this pair
        curTempSet.push_back(node.left);
        curTempSet.push_back(node.right);
        std::vector<int> nextUni;
        for (int x: uni) if (std::find(node.sup.begin(), node.sup.end(), x)!=node.sup.end() && x!=node.left && x!=node.right)
            nextUni.push_back(x);
        tmpColors_.push_back(curTempSet);
        build(nextUni, curTempSet);
        tmpColors_.pop_back();
        // remove this pair and continue
        var.setOfPairs.erase(var.setOfPairs.begin());
    }
}

void OlemskoyColorGraph::buildEndByCenter(const std::vector<int>& uni,
                                          std::vector<int>& curTempSet)
{
    if (tmpColors_.size() + uni.size() >= maxColors_ || tmpColors_.size()+1 == maxColors_-1)
        return;
    for (int v : uni) {
        curTempSet.push_back(v);
        tmpColors_.push_back(curTempSet);
        build({}, curTempSet);
        tmpColors_.pop_back();
        curTempSet.pop_back();
        if (isOver_) return;
    }
}

void OlemskoyColorGraph::thinning() {
    if (tmpColors_.empty()) return;
    int lastSize = (int)tmpColors_.back().size();
    int lastVariants = (int)lvlVariants_.size();
    int lastColorLen = lastSize/2 + lastSize%2;
    for (int i=1; i<lastColorLen; ++i) {
        // phi not implemented in detail; skip deep thinning
        lvlVariants_[lastVariants-1-i].sift(tmpColors_.back());
    }
}

void OlemskoyColorGraph::build(const std::vector<int>& uni,
                               std::vector<int>&       curTempSet)
{
    if (isOver_) return;
    // base: no uni
    if (uni.empty()) {
        tmpColors_.push_back(curTempSet);
        thinning();
        if (!coloringIsOver()) {
            std::vector<int> nextUni;
            // rebuild support: elements not in any tmpColors_
            std::vector<bool> inColor(N_, false);
            for (auto &c: tmpColors_) for (int v:c) inColor[v]=true;
            for (int i=0;i<N_;++i) if (!inColor[i]) nextUni.push_back(i);
            {
        std::vector<int> newTemp;
        build(nextUni, newTemp);
    }
        } else {
            if (bestColors_.empty() || tmpColors_.size() < bestColors_.size()) {
                bestColors_ = tmpColors_;
                maxColors_ = (int)bestColors_.size();
            }
        }
        tmpColors_.pop_back();
        return;
    }
    // create and branch
    createVariants(uni, curTempSet.empty());
    const Variants& var = lvlVariants_.back();
    if (!var.setOfPairs.empty()) {
        buildNotNullVariants(var, uni, curTempSet);
    } else {
        buildEndByCenter(uni, curTempSet);
    }
    lvlVariants_.pop_back();
}
