#pragma once
#include <Eigen/Dense>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>

namespace detail {
/* Welsh–Powell greedy (0-based) */
template<class Matrix>
std::vector<int> greedyColor(const Matrix& A)
{
    const int n = A.rows();
    std::vector<std::vector<int>> g(n);
    std::vector<int> deg(n,0);
    for (int i=0;i<n;++i)
        for (int j=0;j<n;++j)
            if (A(i,j)) { g[i].push_back(j); ++deg[i]; }

    std::vector<int> ord(n); std::iota(ord.begin(),ord.end(),0);
    std::sort(ord.begin(),ord.end(),
              [&](int a,int b){ return deg[a]>deg[b]; });

    std::vector<int> col(n,-1), mark(n,0); int stamp=1;
    for (int v:ord)
    {
        for (int u:g[v]) if (col[u]!=-1) mark[col[u]]=stamp;
        int c=0; while (mark[c]==stamp) ++c;
        col[v]=c; ++stamp;
    }
    return col;
}
} // namespace detail


/*---------------------------------------------------------------*
 |  exact DSATUR + Branch-and-Bound                              |
 *---------------------------------------------------------------*/
class BacktrackingColoring
{
public:
    template<class Matrix>
    static std::vector<int> color(const Matrix& A)
    {
        const int n = A.rows();

        /* adjacency + degree */
        std::vector<std::vector<int>> adj(n);
        std::vector<int> deg(n,0);
        for (int i=0;i<n;++i)
            for (int j=0;j<n;++j)
                if (A(i,j)) { adj[i].push_back(j); ++deg[i]; }

        /* greedy upper bound */
        auto greedy = detail::greedyColor(A);
        int UB = *std::max_element(greedy.begin(),greedy.end()) + 1;
        std::vector<int> best = greedy;

        /* DSATUR structures */
        std::vector<int> sat(n,0);              // saturation degree
        std::vector<int> col(n,-1);             // current colouring
        std::vector<std::vector<int>> forbidCnt(n,std::vector<int>(n,0));

        /* recursion */
        std::function<void(int,int)> dfs = [&](int coloured,int used)
        {
            if (used >= UB) return;             // bound

            if (coloured == n)                  // full solution
            {
                UB   = used;
                best = col;
                return;
            }

            /* choose vertex by DSATUR */
            int v=-1, bestSat=-1, bestDeg=-1;
            for (int i=0;i<n;++i) if (col[i]==-1)
                if (sat[i]>bestSat || (sat[i]==bestSat && deg[i]>bestDeg))
                    { v=i; bestSat=sat[i]; bestDeg=deg[i]; }

            /* try existing colours 0 … used-1 */
            for (int c=0;c<used;++c)
            {
                if (forbidCnt[v][c]) continue;

                col[v]=c;
                for (int u:adj[v])
                    if (col[u]==-1)
                        if (forbidCnt[u][c]++ ==0) ++sat[u];

                dfs(coloured+1, used);

                for (int u:adj[v])
                    if (col[u]==-1)
                        if (--forbidCnt[u][c]==0) --sat[u];
                col[v]=-1;
            }

            /* try NEW colour = used (only if it can improve UB) */
            if (used + 1 < UB)
            {
                int c = used;
                col[v]=c;
                for (int u:adj[v])
                    if (col[u]==-1)
                        if (forbidCnt[u][c]++ ==0) ++sat[u];

                dfs(coloured+1, used+1);

                for (int u:adj[v])
                    if (col[u]==-1)
                        if (--forbidCnt[u][c]==0) --sat[u];
                col[v]=-1;
            }
        };

        dfs(0,0);
        return best;           // 0-based (добавьте +1 при выводе, если нужно)
    }
};
