#pragma once
#include "bsp.h"
#include <vector>
#include <climits>
using namespace std;

class SsspPull : public BspAlgorithm {
public:
    vector<long long> dist, prev;
    vector<int> updated;
    bool work;

    SsspPull(uint32_t n, int src, int nt) {
        dist.assign(n, LLONG_MAX);
        prev.assign(n, LLONG_MAX);
        updated.assign(nt, 0);
        dist[src] = 0;
        prev[src] = 0;
        work = true;
    }

    bool HasWork() const override { return work; }

    void Process(int tid, uint32_t v, const CsrGraph& g) override {
        long long best = dist[v];
        for (uint32_t i = g.in_offsets[v]; i < g.in_offsets[v+1]; i++) {
            uint32_t u = g.in_edges[i];
            int w = g.in_weights[i];
            if (prev[u] != LLONG_MAX) {
                long long candidate = prev[u] + w;
                if (best == LLONG_MAX || candidate < best)
                    best = candidate;
            }
        }
        if (best < dist[v]) {
            dist[v] = best;
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

    long long distance(uint32_t v) const {
        return (dist[v] == LLONG_MAX) ? -1 : dist[v];
    }
};