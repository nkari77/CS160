#pragma once
#include "bsp.h"
#include "lock_pool.h"
#include <vector>
using namespace std;

class BfsPushDense : public BspAlgorithm {
public:
    vector<int> dist, prev;
    vector<uint8_t> in_frontier, in_next;
    vector<int> updated;
    LockPool<1024> pool;
    bool work;

    BfsPushDense(uint32_t n, int src, int nt) {
        dist.assign(n, -1);
        prev.assign(n, -1);
        in_frontier.assign(n, 0);
        in_next.assign(n, 0);
        updated.assign(nt, 0);

        dist[src] = 0;
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

            pool.lock(v);
            if (dist[v] == -1 || candidate < dist[v]) {
                dist[v] = candidate;
                in_next[v] = 1;
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
        swap(in_frontier, in_next);
        fill(in_next.begin(), in_next.end(), 0);
    }

    int distance(uint32_t v) const { return dist[v]; }
};