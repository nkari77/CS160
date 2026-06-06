#pragma once

#include <vector>
#include <cstdint>
#include <string>

using namespace std;

// -------------------------------
// CSR Graph
// -------------------------------
struct CsrGraph {
    uint32_t num_vertices;
    vector<uint32_t> offsets, edges;
    vector<int32_t> weights;

    vector<uint32_t> in_offsets, in_edges;
    vector<int32_t> in_weights;
};

CsrGraph LoadGraph(const char* filename);
CsrGraph LoadGraphUndirected(const char* filename);