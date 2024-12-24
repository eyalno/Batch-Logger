#ifndef CHAIN_H
#define CHAIN_H

#include <vector>
#include <cstddef>

struct Chain {
    std::vector<size_t> jobIds;  // List of job IDs in the chain
    size_t totalRuntime;         // Total runtime of all jobs in the chain
};

#endif // CHAIN_H