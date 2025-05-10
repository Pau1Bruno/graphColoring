#pragma once
#include <vector>
#include <iosfwd>

// Forward declarations
struct GPair;

// Stream operators for debugging
std::ostream& operator<<(std::ostream& os, const GPair& gPair);
std::ostream& operator<<(std::ostream& os, const std::vector<GPair>& gPairs);
std::ostream& operator<<(std::ostream& os, const std::vector<int>& intVector);
std::ostream& operator<<(std::ostream& os, const std::vector<std::vector<int>>& intVectors);

