#pragma once
#include <vector>
#include <iosfwd>

// Forward declarations
struct DSet;
class Variants;

// Stream operators for debugging
std::ostream& operator<<(std::ostream& os, const DSet& dset);
std::ostream& operator<<(std::ostream& os, const Variants& v);
std::ostream& operator<<(std::ostream& os, const std::vector<DSet>& vsets);
std::ostream& operator<<(std::ostream& os, const std::vector<Variants>& vlist);