#pragma once
#include "graph.h"
#include <vector>
#include <atomic>
#include <thread>
#include <climits>
using namespace std;

class SsspPushSparseAtomic {
public:
    vector<atomic<long long>> dist;
    vector<atomic<int>> in_next;
    int nthreads;

    SsspPushSparseAtomic(uint32_t n, int src, int nt)
        : dist(n), in_next(n), nthreads(nt) {
        for (uint32_t i = 0; i < n; i++) {
            dist[i].store(LLONG_MAX);
            in_next[i].store(0);
        }
        dist[src].store(0);
    }

    void Run(const CsrGraph& g) {
        vector<uint32_t> cur = {0};
        vector<vector<uint32_t>> next_t(nthreads);
        int round = 0;

        while (!cur.empty()) {
            cout << "SSSP sparse round " << round++ << " |F|=" << cur.size() << endl;
            for (auto& buf : next_t) buf.clear();

            uint32_t sz = cur.size();
            uint32_t chunk = (sz + nthreads - 1) / nthreads;

            vector<thread> threads;
            for (int t = 0; t < nthreads; t++) {
                uint32_t start = (uint32_t)t * chunk;
                uint32_t end   = min(start + chunk, sz);
                if (start >= sz) break;

                threads.emplace_back([&, t, start, end]() {
                    for (uint32_t idx = start; idx < end; idx++) {
                        uint32_t u = cur[idx];
                        long long du = dist[u].load();
                        if (du == LLONG_MAX) continue;

                        for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++) {
                            uint32_t v = g.edges[i];
                            int w = g.weights[i];
                            long long candidate = du + w;

                            long long cur_val = dist[v].load();
                            while (candidate < cur_val) {
                                if (dist[v].compare_exchange_weak(cur_val, candidate)) {
                                    int expected = 0;
                                    if (in_next[v].compare_exchange_strong(expected, 1))
                                        next_t[t].push_back(v);
                                    break;
                                }
                            }
                        }
                    }
                });
            }
            for (auto& th : threads) th.join();

            for (auto& buf : next_t)
                for (uint32_t v : buf)
                    in_next[v].store(0);

            uint32_t total = 0;
            for (auto& buf : next_t) total += buf.size();
            cur.clear();
            cur.reserve(total);
            for (auto& buf : next_t)
                cur.insert(cur.end(), buf.begin(), buf.end());
        }
    }

    long long distance(uint32_t v) const {
        long long d = dist[v].load();
        return (d == LLONG_MAX) ? -1 : d;
    }
};