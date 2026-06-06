#pragma once
#include "graph.h"
#include <vector>
#include <atomic>
#include <thread>
using namespace std;

class BfsBonus {
public:
    vector<atomic<int>> dist;
    vector<int> prev;
    int nthreads;
    uint32_t n;

    BfsBonus(uint32_t n, int src, int nt)
        : dist(n), prev(n, -1), nthreads(nt), n(n) {
        for (uint32_t i = 0; i < n; i++) dist[i].store(-1);
        dist[src].store(0);
        prev[src] = 0;
    }

    void Run(const CsrGraph& g) {
        vector<uint32_t> cur = {0};
        vector<vector<uint32_t>> next_t(nthreads);

        while (!cur.empty()) {
            for (auto& buf : next_t) buf.clear();

            if (cur.size() > n / 4) {
                // pull round
                vector<int> updated(nthreads, 0);
                uint32_t chunk = (n + nthreads - 1) / nthreads;
                vector<thread> threads;
                vector<uint32_t> next_cur;

                for (int t = 0; t < nthreads; t++) {
                    uint32_t start = (uint32_t)t * chunk;
                    uint32_t end = min(start + chunk, n);
                    if (start >= n) break;
                    threads.emplace_back([&, t, start, end]() {
                        for (uint32_t v = start; v < end; v++) {
                            if (dist[v].load() != -1) continue;
                            for (uint32_t i = g.in_offsets[v]; i < g.in_offsets[v+1]; i++) {
                                uint32_t u = g.in_edges[i];
                                if (prev[u] == -1) continue;
                                int candidate = prev[u] + 1;
                                int expected = -1;
                                if (dist[v].compare_exchange_strong(expected, candidate)) {
                                    next_t[t].push_back(v);
                                    updated[t] = 1;
                                    break;
                                }
                            }
                        }
                    });
                }
                for (auto& th : threads) th.join();

            } else {
                // sparse push round
                uint32_t sz = cur.size();
                uint32_t chunk = (sz + nthreads - 1) / nthreads;
                vector<thread> threads;

                for (int t = 0; t < nthreads; t++) {
                    uint32_t start = (uint32_t)t * chunk;
                    uint32_t end = min(start + chunk, sz);
                    if (start >= sz) break;
                    threads.emplace_back([&, t, start, end]() {
                        for (uint32_t idx = start; idx < end; idx++) {
                            uint32_t u = cur[idx];
                            int du = dist[u].load();
                            for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++) {
                                uint32_t v = g.edges[i];
                                int expected = -1;
                                if (dist[v].compare_exchange_strong(expected, du + 1))
                                    next_t[t].push_back(v);
                            }
                        }
                    });
                }
                for (auto& th : threads) th.join();
            }

            // update prev and concat next_t into cur
            uint32_t total = 0;
            for (auto& buf : next_t) total += buf.size();
            cur.clear();
            cur.reserve(total);
            for (auto& buf : next_t) {
                for (uint32_t v : buf) prev[v] = dist[v].load();
                cur.insert(cur.end(), buf.begin(), buf.end());
            }
        }
    }

    int distance(uint32_t v) const { return dist[v].load(); }
};