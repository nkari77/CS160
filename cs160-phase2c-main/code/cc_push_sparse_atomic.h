#pragma once
#include "graph.h"
#include <vector>
#include <atomic>
#include <thread>
using namespace std;

class CcPushSparseAtomic {
public:
    vector<atomic<int>> comp;
    vector<atomic<int>> in_next;
    int nthreads;

    CcPushSparseAtomic(uint32_t n, int nt)
        : comp(n), in_next(n), nthreads(nt) {
        for (uint32_t i = 0; i < n; i++) {
            comp[i].store(i);
            in_next[i].store(0);
        }
    }

    void Run(const CsrGraph& g) {
        vector<uint32_t> cur;
        cur.resize(g.num_vertices);
        for (uint32_t i = 0; i < g.num_vertices; i++) cur[i] = i;

        vector<vector<uint32_t>> next_t(nthreads);
        int round = 0;

        while (!cur.empty()) {
            cout << "CC sparse round " << round++ << " |F|=" << cur.size() << endl;
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
                        int cu = comp[u].load();

                        auto push = [&](uint32_t v) {
                            int cur_val = comp[v].load();
                            while (cu < cur_val) {
                                if (comp[v].compare_exchange_weak(cur_val, cu)) {
                                    int expected = 0;
                                    if (in_next[v].compare_exchange_strong(expected, 1))
                                        next_t[t].push_back(v);
                                    break;
                                }
                            }
                        };

                        for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++)
                            push(g.edges[i]);
                        for (uint32_t i = g.in_offsets[u]; i < g.in_offsets[u+1]; i++)
                            push(g.in_edges[i]);
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

    int component(uint32_t v) const { return comp[v].load(); }
};