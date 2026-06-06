#pragma once

#include "graph.h"

// -------------------------------
// BSP Base
// -------------------------------
class BspAlgorithm {
public:
    virtual ~BspAlgorithm() = default;
    virtual bool HasWork() const = 0;
    virtual void Process(int tid, uint32_t v, const CsrGraph& g) = 0;
    virtual void PostRound() = 0;
};

void BspSerial(const CsrGraph& g, BspAlgorithm& algo);
void BspParallel(const CsrGraph& g, BspAlgorithm& algo, int nthreads);
