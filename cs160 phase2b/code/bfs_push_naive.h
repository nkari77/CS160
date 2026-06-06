#pragma once
#include "bsp.h"
#include "lock_pool.h"
#include <vector>
using namespace std;

class BfsPushNaive : public BspAlgorithm {
public:
    vector<int> dist, prev;
    vector<int> updated;
    LockPool<1024> pool;
    bool work;

    BfsPushNaive(uint32_t n, int src, int nt) {
        dist.assign(n, -1);
        prev.assign(n, -1);
        updated.assign(nt, 0);

        dist[src] = 0;
        prev[src] = 0;
        work = true;
    }

    bool HasWork() const override { return work; }

    void Process(int tid, uint32_t u, const CsrGraph& g) override {
        if (prev[u] == -1) return;

        for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++) {
            uint32_t v = g.edges[i];
            int candidate = prev[u] + 1;

            pool.lock(v);
            if (dist[v] == -1 || candidate < dist[v]) {
                dist[v] = candidate;
                updated[tid] = 1;
            }
            pool.unlock(v);
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

    int distance(uint32_t v) const { return dist[v]; }
};