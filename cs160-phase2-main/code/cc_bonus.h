#pragma once
#include "bsp.h"
#include <vector>
using namespace std;

class CcPull : public BspAlgorithm {
public:
    vector<int> comp, prev;
    vector<int> updated;
    bool work;

    CcPull(uint32_t n, int nt) {
        comp.resize(n);
        prev.resize(n);
        updated.assign(nt, 0);
        for (uint32_t i = 0; i < n; i++) {
            comp[i] = i;
            prev[i] = i;
        }
        work = true;
    }

    bool HasWork() const override { return work; }

    void Process(int tid, uint32_t v, const CsrGraph& g) override {
        int best = prev[v];
        for (uint32_t i = g.in_offsets[v]; i < g.in_offsets[v+1]; i++)
            best = min(best, prev[g.in_edges[i]]);
        for (uint32_t i = g.offsets[v]; i < g.offsets[v+1]; i++)
            best = min(best, prev[g.edges[i]]);
        if (best < comp[v]) {
            comp[v] = best;
            updated[tid] = 1;
        }
    }

    void PostRound() override {
        work = false;
        for (int x : updated) {
            if (x) { work = true; break; }
        }
        updated.assign(updated.size(), 0);
        prev = comp;
    }

    int component(uint32_t v) const { return comp[v]; }
};