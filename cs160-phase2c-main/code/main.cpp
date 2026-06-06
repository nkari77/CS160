#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include "graph.h"
#include "bsp.h"
#include "bfs_push_dense_atomic.h"
#include "sssp_push_dense_atomic.h"
#include "cc_push_dense_atomic.h"
#include "bfs_push_sparse_atomic.h"
#include "sssp_push_sparse_atomic.h"
#include "cc_push_sparse_atomic.h"

using namespace std;
using Clock = chrono::high_resolution_clock;
using Sec = chrono::duration<double>;

void verify_bfs(const string& label, const vector<int>& ref, auto& algo, uint32_t n) {
    int errors = 0;
    for (uint32_t i = 0; i < n; i++) {
        if (ref[i] != algo.distance(i)) {
            if (++errors < 5)
                cout << label << " mismatch at " << i << " ref=" << ref[i] << " got=" << algo.distance(i) << endl;
        }
    }
    if (errors == 0) cout << label << " correct" << endl;
    else cout << label << " total mismatches: " << errors << endl;
}

void verify_sssp(const string& label, const vector<long long>& ref, auto& algo, uint32_t n) {
    int errors = 0;
    for (uint32_t i = 0; i < n; i++) {
        if (ref[i] != algo.distance(i)) {
            if (++errors < 5)
                cout << label << " mismatch at " << i << " ref=" << ref[i] << " got=" << algo.distance(i) << endl;
        }
    }
    if (errors == 0) cout << label << " correct" << endl;
    else cout << label << " total mismatches: " << errors << endl;
}

void verify_cc(const string& label, const vector<int>& ref, auto& algo, uint32_t n) {
    int errors = 0;
    for (uint32_t i = 0; i < n; i++) {
        if (ref[i] != algo.component(i)) {
            if (++errors < 5)
                cout << label << " mismatch at " << i << " ref=" << ref[i] << " got=" << algo.component(i) << endl;
        }
    }
    if (errors == 0) cout << label << " correct" << endl;
    else cout << label << " total mismatches: " << errors << endl;
}

int main() {
    CsrGraph g = LoadGraph("soc-LiveJournal1-weighted.txt");
    cout << "loaded: " << g.num_vertices << " vertices" << endl;

    // load answer files
    vector<int> ref_bfs(g.num_vertices, -1);
    vector<long long> ref_sssp(g.num_vertices, -1);
    vector<int> ref_cc(g.num_vertices, 0);

    { ifstream f("BFS.txt"); uint32_t v; int d;
      while (f >> v >> d) ref_bfs[v] = d; }
    { ifstream f("SSSP.txt"); uint32_t v; long long d;
      while (f >> v >> d) ref_sssp[v] = d; }
    { ifstream f("CC.txt"); uint32_t v; int d;
      while (f >> v >> d) ref_cc[v] = d; }

    auto t0 = Clock::now();
    BfsPushDenseAtomic bfs(g.num_vertices, 0, 4);
    BspParallel(g, bfs, 4);
    double t = Sec(Clock::now() - t0).count();
    cout << "BFS dense atomic (4t): " << t << " sec" << endl;
    verify_bfs("BFS dense atomic", ref_bfs, bfs, g.num_vertices);

    t0 = Clock::now();
    SsspPushDenseAtomic sssp(g.num_vertices, 0, 4);
    BspParallel(g, sssp, 4);
    t = Sec(Clock::now() - t0).count();
    cout << "SSSP dense atomic (4t): " << t << " sec" << endl;
    verify_sssp("SSSP dense atomic", ref_sssp, sssp, g.num_vertices);

    t0 = Clock::now();
    CcPushDenseAtomic cc(g.num_vertices, 4);
    BspParallel(g, cc, 4);
    t = Sec(Clock::now() - t0).count();
    cout << "CC dense atomic (4t): " << t << " sec" << endl;
    verify_cc("CC dense atomic", ref_cc, cc, g.num_vertices);

    t0 = Clock::now();
    BfsPushSparseAtomic bfs_s(g.num_vertices, 0, 4);
    bfs_s.Run(g);
    t = Sec(Clock::now() - t0).count();
    cout << "BFS sparse atomic (4t): " << t << " sec" << endl;
    verify_bfs("BFS sparse atomic", ref_bfs, bfs_s, g.num_vertices);

    t0 = Clock::now();
    SsspPushSparseAtomic sssp_s(g.num_vertices, 0, 4);
    sssp_s.Run(g);
    t = Sec(Clock::now() - t0).count();
    cout << "SSSP sparse atomic (4t): " << t << " sec" << endl;
    verify_sssp("SSSP sparse atomic", ref_sssp, sssp_s, g.num_vertices);

    t0 = Clock::now();
    CcPushSparseAtomic cc_s(g.num_vertices, 4);
    cc_s.Run(g);
    t = Sec(Clock::now() - t0).count();
    cout << "CC sparse atomic (4t): " << t << " sec" << endl;
    verify_cc("CC sparse atomic", ref_cc, cc_s, g.num_vertices);

    // ===== roadNet-CA =====
    CsrGraph r = LoadGraphUndirected("roadNet-CA.txt");
    cout << "\nroadNet-CA loaded: " << r.num_vertices << " vertices" << endl;

    t0 = Clock::now();
    BfsPushDenseAtomic r_bfs(r.num_vertices, 0, 4);
    BspParallel(r, r_bfs, 4);
    t = Sec(Clock::now() - t0).count();
    cout << "roadNet BFS dense atomic (4t): " << t << " sec" << endl;

    t0 = Clock::now();
    SsspPushDenseAtomic r_sssp(r.num_vertices, 0, 4);
    BspParallel(r, r_sssp, 4);
    t = Sec(Clock::now() - t0).count();
    cout << "roadNet SSSP dense atomic (4t): " << t << " sec" << endl;

    t0 = Clock::now();
    CcPushDenseAtomic r_cc(r.num_vertices, 4);
    BspParallel(r, r_cc, 4);
    t = Sec(Clock::now() - t0).count();
    cout << "roadNet CC dense atomic (4t): " << t << " sec" << endl;

    t0 = Clock::now();
    BfsPushSparseAtomic r_bfs_s(r.num_vertices, 0, 4);
    r_bfs_s.Run(r);
    t = Sec(Clock::now() - t0).count();
    cout << "roadNet BFS sparse atomic (4t): " << t << " sec" << endl;

    t0 = Clock::now();
    SsspPushSparseAtomic r_sssp_s(r.num_vertices, 0, 4);
    r_sssp_s.Run(r);
    t = Sec(Clock::now() - t0).count();
    cout << "roadNet SSSP sparse atomic (4t): " << t << " sec" << endl;

    t0 = Clock::now();
    CcPushSparseAtomic r_cc_s(r.num_vertices, 4);
    r_cc_s.Run(r);
    t = Sec(Clock::now() - t0).count();
    cout << "roadNet CC sparse atomic (4t): " << t << " sec" << endl;

    return 0;
}