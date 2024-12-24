#ifndef JOB_RUNNER_H
#define JOB_RUNNER_H

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>
#include "time_utils.h"
#include "job_info.h"
#include "Chain.h"

class JobRunner {
public:
    void processInput(std::istream& input);
    void validateAndPrepareJobs();
    void findChains();
    void printChains() const;

private:
    std::unordered_map<int, JobInfo> logContainer;
    std::unordered_set<int> nextJobSet;
    std::vector<int> startJobs;
    std::vector<Chain> chains;
};

#endif // JOB_RUNNER_H