#include <iostream>
#include <fstream>
#include "graph.h"
#include "bsp.h"
#include "bfs.h"
#include "sssp.h"
#include "cc.h"

using namespace std;

int main() {

    CsrGraph g = LoadGraph("soc-LiveJournal1-weighted.txt");

    // -------------------------------
    // BFS
    // -------------------------------
    Bfs bfs_serial(g.num_vertices, 0, 1);
    BspSerial(g, bfs_serial);

    Bfs bfs_parallel(g.num_vertices, 0, 4);
    BspParallel(g, bfs_parallel, 4);

    int errors = 0;
    for (uint32_t i = 0; i < g.num_vertices; i++) {
        if (bfs_serial.dist[i] != bfs_parallel.dist[i]) {
            errors++;
            if (errors < 5) {
                cout << "BFS mismatch at vertex " << i
                     << " serial=" << bfs_serial.dist[i]
                     << " parallel=" << bfs_parallel.dist[i]
                     << endl;
            } else if (errors == 5) {
                cout << "(further mismatches suppressed)" << endl;
            }
        }
    }
    if (errors == 0) {
        cout << "BFS serial vs parallel match ✔" << endl;
    } else {
        cout << "BFS total mismatches: " << errors << endl;
    }

    ofstream bfs_out("BFS.txt");
    for (uint32_t i = 0; i < g.num_vertices; i++)
        bfs_out << i << " " << bfs_serial.dist[i] << "\n";
    cout << "BFS.txt written" << endl;

    // -------------------------------
    // SSSP
    // -------------------------------
    Sssp sssp_serial(g.num_vertices, 0, 1);
    BspSerial(g, sssp_serial);

    Sssp sssp_parallel(g.num_vertices, 0, 4);
    BspParallel(g, sssp_parallel, 4);

    errors = 0;
    for (uint32_t i = 0; i < g.num_vertices; i++) {
        long long s = sssp_serial.distance(i);
        long long p = sssp_parallel.distance(i);
        if (s != p) {
            errors++;
            if (errors < 5) {
                cout << "SSSP mismatch at vertex " << i
                     << " serial=" << s
                     << " parallel=" << p
                     << endl;
            } else if (errors == 5) {
                cout << "(further mismatches suppressed)" << endl;
            }
        }
    }
    if (errors == 0) {
        cout << "SSSP serial vs parallel match ✔" << endl;
    } else {
        cout << "SSSP total mismatches: " << errors << endl;
    }

    ofstream sssp_out("SSSP.txt");
    for (uint32_t i = 0; i < g.num_vertices; i++)
        sssp_out << i << " " << sssp_serial.distance(i) << "\n";
    cout << "SSSP.txt written" << endl;

    // -------------------------------
    // CC
    // -------------------------------
    CC cc_serial(g.num_vertices, 1);
    BspSerial(g, cc_serial);

    CC cc_parallel(g.num_vertices, 4);
    BspParallel(g, cc_parallel, 4);

    errors = 0;
    for (uint32_t i = 0; i < g.num_vertices; i++) {
        if (cc_serial.comp[i] != cc_parallel.comp[i]) {
            errors++;
            if (errors < 5) {
                cout << "CC mismatch at vertex " << i
                     << " serial=" << cc_serial.comp[i]
                     << " parallel=" << cc_parallel.comp[i]
                     << endl;
            } else if (errors == 5) {
                cout << "(further mismatches suppressed)" << endl;
            }
        }
    }
    if (errors == 0) {
        cout << "CC serial vs parallel match ✔" << endl;
    } else {
        cout << "CC total mismatches: " << errors << endl;
    }

    ofstream cc_out("CC.txt");
    for (uint32_t i = 0; i < g.num_vertices; i++)
        cc_out << i << " " << cc_serial.comp[i] << "\n";
    cout << "CC.txt written" << endl;

    return 0;
}
