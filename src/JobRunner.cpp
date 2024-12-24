#include "../lib/JobRunner.h"

void JobRunner::processInput(std::istream& input) {
    if (!input) {
            throw std::logic_error("Error: Could not open input.txt");
    }
    
    std::string inputLine;
    getline(input, inputLine);

    if (inputLine != "#job_id,runtime_in_seconds,next_job_id") {
        throw std::logic_error("Missing Header");
    }

    while (getline(input, inputLine)) {
        
      
        if (inputLine.empty() || inputLine[0] == '#') {
            continue;
        }
        std::stringstream ss(inputLine);
        
        int jobId, runtimeSeconds, nextJobId;
        char delimiter1, delimiter2;

        if (!(ss >> jobId >> delimiter1 >> runtimeSeconds >> delimiter2 >> nextJobId)) {
            throw std::logic_error("Log Input Malformed: " + inputLine);
        }

        if (jobId <= 0 || runtimeSeconds < 0 || nextJobId < 0 || delimiter1 != ',' || delimiter2 != ',') {
            throw std::logic_error("Log Input Malformed: " + inputLine);
        }

        JobInfo jobInfo = {static_cast<size_t>(runtimeSeconds), static_cast<size_t>(nextJobId)};
        auto result = logContainer.insert({jobId, std::move(jobInfo)});
        if (!result.second) {
            throw std::logic_error("Duplicate job ID detected: " + std::to_string(jobId));
        }

        if (nextJobId > 0) {
            nextJobSet.insert(nextJobId);
        }
    }
}

void JobRunner::validateAndPrepareJobs() {
    if (logContainer.empty()) {
        throw std::logic_error("No log entry found.");
    }

    for (const auto& [jobId, jobInfo] : logContainer) {
        if (jobInfo.nextJobId != 0 && logContainer.find(jobInfo.nextJobId) == logContainer.end()) {
            throw std::logic_error("Job ID: " + std::to_string(jobId) + 
                                   " referencing non-existing next Job ID: " + 
                                   std::to_string(jobInfo.nextJobId));
        }

        if (nextJobSet.find(jobId) == nextJobSet.end()) {
            startJobs.push_back(jobId);
        }
    }

    if (startJobs.empty()) {
        throw std::logic_error("No independent starting Jobs.");
    }
}

void JobRunner::findChains() {
    for (const int startJob : startJobs) {
        Chain chain;
        chain.totalRuntime = 0;
        size_t currentJob = startJob;

        while (currentJob != 0) {
            const auto& jobInfo = logContainer[currentJob];
            chain.jobIds.push_back(currentJob);
            chain.totalRuntime += jobInfo.runtimeSeconds;
            currentJob = jobInfo.nextJobId;
        }
        chains.push_back(std::move(chain));
    }

    std::sort(chains.begin(), chains.end(), [](const Chain& a, const Chain& b) {
        return a.totalRuntime > b.totalRuntime;
    });
}

void JobRunner::printChains() const {
    for (const auto& chain : chains) {
        std::cout << "-" << std::endl;
        std::cout << "start_job: " << chain.jobIds.front() << std::endl;
        std::cout << "last_job: " << chain.jobIds.back() << std::endl;
        std::cout << "number_of_jobs: " << chain.jobIds.size() << std::endl;
        std::cout << "job_chain_runtime: " << timeFormat(chain.totalRuntime) << std::endl;
        int averageTime = chain.jobIds.size() ? chain.totalRuntime / chain.jobIds.size() : 0;
        std::cout << "average_job_time: " << timeFormat(averageTime) << std::endl;
    }
}