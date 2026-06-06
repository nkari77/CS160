#pragma once
#include "bsp.h"
#include <vector>
using namespace std;

class CcPushDenseSerial : public BspAlgorithm {
public:
    vector<int> comp, prev;
    vector<uint8_t> in_frontier, in_next;
    vector<int> updated;
    bool work;

    CcPushDenseSerial(uint32_t n) {
        comp.resize(n);
        prev.resize(n);
        in_frontier.assign(n, 1); // all vertices start in frontier
        in_next.assign(n, 0);
        updated.assign(1, 0);

        for (uint32_t i = 0; i < n; i++) {
            comp[i] = i;
            prev[i] = i;
        }

        work = true;
    }

    bool HasWork() const override { return work; }

    void Process(int tid, uint32_t u, const CsrGraph& g) override {
        if (!in_frontier[u]) return;

        for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++) {
            uint32_t v = g.edges[i];
            if (prev[u] < comp[v]) {
                comp[v] = prev[u];
                in_next[v] = 1;
                updated[tid] = 1;
            }
        }

        for (uint32_t i = g.in_offsets[u]; i < g.in_offsets[u+1]; i++) {
            uint32_t v = g.in_edges[i];
            if (prev[u] < comp[v]) {
                comp[v] = prev[u];
                in_next[v] = 1;
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
        swap(in_frontier, in_next);
        fill(in_next.begin(), in_next.end(), 0);
    }

    int component(uint32_t v) const { return comp[v]; }
};