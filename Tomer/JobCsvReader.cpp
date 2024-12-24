#include "JobCsvReader.h"

namespace Utilities::String
{
	std::vector<std::string> Split(std::string str, char delimeter)
	{
		std::vector<std::string> values{};
		size_t start_index{}, end_index{};
		while ((end_index = str.find(delimeter, start_index)) && end_index != std::string::npos)
		{
			auto distance = end_index - start_index;
			values.push_back(str.substr(start_index, distance));
			start_index = end_index + 1;
		}
		values.push_back(str.substr(start_index));
		return values;
	}

	size_t ToNumber(const std::string& str)
	{
		return std::accumulate(str.begin(), str.end(), 0, [](int sum, char character)
			{
				if (!std::isdigit(character))
					throw std::logic_error("Unknown character.");

				return sum * 10 + character - '0';
			});
	}
}

JobDetailPointers JobCsvReader::ReadJobs()
{
	JobDetailPointers job_details{};
	std::string whole_line{};

	if (!std::getline(std::cin, whole_line) || whole_line != Header)
		throw std::logic_error("First line doesn't exist, or header is malformed.");

	using namespace Utilities::String;

	while (std::getline(std::cin, whole_line))
	{
		auto seperated_line = Split(whole_line, ',');
		if (seperated_line.size() != 3)
			throw std::logic_error("Too many or too few columns.");

		size_t job_id = ToNumber(seperated_line[0]),
			job_length = ToNumber(seperated_line[1]),
			job_next = ToNumber(seperated_line[2]);

		if (!job_id)
			throw std::logic_error("0 is reserved as a termination job_id number.");
		if (job_id == job_next)
			throw std::logic_error("Job cannot be self-referential.");

		job_details.push_back(std::make_shared<JobDetail>(job_id, job_length, job_next));
	}

	return job_details;
}

JobStatistics JobCsvReader::ComputeJobStatistics()
{
	auto job_details = ReadJobs();

	JobStatistics job_statistics;
	job_statistics.reserve(job_details.size());

	std::unordered_map<int, JobDetailPointer> job_roots{};
	std::unordered_map<int, JobDetailPointer> all_jobs{};
	job_roots.reserve(job_details.size());
	all_jobs.reserve(job_details.size());

	std::for_each(job_details.begin(), job_details.end(), [&](const JobDetailPointer& job_detail) 
		{ 
			job_roots.insert({ job_detail->job_id, job_detail }); 
			auto [_, success] = all_jobs.insert({ job_detail->job_id, job_detail });
			if (!success)
				throw std::logic_error("Duplicate job detected.");
		});
	std::for_each(job_details.begin(), job_details.end(), [&job_roots](const JobDetailPointer& job_detail) 
		{ 
			if (job_detail->job_next && !job_roots.erase(job_detail->job_next))
				throw std::logic_error("Non-null terminator (0) unrecognized next job."); 
		});

	if (job_roots.empty())
		throw std::logic_error("No root jobs. Cannot create chains.");

	int chain_id{1};
	const int chain_sequence_id{ 1 };
	for (auto& [_, root_detail] : job_roots)
	{
		root_detail->chain_id = chain_id++;
		root_detail->chain_sequence_number = chain_sequence_id;

		auto root_valid = BuildChain(root_detail, root_detail->job_next, all_jobs, root_detail->chain_id, chain_sequence_id);
		if (!root_valid)
			throw std::logic_error("Invalid root. Job chain is not terminated.");

		JobStatistic job_statistic;
		PopulateJobStatistic(root_detail, all_jobs, job_statistic);
		job_statistics.push_back(job_statistic);
	}

	return job_statistics;
}

void JobCsvReader::PopulateJobStatistic(JobDetailPointer job_detail_root, std::unordered_map<int, JobDetailPointer> all_jobs, JobStatistic& out_statistic)
{
	if (!job_detail_root->job_id)
		return;

	out_statistic.count++;
	out_statistic.total += job_detail_root->job_time;
	if (!out_statistic.job_id)
		out_statistic.job_id = job_detail_root->job_id;
	if (!job_detail_root->job_next)
		out_statistic.last_job_id = job_detail_root->job_id;
	if (job_detail_root->job_next)
		PopulateJobStatistic(all_jobs.at(job_detail_root->job_next), all_jobs, out_statistic);
}

std::string JobCsvReader::FormatReport(JobStatistics job_statistic)
{
	std::sort(job_statistic.begin(), job_statistic.end(), [](const JobStatistic& a, const JobStatistic& b) { return a.Average() > b.Average(); });
	std::ostringstream output_stream;

	auto add_string = [&output_stream](const auto& str) { output_stream << str; };
	auto add_newline = [&add_string] { add_string("\n"); };

	for (const auto& job_statistic : job_statistic)
	{
		add_string("-");
		add_newline();
		add_string("start_job: ");
		add_string(job_statistic.job_id);
		add_newline();
		add_string("last_job: ");
		add_string(job_statistic.last_job_id);
		add_newline();
		add_string("number_of_jobs: ");
		add_string(job_statistic.count);
		add_newline();
		add_string("job_chain_runtime: ");
		add_string(ToTimestamp(job_statistic.total));
		add_newline();
		add_string("average_job_time: ");
		add_string(ToTimestamp(job_statistic.Average()));
		add_newline();
	}

	add_string("-");

	return output_stream.str();
}

bool JobCsvReader::BuildChain(JobDetailPointer job_detail, size_t job_next, std::unordered_map<int, JobDetailPointer> job_details, const int chain_id, int chain_sequence_number)
{
	if (!job_next)
		return true;

	if (job_details.count(job_next))
	{
		auto next_job = job_details.at(job_next);
		if (next_job->chain_id != 0)
			throw std::logic_error("Job belongs to alternate chain.");

		next_job->chain_id = chain_id;
		next_job->chain_sequence_number = ++chain_sequence_number;

		return BuildChain(next_job, next_job->job_next, job_details, chain_id, chain_sequence_number);
	}

	return false;
}

std::string JobCsvReader::ToTimestamp(int total_seconds)
{
	std::ostringstream timestamp;

	int hours = total_seconds / TimeConversions::minutes_per_hour / TimeConversions::seconds_per_minute;
	int minutes = total_seconds % (TimeConversions::minutes_per_hour * TimeConversions::seconds_per_minute) / TimeConversions::minutes_per_hour;
	int seconds = total_seconds % TimeConversions::seconds_per_minute;

	auto format_time = [](int time) -> std::string { return time < 10 ? std::string{"0"} + std::to_string(time) : std::to_string(time); };

	timestamp << format_time(hours) << ":" << format_time(minutes) << ":" << format_time(seconds);

	return timestamp.str();
}
