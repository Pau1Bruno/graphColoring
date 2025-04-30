#include "Utils.h"
#include <iostream>
#include "Pair.h"
#include "Variants.h"

// Stream operator for Pair
std::ostream& operator<<(std::ostream& os, const Pair& p) {
    os << "(" << p.left << "," << p.right << ") sup={";
    for (size_t i = 0; i < p.sup.size(); ++i) {
        os << p.sup[i];
        if (i + 1 < p.sup.size()) os << ",";
    }
    os << "}";
    return os;
}

// Stream operator for Variants
std::ostream& operator<<(std::ostream& os, const Variants& v) {
    os << "Variants(size=" << v.setOfPairs.size() << ") { ";
    for (size_t i = 0; i < v.setOfPairs.size(); ++i) {
        os << v.setOfPairs[i];
        if (i + 1 < v.setOfPairs.size()) os << ", ";
    }
    os << " }";
    return os;
}

// Print a list of Variants for debugging
void printVariantsList(const std::vector<Variants>& list) {
    for (size_t i = 0; i < list.size(); ++i) {
        std::cout << "Variant[" << i << "]: " << list[i] << std::endl;
    }
}
