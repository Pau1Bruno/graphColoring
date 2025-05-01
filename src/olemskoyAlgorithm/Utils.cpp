#include "Utils.h"
#include <iostream>
#include "DSet.h"
#include "Variants.h"

#include "Utils.h"
#include <algorithm>

//-----------------------------------------------------------------------------
// DSet: prints "D(i,j) -> { a b c }"
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const DSet& dset) {
    os << "D(" << dset.i << "," << dset.j << ") -> {";
    for (int x : dset.set) {
        os << " " << x;
    }
    os << " }" << '\n';
    return os;
}

//-----------------------------------------------------------------------------
// Variants: prints "[Variants size=N] (p1, p2, ...)"
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const Variants& v) {
    os << "[Variants size=" << v.setOfPairs.size() << "] {";
    for (size_t i = 0; i < v.setOfPairs.size(); ++i) {
        os << " " << v.setOfPairs[i];
        if (i + 1 < v.setOfPairs.size()) os << ",";
    }
    os << " }" << '\n';
    return os;
}

//-----------------------------------------------------------------------------
// vector<DSet>: prints a header then each DSet on its own line
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const std::vector<DSet>& vsets) {
    os << "D‐sets:\n";
    for (auto const & ds : vsets) {
        os << "  " << ds << "\n";
    }
    return os;
}

//-----------------------------------------------------------------------------
// std::vector<Variants>: prints each Variants on its own line
//-----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const std::vector<Variants>& vlist) {
    os << "Variants‐list (size=" << vlist.size() << "):\n";
    for (size_t i = 0; i < vlist.size(); ++i) {
        os << " [" << i << "] " << vlist[i] << "\n";
    }
    return os;
}