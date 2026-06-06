#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <functional>
#include <queue>
#include <unordered_set>
#include <thread>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <stdexcept>
#include <cassert>

// ============================================================
// Data Structures
// ============================================================

struct CSRGraph {
    int num_vertices;
    std::vector<int> offsets;  // size = num_vertices + 1
    std::vector<int> edges;    // concatenated adjacency lists
    // offsets[i]..offsets[i+1] gives the range of neighbors for vertex i
};

using QueryCallback = std::function<std::string(const CSRGraph&, int src, int K)>;

struct QueryTask {
    int src;
    int K;
    QueryCallback cb;
    std::string result;
    std::string expected;
};

// ============================================================
// Graph Loading
// ============================================================

CSRGraph LoadGraph(const std::string& filepath) {
    std::ifstream fin(filepath);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open graph file: " + filepath);
    }

    // First pass: collect all edges and find max node ID
    std::vector<std::pair<int,int>> edge_list;
    int max_id = -1;
    std::string line;

    while (std::getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        int u, v;
        if (!(iss >> u >> v)) continue;
        edge_list.emplace_back(u, v);
        max_id = std::max(max_id, std::max(u, v));
    }
    fin.close();

    if (max_id < 0) {
        return CSRGraph{0, {0}, {}};
    }

    CSRGraph g;
    g.num_vertices = max_id + 1;
    g.offsets.assign(g.num_vertices + 1, 0);

    // Count out-degrees
    for (auto& [u, v] : edge_list) {
        g.offsets[u + 1]++;
    }
    // Prefix sum
    for (int i = 1; i <= g.num_vertices; i++) {
        g.offsets[i] += g.offsets[i - 1];
    }

    // Fill edges
    g.edges.resize(edge_list.size());
    std::vector<int> pos(g.offsets.begin(), g.offsets.begin() + g.num_vertices);
    for (auto& [u, v] : edge_list) {
        g.edges[pos[u]++] = v;
    }

    std::cerr << "Graph loaded: " << g.num_vertices << " vertices, "
              << g.edges.size() << " edges\n";
    return g;
}

// ============================================================
// K-Hop BFS Core
// ============================================================

// Returns all vertices reachable within exactly K hops from src (excluding src itself).
// Uses BFS with level tracking.
std::vector<int> KHopBFS(const CSRGraph& g, int src, int K) {
    if (src < 0 || src >= g.num_vertices || K <= 0) {
        return {};
    }

    std::vector<bool> visited(g.num_vertices, false);
    visited[src] = true;

    std::vector<int> current_level = {src};
    std::vector<int> reachable;

    for (int hop = 0; hop < K; hop++) {
        std::vector<int> next_level;
        for (int u : current_level) {
            for (int i = g.offsets[u]; i < g.offsets[u + 1]; i++) {
                int v = g.edges[i];
                if (!visited[v]) {
                    visited[v] = true;
                    next_level.push_back(v);
                    reachable.push_back(v);
                }
            }
        }
        current_level = std::move(next_level);
        if (current_level.empty()) break;
    }

    return reachable;
}

// ============================================================
// Query Callbacks
// ============================================================

// Count reachable nodes within K hops (excluding src)
std::string CountCallback(const CSRGraph& g, int src, int K) {
    auto reachable = KHopBFS(g, src, K);
    return std::to_string(static_cast<int>(reachable.size()));
}

// Max node ID among reachable nodes within K hops (excluding src); -1 if none
std::string MaxCallback(const CSRGraph& g, int src, int K) {
    auto reachable = KHopBFS(g, src, K);
    if (reachable.empty()) return "-1";
    int mx = *std::max_element(reachable.begin(), reachable.end());
    return std::to_string(mx);
}

// ============================================================
// Sequential Execution
// ============================================================

void RunTasksSequential(const CSRGraph& g, std::vector<QueryTask>& tasks) {
    for (auto& task : tasks) {
        task.result = task.cb(g, task.src, task.K);
    }
}

// ============================================================
// Parallel Execution
// ============================================================

void RunTasksParallel(const CSRGraph& g, std::vector<QueryTask>& tasks, int num_threads) {
    std::atomic<int> next_task{0};
    int total = static_cast<int>(tasks.size());

    auto worker = [&]() {
        while (true) {
            int idx = next_task.fetch_add(1, std::memory_order_relaxed);
            if (idx >= total) break;
            tasks[idx].result = tasks[idx].cb(g, tasks[idx].src, tasks[idx].K);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(worker);
    }
    for (auto& t : threads) t.join();
}

// ============================================================
// Query File Loading
// ============================================================

std::vector<QueryTask> LoadQueries(const std::string& filepath) {
    std::ifstream fin(filepath);
    if (!fin.is_open()) {
        throw std::runtime_error("Cannot open query file: " + filepath);
    }

    std::vector<QueryTask> tasks;
    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        int src, K, queryType;
        std::string expected;
        if (!(iss >> src >> K >> queryType >> expected)) continue;

        QueryCallback cb = (queryType == 1) ? CountCallback : MaxCallback;
        tasks.push_back({src, K, cb, "", expected});
    }
    return tasks;
}

// ============================================================
// Verification
// ============================================================

bool VerifyResults(const std::vector<QueryTask>& seq_tasks,
                   const std::vector<QueryTask>& par_tasks) {
    bool all_ok = true;
    assert(seq_tasks.size() == par_tasks.size());

    for (size_t i = 0; i < seq_tasks.size(); i++) {
        bool match_expected = (seq_tasks[i].result == seq_tasks[i].expected);
        bool match_parallel  = (seq_tasks[i].result == par_tasks[i].result);

        if (!match_expected) {
            std::cerr << "[FAIL] Query " << i << " (src=" << seq_tasks[i].src
                      << " K=" << seq_tasks[i].K << "): got " << seq_tasks[i].result
                      << ", expected " << seq_tasks[i].expected << "\n";
            all_ok = false;
        }
        if (!match_parallel) {
            std::cerr << "[MISMATCH] Query " << i << ": seq=" << seq_tasks[i].result
                      << " par=" << par_tasks[i].result << "\n";
            all_ok = false;
        }
    }
    return all_ok;
}

// ============================================================
// Main
// ============================================================

int main(int argc, char* argv[]) {
    // Default paths — override via command line args
    std::string graph_file  = (argc > 1) ? argv[1] : "soc-Slashdot0902.txt";
    std::string query_file  = (argc > 2) ? argv[2] : "queries20.txt";
    int num_threads         = (argc > 3) ? std::stoi(argv[3]) : 4;

    std::cout << "Graph file : " << graph_file  << "\n";
    std::cout << "Query file : " << query_file  << "\n";
    std::cout << "Threads    : " << num_threads  << "\n\n";

    // 1. Load graph
    CSRGraph g = LoadGraph(graph_file);

    // 2. Load queries
    auto tasks_template = LoadQueries(query_file);
    std::cout << "Queries loaded: " << tasks_template.size() << "\n\n";

    // 3. Sequential run
    auto seq_tasks = tasks_template;  // copy
    auto t1 = std::chrono::high_resolution_clock::now();
    RunTasksSequential(g, seq_tasks);
    auto t2 = std::chrono::high_resolution_clock::now();
    double seq_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

    // 4. Parallel run
    auto par_tasks = tasks_template;  // copy
    auto t3 = std::chrono::high_resolution_clock::now();
    RunTasksParallel(g, par_tasks, num_threads);
    auto t4 = std::chrono::high_resolution_clock::now();
    double par_ms = std::chrono::duration<double, std::milli>(t4 - t3).count();

    // 5. Verify
    bool ok = VerifyResults(seq_tasks, par_tasks);
    std::cout << "Correctness: " << (ok ? "ALL PASS" : "SOME FAILURES") << "\n\n";

    // 6. Report timing
    std::cout << "Sequential time : " << seq_ms << " ms\n";
    std::cout << "Parallel time   : " << par_ms << " ms\n";
    std::cout << "Speedup         : " << seq_ms / par_ms << "x\n";

    return ok ? 0 : 1;
}
