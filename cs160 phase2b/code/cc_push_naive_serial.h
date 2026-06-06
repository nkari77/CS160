#pragma once
#include "bsp.h"
#include <vector>
using namespace std;

class CcPushNaiveSerial : public BspAlgorithm {
public:
    vector<int> comp, prev;
    vector<int> updated;
    bool work;

    CcPushNaiveSerial(uint32_t n) {
        comp.resize(n);
        prev.resize(n);
        updated.assign(1, 0);

        for (uint32_t i = 0; i < n; i++) {
            comp[i] = i;
            prev[i] = i;
        }

        work = true;
    }

    bool HasWork() const override { return work; }

    void Process(int tid, uint32_t u, const CsrGraph& g) override {
        // push to out-neighbors
        for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++) {
            uint32_t v = g.edges[i];
            if (prev[u] < comp[v]) {
                comp[v] = prev[u];
                updated[tid] = 1;
            }
        }

        // push to in-neighbors (undirected)
        for (uint32_t i = g.in_offsets[u]; i < g.in_offsets[u+1]; i++) {
            uint32_t v = g.in_edges[i];
            if (prev[u] < comp[v]) {
                comp[v] = prev[u];
                updated[tid] = 1;
            }
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