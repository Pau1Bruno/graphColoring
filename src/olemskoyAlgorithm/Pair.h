#pragma once

#include <vector>

// Represents a non-edge between two vertices and a support set
struct Pair {
    int left;
    int right;
    std::vector<int> sup; // support vertices
};