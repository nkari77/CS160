#include "bsp.h"

#include <vector>
#include <algorithm>
#include <thread>

using namespace std;

void BspSerial(const CsrGraph& g, BspAlgorithm& algo) {
    while (algo.HasWork()) {
        for (uint32_t v = 0; v < g.num_vertices; v++) {
            algo.Process(0, v, g);
        }
        algo.PostRound();
    }
}

void BspParallel(const CsrGraph& g, BspAlgorithm& algo, int nthreads) {
    while (algo.HasWork()) {
        vector<thread> threads;

        // Ceiling division so no vertices are dropped when
        // num_vertices is not evenly divisible by nthreads
        uint32_t chunk = (g.num_vertices + nthreads - 1) / nthreads;

        for (int t = 0; t < nthreads; t++) {
            uint32_t start = (uint32_t)t * chunk;
            uint32_t end   = min(start + chunk, g.num_vertices);
            if (start >= g.num_vertices) break;

            threads.emplace_back([&, t, start, end]() {
                for (uint32_t v = start; v < end; v++) {
                    algo.Process(t, v, g);
                }
            });
        }

        for (auto& th : threads) th.join();
        algo.PostRound();
    }
}
