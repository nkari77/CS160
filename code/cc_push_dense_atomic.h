#pragma once
#include "bsp.h"
#include <vector>
#include <atomic>
using namespace std;

class CcPushDenseAtomic : public BspAlgorithm {
public:
    vector<atomic<int>> comp;
    vector<int> prev;
    vector<uint8_t> in_frontier, in_next;
    vector<int> updated;
    bool work;

    CcPushDenseAtomic(uint32_t n, int nt)
        : comp(n), prev(n), in_frontier(n, 1), in_next(n, 0), updated(nt, 0) {
        for (uint32_t i = 0; i < n; i++) {
            comp[i].store(i);
            prev[i] = i;
        }
        work = true;
    }

    bool HasWork() const override { return work; }

    void Process(int tid, uint32_t u, const CsrGraph& g) override {
        if (!in_frontier[u]) return;

        auto push = [&](uint32_t v) {
            int candidate = prev[u];
            int cur = comp[v].load();
            while (candidate < cur) {
                if (comp[v].compare_exchange_weak(cur, candidate)) {
                    in_next[v] = 1;
                    updated[tid] = 1;
                    break;
                }
            }
        };

        for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++)
            push(g.edges[i]);
        for (uint32_t i = g.in_offsets[u]; i < g.in_offsets[u+1]; i++)
            push(g.in_edges[i]);
    }

    void PostRound() override {
        work = false;
        for (int x : updated) {
            if (x) { work = true; break; }
        }
        updated.assign(updated.size(), 0);
        for (uint32_t i = 0; i < prev.size(); i++) prev[i] = comp[i].load();
        swap(in_frontier, in_next);
        fill(in_next.begin(), in_next.end(), 0);
    }

    int component(uint32_t v) const { return comp[v].load(); }
};