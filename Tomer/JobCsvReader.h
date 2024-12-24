#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <memory>

struct JobStatistic
{
	int job_id{}, last_job_id{}, total{}, count{};
	int Average() const { return total / count; }
};

using JobStatistics = std::vector<JobStatistic>;

struct JobDetail
{
	JobDetail(size_t jobid, size_t jobtime, size_t jobnext)
		: job_id{ jobid },job_time{ jobtime }, job_next{ jobnext } { }

	size_t job_id, job_time, job_next, chain_id{}, chain_sequence_number{};
};
using JobDetailPointer = std::shared_ptr<JobDetail>;
using JobDetailPointers = std::vector<JobDetailPointer>;

namespace Utilities::String
{
	std::vector<std::string> Split(std::string str, char delimeter);
	static size_t ToNumber(const std::string& str);
}

class JobCsvReader
{
public:

	static constexpr const char* MalformedInputErrorMessage = "Malformed Input";
	static constexpr const char* Header = "#job_id,runtime_in_seconds,next_job_id";

	static JobStatistics ComputeJobStatistics();

	static std::string FormatReport(JobStatistics job_details);

protected:

	struct TimeConversions
	{
		static const int seconds_per_minute = 60;
		static const int minutes_per_hour = 60;
	};

private:
	static JobDetailPointers ReadJobs();
	static void PopulateJobStatistic(JobDetailPointer job_detail_root, std::unordered_map<int, JobDetailPointer> job_details, JobStatistic& out_statistic);
	static bool BuildChain(JobDetailPointer job_detail, size_t job_next, std::unordered_map<int, JobDetailPointer> job_details, const int chain_id, int chain_sequence_number);
	static std::string ToTimestamp(int total_seconds);
};

