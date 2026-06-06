#include <iostream>
#include <fstream>
#include <chrono>
#include <climits>
#include <algorithm>
#include "graph.h"
#include "bsp.h"
#include "bfs_push_naive_serial.h"
#include "bfs_push_dense_serial.h"
#include "bfs_push_naive.h"
#include "bfs_push_dense.h"
#include "sssp_push_naive_serial.h"
#include "sssp_push_dense_serial.h"
#include "sssp_push_naive.h"
#include "sssp_push_dense.h"
#include "cc_push_naive_serial.h"
#include "cc_push_dense_serial.h"
#include "cc_push_naive.h"
#include "cc_push_dense.h"

using namespace std;

using Clock = chrono::high_resolution_clock;
using Sec = chrono::duration<double>;

void VerifyBfs(const string& label, const vector<int>& ref, const vector<int>& cmp, uint32_t n) {
    int errors = 0;
    for (uint32_t i = 0; i < n; i++) {
        if (ref[i] != cmp[i]) {
            errors++;
            if (errors < 5)
                cout << label << " mismatch at vertex " << i
                     << " ref=" << ref[i] << " got=" << cmp[i] << endl;
            else if (errors == 5)
                cout << "(further mismatches suppressed)" << endl;
        }
    }
    if (errors == 0) cout << label << " passed" << endl;
    else cout << label << " total mismatches: " << errors << endl;
}

void VerifySssp(const string& label, const vector<long long>& ref, const vector<long long>& cmp, uint32_t n) {
    int errors = 0;
    for (uint32_t i = 0; i < n; i++) {
        if (ref[i] != cmp[i]) {
            errors++;
            if (errors < 5)
                cout << label << " mismatch at vertex " << i
                     << " ref=" << ref[i] << " got=" << cmp[i] << endl;
            else if (errors == 5)
                cout << "(further mismatches suppressed)" << endl;
        }
    }
    if (errors == 0) cout << label << " passed" << endl;
    else cout << label << " total mismatches: " << errors << endl;
}

void VerifyCC(const string& label, const vector<int>& ref, const vector<int>& cmp, uint32_t n) {
    int errors = 0;
    for (uint32_t i = 0; i < n; i++) {
        if (ref[i] != cmp[i]) {
            errors++;
            if (errors < 5)
                cout << label << " mismatch at vertex " << i
                     << " ref=" << ref[i] << " got=" << cmp[i] << endl;
            else if (errors == 5)
                cout << "(further mismatches suppressed)" << endl;
        }
    }
    if (errors == 0) cout << label << " passed" << endl;
    else cout << label << " total mismatches: " << errors << endl;
}

int main() {
    CsrGraph g = LoadGraph("soc-LiveJournal1-weighted.txt");

    // ================= Top 3 in-degrees =================
    vector<pair<uint32_t,uint32_t>> indegrees(g.num_vertices);
    for (uint32_t i = 0; i < g.num_vertices; i++)
        indegrees[i] = {g.in_offsets[i+1] - g.in_offsets[i], i};
    sort(indegrees.rbegin(), indegrees.rend());
    cout << "Top 3 in-degrees:" << endl;
    for (int i = 0; i < 3; i++)
        cout << "  vertex " << indegrees[i].second << " in-degree=" << indegrees[i].first << endl;

    // ================= BFS =================
    double t;

    auto t0 = Clock::now();
    BfsPushNaiveSerial bfs_naive_s(g.num_vertices, 0);
    BspSerial(g, bfs_naive_s);
    double bfs_naive_s_time = Sec(Clock::now() - t0).count();
    cout << "\nBFS serial naive: " << bfs_naive_s_time << " sec (1x baseline)" << endl;

    t0 = Clock::now();
    BfsPushDenseSerial bfs_dense_s(g.num_vertices, 0);
    BspSerial(g, bfs_dense_s);
    t = Sec(Clock::now() - t0).count();
    cout << "BFS serial dense: " << t << " sec  speedup=" << bfs_naive_s_time/t << "x" << endl;

    for (int nt : {1, 4}) {
        t0 = Clock::now();
        BfsPushNaive bfs_naive_p(g.num_vertices, 0, nt);
        BspParallel(g, bfs_naive_p, nt);
        t = Sec(Clock::now() - t0).count();
        cout << "BFS naive parallel (" << nt << "t): " << t << " sec  speedup=" << bfs_naive_s_time/t << "x" << endl;
    }

    for (int nt : {1, 4}) {
        t0 = Clock::now();
        BfsPushDense bfs_dense_p(g.num_vertices, 0, nt);
        BspParallel(g, bfs_dense_p, nt);
        t = Sec(Clock::now() - t0).count();
        cout << "BFS dense parallel (" << nt << "t): " << t << " sec  speedup=" << bfs_naive_s_time/t << "x" << endl;
    }

    BfsPushNaive bfs_naive_p4(g.num_vertices, 0, 4);
    BspParallel(g, bfs_naive_p4, 4);
    BfsPushDense bfs_dense_p4(g.num_vertices, 0, 4);
    BspParallel(g, bfs_dense_p4, 4);

    VerifyBfs("dense serial vs naive serial", bfs_naive_s.dist, bfs_dense_s.dist, g.num_vertices);
    VerifyBfs("naive parallel vs naive serial", bfs_naive_s.dist, bfs_naive_p4.dist, g.num_vertices);
    VerifyBfs("dense parallel vs naive serial", bfs_naive_s.dist, bfs_dense_p4.dist, g.num_vertices);

    vector<int> phase2a_bfs(g.num_vertices, -1);
    ifstream fbfs("expected_BFS.txt");
    uint32_t vid; int d;
    while (fbfs >> vid >> d) phase2a_bfs[vid] = d;
    VerifyBfs("naive serial vs Phase 2a", phase2a_bfs, bfs_naive_s.dist, g.num_vertices);

    ofstream bfs_out("BFS.txt");
    for (uint32_t i = 0; i < g.num_vertices; i++)
        bfs_out << i << " " << bfs_naive_s.dist[i] << "\n";
    cout << "BFS.txt written" << endl;

    // ================= SSSP =================

    t0 = Clock::now();
    SsspPushNaiveSerial sssp_naive_s(g.num_vertices, 0);
    BspSerial(g, sssp_naive_s);
    double sssp_naive_s_time = Sec(Clock::now() - t0).count();
    cout << "\nSSP serial naive: " << sssp_naive_s_time << " sec (1x baseline)" << endl;

    t0 = Clock::now();
    SsspPushDenseSerial sssp_dense_s(g.num_vertices, 0);
    BspSerial(g, sssp_dense_s);
    t = Sec(Clock::now() - t0).count();
    cout << "SSSP serial dense: " << t << " sec  speedup=" << sssp_naive_s_time/t << "x" << endl;

    for (int nt : {1, 4}) {
        t0 = Clock::now();
        SsspPushNaive sssp_naive_p(g.num_vertices, 0, nt);
        BspParallel(g, sssp_naive_p, nt);
        t = Sec(Clock::now() - t0).count();
        cout << "SSSP naive parallel (" << nt << "t): " << t << " sec  speedup=" << sssp_naive_s_time/t << "x" << endl;
    }

    for (int nt : {1, 4}) {
        t0 = Clock::now();
        SsspPushDense sssp_dense_p(g.num_vertices, 0, nt);
        BspParallel(g, sssp_dense_p, nt);
        t = Sec(Clock::now() - t0).count();
        cout << "SSSP dense parallel (" << nt << "t): " << t << " sec  speedup=" << sssp_naive_s_time/t << "x" << endl;
    }

    SsspPushNaive sssp_naive_p4(g.num_vertices, 0, 4);
    BspParallel(g, sssp_naive_p4, 4);
    SsspPushDense sssp_dense_p4(g.num_vertices, 0, 4);
    BspParallel(g, sssp_dense_p4, 4);

    VerifySssp("SSSP dense serial vs naive serial", sssp_naive_s.dist, sssp_dense_s.dist, g.num_vertices);
    VerifySssp("SSSP naive parallel vs naive serial", sssp_naive_s.dist, sssp_naive_p4.dist, g.num_vertices);
    VerifySssp("SSSP dense parallel vs naive serial", sssp_naive_s.dist, sssp_dense_p4.dist, g.num_vertices);

    vector<long long> phase2a_sssp(g.num_vertices, -1);
    ifstream fsssp("expected_SSSP.txt");
    uint32_t vid2; long long d2;
    while (fsssp >> vid2 >> d2) phase2a_sssp[vid2] = d2;
    int sssp_errors = 0;
    for (uint32_t i = 0; i < g.num_vertices; i++) {
        long long got = sssp_naive_s.distance(i);
        if (phase2a_sssp[i] != got) {
            sssp_errors++;
            if (sssp_errors < 5)
                cout << "SSSP naive serial vs Phase 2a mismatch at vertex " << i
                     << " ref=" << phase2a_sssp[i] << " got=" << got << endl;
            else if (sssp_errors == 5)
                cout << "(further mismatches suppressed)" << endl;
        }
    }
    if (sssp_errors == 0) cout << "SSSP naive serial vs Phase 2a passed" << endl;
    else cout << "SSSP naive serial vs Phase 2a total mismatches: " << sssp_errors << endl;

    ofstream sssp_out("SSSP.txt");
    for (uint32_t i = 0; i < g.num_vertices; i++)
        sssp_out << i << " " << sssp_naive_s.distance(i) << "\n";
    cout << "SSSP.txt written" << endl;

    // ================= CC =================

    t0 = Clock::now();
    CcPushNaiveSerial cc_naive_s(g.num_vertices);
    BspSerial(g, cc_naive_s);
    double cc_naive_s_time = Sec(Clock::now() - t0).count();
    cout << "\nCC serial naive: " << cc_naive_s_time << " sec (1x baseline)" << endl;

    t0 = Clock::now();
    CcPushDenseSerial cc_dense_s(g.num_vertices);
    BspSerial(g, cc_dense_s);
    t = Sec(Clock::now() - t0).count();
    cout << "CC serial dense: " << t << " sec  speedup=" << cc_naive_s_time/t << "x" << endl;

    for (int nt : {1, 4}) {
        t0 = Clock::now();
        CcPushNaive cc_naive_p(g.num_vertices, nt);
        BspParallel(g, cc_naive_p, nt);
        t = Sec(Clock::now() - t0).count();
        cout << "CC naive parallel (" << nt << "t): " << t << " sec  speedup=" << cc_naive_s_time/t << "x" << endl;
    }

    for (int nt : {1, 4}) {
        t0 = Clock::now();
        CcPushDense cc_dense_p(g.num_vertices, nt);
        BspParallel(g, cc_dense_p, nt);
        t = Sec(Clock::now() - t0).count();
        cout << "CC dense parallel (" << nt << "t): " << t << " sec  speedup=" << cc_naive_s_time/t << "x" << endl;
    }

    CcPushNaive cc_naive_p4(g.num_vertices, 4);
    BspParallel(g, cc_naive_p4, 4);
    CcPushDense cc_dense_p4(g.num_vertices, 4);
    BspParallel(g, cc_dense_p4, 4);

    VerifyCC("CC dense serial vs naive serial", cc_naive_s.comp, cc_dense_s.comp, g.num_vertices);
    VerifyCC("CC naive parallel vs naive serial", cc_naive_s.comp, cc_naive_p4.comp, g.num_vertices);
    VerifyCC("CC dense parallel vs naive serial", cc_naive_s.comp, cc_dense_p4.comp, g.num_vertices);

    vector<int> phase2a_cc(g.num_vertices, 0);
    ifstream fcc("expected_CC.txt");
    uint32_t vid3; int d3;
    while (fcc >> vid3 >> d3) phase2a_cc[vid3] = d3;
    VerifyCC("CC naive serial vs Phase 2a", phase2a_cc, cc_naive_s.comp, g.num_vertices);

    ofstream cc_out("CC.txt");
    for (uint32_t i = 0; i < g.num_vertices; i++)
        cc_out << i << " " << cc_naive_s.comp[i] << "\n";
    cout << "CC.txt written" << endl;

    return 0;
}