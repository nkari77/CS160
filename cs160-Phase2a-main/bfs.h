#pragma once

#include "bsp.h"

#include <vector>
#include <cstdint>

using namespace std;


// BFS
class Bfs : public BspAlgorithm {
public:
    vector<int32_t> dist, prev;
    vector<int>     updated;
    bool            work;

    Bfs(uint32_t n, int src, int nt) {
        dist.assign(n, -1);
        prev.assign(n, -1);
        updated.assign(nt, 0);

        dist[src] = 0;
        prev[src] = 0;
        work = true;
    }

    bool HasWork() const override { return work; }

    void Process(int tid, uint32_t v, const CsrGraph& g) override {
        // Scan ALL in-neighbors and take the minimum hop distance,
        // not just the first reachable one.
        int32_t best      = dist[v];
        bool    updated_v = false;

        for (uint32_t i = g.in_offsets[v]; i < g.in_offsets[v+1]; i++) {
            uint32_t u = g.in_edges[i];
            if (prev[u] == -1) continue;
            int32_t candidate = prev[u] + 1;
            if (best == -1 || candidate < best) {
                best      = candidate;
                updated_v = true;
            }
        }

        if (updated_v) {
            dist[v]      = best;
            updated[tid] = 1;
        }
    }

    void PostRound() override {
        work = false;
        for (int x : updated) {
            if (x) { work = true; break; }
        }
        updated.assign(updated.size(), 0);
        prev = dist;
    }
};
