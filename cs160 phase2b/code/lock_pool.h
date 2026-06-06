#pragma once
#include <mutex>
#include <vector>
#include <cstdint>
using namespace std;

// Fixed size pool (template version for normal use)
template<int K = 1024>
class LockPool {
public:
    void lock(uint32_t v)   { locks[v & (K-1)].lock(); }
    void unlock(uint32_t v) { locks[v & (K-1)].unlock(); }
private:
    mutex locks[K];
};

// Runtime configurable pool (for lock pool size experiment)
class LockPoolDynamic {
public:
    LockPoolDynamic(int k) : K(k), locks(k) {}
    void lock(uint32_t v)   { locks[v % K].lock(); }
    void unlock(uint32_t v) { locks[v % K].unlock(); }
private:
    int K;
    vector<mutex> locks;
};