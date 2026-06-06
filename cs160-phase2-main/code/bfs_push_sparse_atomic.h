#pragma once
#include "graph.h"
#include <vector>
#include <atomic>
#include <thread>
#include <cstring>
using namespace std;

class BfsPushSparseAtomic {
public:
    vector<atomic<int>> dist;
    int nthreads;

    BfsPushSparseAtomic(uint32_t n, int src, int nt)
        : dist(n), nthreads(nt) {
        for (uint32_t i = 0; i < n; i++) dist[i].store(-1);
        dist[src].store(0);
    }

    void Run(const CsrGraph& g) {
        vector<uint32_t> cur = {0};
        vector<vector<uint32_t>> next_t(nthreads);
        int round = 0;

        while (!cur.empty()) {
            cout << "BFS sparse round " << round++ << " |F|=" << cur.size() << endl;
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

            uint32_t total = 0;
            for (auto& buf : next_t) total += buf.size();
            cur.clear();
            cur.reserve(total);
            for (auto& buf : next_t)
                cur.insert(cur.end(), buf.begin(), buf.end());
        }
    }

    int distance(uint32_t v) const { return dist[v].load(); }
};