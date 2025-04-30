#pragma once
#include <vector>
#include <iosfwd>

// Forward declarations
class Pair;
class Variants;

// Stream operators for debugging
std::ostream& operator<<(std::ostream& os, const Pair& p);
std::ostream& operator<<(std::ostream& os, const Variants& v);

// Print a list of Variants for debugging
void printVariantsList(const std::vector<Variants>& list);
