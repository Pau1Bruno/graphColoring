#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <numeric>

namespace greedy {

/* ---------- результат ---------- */
struct ColoringResult {
    int chromaticNumber = 0;                 // χ
    std::unordered_map<int,int> colorOf;     // v → цвет (1-based)
};

/* ---------- основной класс ----- */
template<class Matrix>
class Coloring
{
public:
    explicit Coloring(const Matrix& M, int maxColor = 10000)
        : n_(M.rows()), maxColor_(maxColor)
    {
        buildAdjacency(M);
        calculate();
    }

    int  chromaticNumber()  const { return result_.chromaticNumber; }
    const std::unordered_map<int,int>& colors() const { return result_.colorOf; }

    /* статический шорт-кат, если объект класс не нужен */
    static ColoringResult run(const Matrix& M, int maxColor = 1000)
    {
        return Coloring(M, maxColor).result_;
    }

private:
    /* ---------- данные ---------- */
    int n_;                                    // |V|
    int maxColor_;
    std::vector<std::vector<int>> adj_;        // списки смежности
    ColoringResult result_;

    /* ---------- строим adj из матрицы ---- */
    void buildAdjacency(const Matrix& A) {
        adj_.assign(n_, {});
        for (int i = 0; i < n_; ++i)
            for (int j = i + 1; j < n_; ++j)
                if (A(i,j)) {              
                    adj_[i].push_back(j);
                    adj_[j].push_back(i);
                }
    }

    /* ---------- главный цикл ------------- */
    void calculate() {
        std::vector<int> base(n_);
        std::iota(base.begin(), base.end(), 0);

        std::vector<std::vector<int>> orders;
        addSimpleAndRandomOrders(orders, base);
        addBasedOnDegree(orders);
        addForTree(orders);

        int best = maxColor_;
        for (const auto& ord : orders) {
            auto cur = makeColoring(ord);
            if (cur.chromaticNumber < best) {
                best   = cur.chromaticNumber;
                result_ = std::move(cur);
            }
        }
    }

    /* ---------- жадная раскраска ---------- */
    ColoringResult makeColoring(const std::vector<int>& order) const {
        ColoringResult res;
        int bestColor = 0;
        for (int v : order) {
            std::unordered_map<int,bool> used;
            for (int nb : adj_[v])
                if (res.colorOf.count(nb)) used[ res.colorOf.at(nb) ] = true;

            int c = 1; while (used.count(c)) ++c;
            res.colorOf[v] = c;
            bestColor = std::max(bestColor, c);
        }
        res.chromaticNumber = bestColor;
        return res;
    }

    /* ---------- генерация порядков -------- */
    void addSimpleAndRandomOrders(std::vector<std::vector<int>>& lst,
                                  const std::vector<int>& base) const {
        lst.push_back(base);                     // исходный
        std::vector<int> tmp(base);
        std::mt19937 rng(std::random_device{}());
        int cnt = static_cast<int>(std::sqrt(base.size()));
        for (int i = 0; i < cnt; ++i) {
            std::shuffle(tmp.begin(), tmp.end(), rng);
            lst.push_back(tmp);
        }
    }

    void addBasedOnDegree(std::vector<std::vector<int>>& lst) const {
        struct VD { int v; int deg; };
        std::vector<VD> vd;
        for (int v = 0; v < n_; ++v) vd.push_back({v, static_cast<int>(adj_[v].size())});
        std::sort(vd.begin(), vd.end(), [](auto&a,auto&b){ return a.deg>b.deg; });
        std::vector<int> order; for (auto& t:vd) order.push_back(t.v);
        lst.push_back(order);

        auto shLittle = order;
        for (size_t i=0;i+1<shLittle.size();i+=2) std::swap(shLittle[i], shLittle[i+1]);
        lst.push_back(shLittle);

        int n = order.size();
        if (n>1) {
            std::mt19937 rng(std::random_device{}());
            auto half = order;
            int mid = n/2;
            std::shuffle(half.begin(), half.begin()+mid, rng);
            std::shuffle(half.begin()+mid, half.end(), rng);
            lst.push_back(half);

            int extra = static_cast<int>(std::sqrt(n));
            for (int t=0;t<extra;++t) {
                int pivot = 1 + rng()%(n-2);
                auto tmp = order;
                std::shuffle(tmp.begin(), tmp.begin()+pivot, rng);
                std::shuffle(tmp.begin()+pivot, tmp.end(), rng);
                lst.push_back(tmp);
            }
        }
    }

    void addForTree(std::vector<std::vector<int>>& lst) const {
        struct VD { int v; int deg; };
        std::vector<VD> vd;
        for (int v=0; v<n_; ++v) vd.push_back({v, static_cast<int>(adj_[v].size())});
        std::sort(vd.begin(), vd.end(), [](auto&a,auto&b){return a.deg>b.deg;});
        std::vector<int> vdOrder; for(auto&x:vd) vdOrder.push_back(x.v);

        auto bfsGen=[&](bool pickMax){
            std::vector<int> res;
            std::vector<char> added(n_,0);
            for(int start: vdOrder){
                if(added[start]) continue;
                added[start]=1; res.push_back(start);
                std::vector<int> q{start};
                while(true){
                    bool progress=false;
                    size_t qs=q.size();
                    for(size_t qi=0;qi<qs;++qi){
                        int v=q[qi];
                        int bestNb=-1, bestDeg=-1;
                        for(int nb: adj_[v]) if(!added[nb]){
                            if(!pickMax){ bestNb=nb; break; }
                            if(static_cast<int>(adj_[nb].size())>bestDeg){bestDeg=adj_[nb].size(); bestNb=nb;}
                        }
                        if(bestNb!=-1){ added[bestNb]=1; q.push_back(bestNb); res.push_back(bestNb); progress=true; break;}  
                    }
                    if(!progress) break;
                }
            }
            lst.push_back(res);
        };
        bfsGen(true);  // вариация с «max-degree neighbor»
    }
};

}
