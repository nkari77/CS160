#pragma once
#include "graph.h"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;

struct Delay {
    size_t before_index;
    int    ms;
};

uint32_t find_max_vertex(const string& initial, const string& updates) {
    uint32_t maxv = 0;
    uint32_t src, dst; int w;

    ifstream f1(initial);
    string line;
    while (getline(f1, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        ss >> src >> dst >> w;
        if (src > maxv) maxv = src;
        if (dst > maxv) maxv = dst;
    }

    ifstream f2(updates);
    while (getline(f2, line)) {
        if (line.empty() || line[0] == '#' || line[0] == 'D') continue;
        istringstream ss(line);
        string tag; ss >> tag;
        ss >> src >> dst >> w;
        if (src > maxv) maxv = src;
        if (dst > maxv) maxv = dst;
    }
    return maxv;
}

Graph load_initial(const string& path, uint32_t maxv) {
    ifstream f(path);

    Graph g(maxv + 1);
    string line;
    while (getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        uint32_t src, dst; int w;
        ss >> src >> dst >> w;
        g.adj[src].push_back({dst, w});
    }
    return g;
}

void parse_updates(const string& path,
                   vector<AddRequest>& adds,
                   vector<Delay>& delays)
{
    ifstream f(path);
    string line;
    while (getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        istringstream ss(line);
        string tag; ss >> tag;
        if (tag == "ADD") {
            uint32_t s, d; int wt;
            ss >> s >> d >> wt;
            adds.push_back({s, d, wt});
        } else if (tag == "D") {
            int ms; ss >> ms;
            delays.push_back({adds.size(), ms});
        }
    }
}