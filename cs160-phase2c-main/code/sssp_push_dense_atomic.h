#pragma once
#include "bsp.h"
#include <vector>
#include <atomic>
#include <climits>
using namespace std;

class SsspPushDenseAtomic : public BspAlgorithm {
public:
    vector<atomic<long long>> dist;
    vector<long long> prev;
    vector<uint8_t> in_frontier, in_next;
    vector<int> updated;
    bool work;

    SsspPushDenseAtomic(uint32_t n, int src, int nt)
        : dist(n), prev(n, LLONG_MAX), in_frontier(n, 0), in_next(n, 0), updated(nt, 0) {
        for (uint32_t i = 0; i < n; i++) dist[i].store(LLONG_MAX);

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
            int w = g.weights[i];
            long long candidate = prev[u] + w;

            long long cur = dist[v].load();
            while (candidate < cur) {
                if (dist[v].compare_exchange_weak(cur, candidate)) {
                    in_next[v] = 1;
                    updated[tid] = 1;
                    break;
                }
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

    long long distance(uint32_t v) const {
        long long d = dist[v].load();
        return (d == LLONG_MAX) ? -1 : d;
    }
};