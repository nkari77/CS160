#pragma once
#include "bsp.h"
#include <vector>
#include <climits>
using namespace std;

class SsspPushDenseSerial : public BspAlgorithm {
public:
    vector<long long> dist, prev;
    vector<uint8_t> in_frontier, in_next;
    vector<int> updated;
    bool work;

    SsspPushDenseSerial(uint32_t n, int src) {
        dist.assign(n, LLONG_MAX);
        prev.assign(n, LLONG_MAX);
        in_frontier.assign(n, 0);
        in_next.assign(n, 0);
        updated.assign(1, 0);

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
            int w = g.weights[i];
            long long candidate = prev[u] + w;

            if (dist[v] == LLONG_MAX || candidate < dist[v]) {
                dist[v] = candidate;
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
        prev = dist;
        swap(in_frontier, in_next);
        fill(in_next.begin(), in_next.end(), 0);
    }

    long long distance(uint32_t v) const {
        return (dist[v] == LLONG_MAX) ? -1 : dist[v];
    }
};