#include "Utils.h"
#include <iostream>
#include "GPair.h"
#include <algorithm>

//-----------------------------------------------------------------------------
// GPair: prints "GPair(i,j) -> { a b c }"
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const GPair& gPair) {
    os << "D(" << gPair.i << "," << gPair.j << ") -> {";
    for (int x : gPair.set) {
        os << " " << x;
    }
    os << " }" << '\n';
    return os;
}

//-----------------------------------------------------------------------------
// vector<GPair>: prints a header then each GPair on its own line
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const std::vector<GPair>& gPairs) {
    os << "[GPairs (size=" << gPairs.size() <<")]\n";
    for (auto const & dg : gPairs) {
        os << "  " << dg;
    }
    return os;
}

//-----------------------------------------------------------------------------
// std::vector<int>: prints vector<int>
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const std::vector<int>& vector) {
    os << "vector<int> = { ";
    for (size_t i = 0; i < vector.size(); ++i) {
        os << " " << vector[i];
        if (i + 1 < vector.size()) os << ",";
    }
    os << " } " << '\n';
    return os;
}

//-----------------------------------------------------------------------------
// std::vector<std::vector<int>>: prints vector<vector<int>>
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<int>>& vectors) {
    os << "vector<int> = { ";
    for (size_t i = 0; i < vectors.size(); ++i) {
        os << " " << vectors[i];
        if (i + 1 < vectors.size()) os << ",";
    }
    os << " } " << '\n';
    return os;
}