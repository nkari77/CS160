#include "graph.h"
#include <fstream>
#include <sstream>
#include <tuple>
#include <algorithm>
#include <random>
using namespace std;

CsrGraph LoadGraph(const char* filename) {
    ifstream file(filename);
    vector<tuple<uint32_t,uint32_t,int32_t>> edge_list;
    uint32_t max_v = 0;
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        uint32_t u, v; int32_t w;
        if (!(ss >> u >> v >> w)) continue;
        edge_list.emplace_back(u, v, w);
        max_v = max(max_v, max(u, v));
    }
    uint32_t n = max_v + 1;
    CsrGraph g;
    g.num_vertices = n;
    g.offsets.assign(n+1, 0);
    g.in_offsets.assign(n+1, 0);
    for (auto& e : edge_list) {
        g.offsets[get<0>(e)+1]++;
        g.in_offsets[get<1>(e)+1]++;
    }
    for (uint32_t i = 1; i <= n; i++) {
        g.offsets[i]    += g.offsets[i-1];
        g.in_offsets[i] += g.in_offsets[i-1];
    }
    g.edges.resize(edge_list.size());
    g.weights.resize(edge_list.size());
    g.in_edges.resize(edge_list.size());
    g.in_weights.resize(edge_list.size());
    vector<uint32_t> cur_out = g.offsets;
    vector<uint32_t> cur_in  = g.in_offsets;
    for (auto& e : edge_list) {
        uint32_t src = get<0>(e);
        uint32_t dst = get<1>(e);
        int32_t  wt  = get<2>(e);
        uint32_t pos   = cur_out[src]++;
        g.edges[pos]   = dst;
        g.weights[pos] = wt;
        uint32_t pos2      = cur_in[dst]++;
        g.in_edges[pos2]   = src;
        g.in_weights[pos2] = wt;
    }
    return g;
}

CsrGraph LoadGraphUndirected(const char* filename) {
    ifstream file(filename);
    vector<tuple<uint32_t,uint32_t,int32_t>> edge_list;
    uint32_t max_v = 0;
    mt19937 rng(42);
    uniform_int_distribution<int32_t> wdist(1, 400);
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        uint32_t u, v;
        if (!(ss >> u >> v)) continue;
        int32_t w = wdist(rng);
        edge_list.emplace_back(u, v, w);
        edge_list.emplace_back(v, u, w);
        max_v = max(max_v, max(u, v));
    }
    uint32_t n = max_v + 1;
    CsrGraph g;
    g.num_vertices = n;
    g.offsets.assign(n+1, 0);
    g.in_offsets.assign(n+1, 0);
    for (auto& e : edge_list) {
        g.offsets[get<0>(e)+1]++;
        g.in_offsets[get<1>(e)+1]++;
    }
    for (uint32_t i = 1; i <= n; i++) {
        g.offsets[i]    += g.offsets[i-1];
        g.in_offsets[i] += g.in_offsets[i-1];
    }
    g.edges.resize(edge_list.size());
    g.weights.resize(edge_list.size());
    g.in_edges.resize(edge_list.size());
    g.in_weights.resize(edge_list.size());
    vector<uint32_t> cur_out = g.offsets;
    vector<uint32_t> cur_in  = g.in_offsets;
    for (auto& e : edge_list) {
        uint32_t src = get<0>(e);
        uint32_t dst = get<1>(e);
        int32_t  wt  = get<2>(e);
        uint32_t pos   = cur_out[src]++;
        g.edges[pos]   = dst;
        g.weights[pos] = wt;
        uint32_t pos2      = cur_in[dst]++;
        g.in_edges[pos2]   = src;
        g.in_weights[pos2] = wt;
    }
    return g;
}