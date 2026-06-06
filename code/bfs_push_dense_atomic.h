#pragma once
#include "bsp.h"
#include <vector>
#include <atomic>
using namespace std;

class BfsPushDenseAtomic : public BspAlgorithm {
public:
    vector<atomic<int>> dist;
    vector<int> prev;
    vector<uint8_t> in_frontier, in_next;
    vector<int> updated;
    bool work;

    BfsPushDenseAtomic(uint32_t n, int src, int nt) 
        : dist(n), prev(n, -1), in_frontier(n, 0), in_next(n, 0), updated(nt, 0) {
        for (uint32_t i = 0; i < n; i++) dist[i].store(-1);

        dist[src].store(0);
        prev[src] = 0;
        in_frontier[src] = 1;
        work = true;
    }

    bool HasWork() const override { return work; }

    void Process(int tid, uint32_t u, const CsrGraph& g) override {
        if (!in_frontier[u]) return;

        for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++) {
            uint32_t v = g.edges[i];
            int candidate = prev[u] + 1;
            int expected = -1;
            if (dist[v].compare_exchange_strong(expected, candidate)) {
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
        for (uint32_t i = 0; i < prev.size(); i++) prev[i] = dist[i].load();
        swap(in_frontier, in_next);
        fill(in_next.begin(), in_next.end(), 0);
    }

    int distance(uint32_t v) const { return dist[v].load(); }
};