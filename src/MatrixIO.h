#pragma once
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <stdexcept>

/*  Данные одного графа из файла */
struct DenseGraph
{
    int             n        = 0;         // порядок графа
    double          density  = 0.0;       // если был параметр d=
    Eigen::MatrixXi A;                    // n×n, 0/1
};

/*------------------------------------------------------------------------
   Утилита: убрать комментарий «// ...» и пробелы по краям
  ----------------------------------------------------------------------*/
static inline std::string strip(const std::string& s)
{
    auto pos = s.find("//");
    std::string t = pos==std::string::npos ? s : s.substr(0,pos);

    size_t L = 0, R = t.size();
    while (L<R && std::isspace(static_cast<unsigned char>(t[L]))) ++L;
    while (R> L && std::isspace(static_cast<unsigned char>(t[R-1]))) --R;
    return t.substr(L,R-L);
}

/*------------------------------------------------------------------
   service:  оставить в строке только символы, допустимые в числах
 ------------------------------------------------------------------*/
static inline void keepOnlyNumberChars(std::string& s)
{
    auto ok = [](char ch) {
        return std::isdigit(static_cast<unsigned char>(ch)) ||
               ch=='+' || ch=='-' || ch=='.' ||
               ch=='e' || ch=='E' || ch==' ';
    };
    for (char& ch : s)
        if (!ok(ch)) ch = ' ';        // всё остальное → пробел
}

/*------------------------------------------------------------------------
   Прочитать все графы из файла `fileName`
  ----------------------------------------------------------------------*/
inline std::vector<DenseGraph> loadGraphs(const std::string& fileName)
{
    std::ifstream fin(fileName);
    if (!fin) throw std::runtime_error("Cannot open "+fileName);

    std::vector<DenseGraph> graphs;
    std::string line;

    while (true)
    {
        /* ---------- ждём первой строки с  n=  ------------------------ */
        DenseGraph g;
        while (std::getline(fin,line))
        {
            line = strip(line);
            if (line.empty() || line[0]=='-') continue;    // пропускаем ------
            if (line.rfind("n",0)==0) break;               // нашли n=
        }
        if (!fin) break;                                   // EOF

        /* ---------- читаем параметры до matrix: ----------------------- */
        do {
            line = strip(line);
            if (line.empty()) continue;

            if (line.rfind("n",0)==0) {
                g.n = std::stoi(line.substr(line.find('=')+1));
            }
            else if (line.rfind("d",0)==0) {
                g.density = std::stod(line.substr(line.find('=')+1));
            }
            else if (line.find("matrix")!=std::string::npos) {
                break;                                     // переходим к матрице
            }
        } while (std::getline(fin,line));

        if (g.n<=0) throw std::runtime_error("Bad or missing n= in "+fileName);

        /* ---------- считываем матрицу n×n ---------------------------- */
        g.A = Eigen::MatrixXi::Zero(g.n,g.n);
        const double EPS = 1e-12;

        int read = 0;
        while (read < g.n*g.n && std::getline(fin,line))
        {
            line = strip(line);
            if (line.empty()) continue;
            keepOnlyNumberChars(line);                      // ← функция-фильтр

            std::istringstream iss(line);
            double val;
            while (iss >> val)
            {
                int r = read / g.n,  c = read % g.n;
                g.A(r,c) = (std::abs(val) < EPS ? 0 : 1);
                if (++read == g.n*g.n) break;
            }
        }
        if (read != g.n*g.n)
            throw std::runtime_error("Matrix size mismatch in "+fileName);

        graphs.push_back(std::move(g));

        /* ---------- после матрицы просто продолжаем цикл ------------- */
        /*      будь то строка “-----” или сразу “n= …” ―
                внешний while снова всё обработает                      */
    }
    return graphs;
}

/*------------------------------------------------------------
 *  writeMatrix("start_matrix.txt", G.A);
 *-----------------------------------------------------------*/
inline void writeMatrix(const std::string& fileName,
                        const Eigen::MatrixXi& M)
{
    std::ofstream fout(fileName);
    if (!fout) throw std::runtime_error("Cannot open " + fileName);

    const int n = M.rows();
    fout << "n = " << n << '\n';
    fout << "matrix:\n";

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
            fout << std::setw(2) << M(i,j) << ' ';
        fout << '\n';
    }
}