#pragma once

#include <vector>

// Represents a non-edge between two vertices and a support set
struct DSet {
    int i;
    int j;
    std::vector<int> set; // support vertices
};