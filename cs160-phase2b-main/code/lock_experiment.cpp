#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>
#include <cstdint>
#include "graph.h"
#include "bsp.h"
#include <climits>

using namespace std;
using Clock = chrono::high_resolution_clock;
using Sec = chrono::duration<double>;

// Dynamic lock pool with runtime K
class DynLockPool {
public:
    DynLockPool(int k) : K(k), locks(k) {}
    void lock(uint32_t v)   { locks[v % K].lock(); }
    void unlock(uint32_t v) { locks[v % K].unlock(); }
private:
    int K;
    vector<mutex> locks;
};

// BFS dense parallel with dynamic lock pool
class BfsDynK : public BspAlgorithm {
public:
    vector<int> dist, prev;
    vector<uint8_t> in_frontier, in_next;
    vector<int> updated;
    DynLockPool pool;
    bool work;

    BfsDynK(uint32_t n, int src, int nt, int K)
        : pool(K) {
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
        for (int x : updated) { if (x) { work = true; break; } }
        updated.assign(updated.size(), 0);
        prev = dist;
        swap(in_frontier, in_next);
        fill(in_next.begin(), in_next.end(), 0);
    }
};

// SSSP dense parallel with dynamic lock pool
class SsspDynK : public BspAlgorithm {
public:
    vector<long long> dist, prev;
    vector<uint8_t> in_frontier, in_next;
    vector<int> updated;
    DynLockPool pool;
    bool work;

    SsspDynK(uint32_t n, int src, int nt, int K)
        : pool(K) {
        dist.assign(n, LLONG_MAX);
        prev.assign(n, LLONG_MAX);
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
            int w = g.weights[i];
            long long candidate = prev[u] + w;
            pool.lock(v);
            if (dist[v] == LLONG_MAX || candidate < dist[v]) {
                dist[v] = candidate;
                in_next[v] = 1;
                updated[tid] = 1;
            }
            pool.unlock(v);
        }
    }

    void PostRound() override {
        work = false;
        for (int x : updated) { if (x) { work = true; break; } }
        updated.assign(updated.size(), 0);
        prev = dist;
        swap(in_frontier, in_next);
        fill(in_next.begin(), in_next.end(), 0);
    }
};

// CC dense parallel with dynamic lock pool
class CcDynK : public BspAlgorithm {
public:
    vector<int> comp, prev;
    vector<uint8_t> in_frontier, in_next;
    vector<int> updated;
    DynLockPool pool;
    bool work;

    CcDynK(uint32_t n, int nt, int K)
        : pool(K) {
        comp.resize(n);
        prev.resize(n);
        in_frontier.assign(n, 1);
        in_next.assign(n, 0);
        updated.assign(nt, 0);
        for (uint32_t i = 0; i < n; i++) { comp[i] = i; prev[i] = i; }
        work = true;
    }

    bool HasWork() const override { return work; }

    void Process(int tid, uint32_t u, const CsrGraph& g) override {
        if (!in_frontier[u]) return;
        for (uint32_t i = g.offsets[u]; i < g.offsets[u+1]; i++) {
            uint32_t v = g.edges[i];
            pool.lock(v);
            if (prev[u] < comp[v]) { comp[v] = prev[u]; in_next[v] = 1; updated[tid] = 1; }
            pool.unlock(v);
        }
        for (uint32_t i = g.in_offsets[u]; i < g.in_offsets[u+1]; i++) {
            uint32_t v = g.in_edges[i];
            pool.lock(v);
            if (prev[u] < comp[v]) { comp[v] = prev[u]; in_next[v] = 1; updated[tid] = 1; }
            pool.unlock(v);
        }
    }

    void PostRound() override {
        work = false;
        for (int x : updated) { if (x) { work = true; break; } }
        updated.assign(updated.size(), 0);
        prev = comp;
        swap(in_frontier, in_next);
        fill(in_next.begin(), in_next.end(), 0);
    }
};

int main() {
    CsrGraph g = LoadGraph("soc-LiveJournal1-weighted.txt");

    for (int K : {1024, 16384, 262144}) {
        cout << "\nK = " << K << endl;

        auto t0 = Clock::now();
        BfsDynK bfs(g.num_vertices, 0, 4, K);
        BspParallel(g, bfs, 4);
        cout << "BFS dense parallel 4t: " << Sec(Clock::now() - t0).count() << " sec" << endl;

        t0 = Clock::now();
        SsspDynK sssp(g.num_vertices, 0, 4, K);
        BspParallel(g, sssp, 4);
        cout << "SSSP dense parallel 4t: " << Sec(Clock::now() - t0).count() << " sec" << endl;

        t0 = Clock::now();
        CcDynK cc(g.num_vertices, 4, K);
        BspParallel(g, cc, 4);
        cout << "CC dense parallel 4t: " << Sec(Clock::now() - t0).count() << " sec" << endl;
    }

    return 0;
}
