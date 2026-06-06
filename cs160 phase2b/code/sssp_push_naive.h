#pragma once
#include "bsp.h"
#include "lock_pool.h"
#include <vector>
#include <climits>
using namespace std;

class SsspPushNaive : public BspAlgorithm {
public:
    vector<long long> dist, prev;
    vector<int> updated;
    LockPool<1024> pool;
    bool work;

    SsspPushNaive(uint32_t n, int src, int nt) {
        dist.assign(n, LLONG_MAX);
        prev.assign(n, LLONG_MAX);
        updated.assign(nt, 0);

        dist[src] = 0;
        prev[src] = 0;
        work = true;
    }

    bool HasWork() const override { return work; }

    void Process(int tid, uint32_t u, const CsrGraph& g) override {
        if (prev[u] == LLONG_MAX) return;

        for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++) {
            uint32_t v = g.edges[i];
            int w = g.weights[i];
            long long candidate = prev[u] + w;

            pool.lock(v);
            if (dist[v] == LLONG_MAX || candidate < dist[v]) {
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

    long long distance(uint32_t v) const {
        return (dist[v] == LLONG_MAX) ? -1 : dist[v];
    }
};