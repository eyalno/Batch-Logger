#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>

using namespace std;

// Function to find the job order
vector<int> jobOrder(int numJobs, vector<pair<int, int>>& dependencies) {
    vector<int> result;
    unordered_map<int, vector<int>> graph;
    vector<int> indegree(numJobs, 0);

    // Build the graph
    for (auto& dep : dependencies) {
        graph[dep.first].push_back(dep.second);
        indegree[dep.second]++;
    }

    // Initialize a queue with jobs having no dependencies (indegree = 0)
    queue<int> q;
    for (int i = 0; i < numJobs; i++) {
        if (indegree[i] == 0) {
            q.push(i);
        }
    }

    // Perform Topological Sort
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        result.push_back(current);

        for (int neighbor : graph[current]) {
            indegree[neighbor]--;
            if (indegree[neighbor] == 0) {
                q.push(neighbor);
            }
        }
    }

    // If the result doesn't include all jobs, there's a cycle
    if (result.size() != numJobs) {
        throw runtime_error("Cyclic dependency detected. No valid job order.");
    }

    return result;
}

// Main Function
int main() {
    int numJobs = 6; // Example: Number of jobs
    vector<pair<int, int>> dependencies = {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5} // Example dependencies
    };

    try {
        vector<int> order = jobOrder(numJobs, dependencies);
        cout << "Valid Job Order: ";
        for (int job : order) {
            cout << job << " ";
        }
    } catch (const runtime_error& e) {
        cout << e.what() << endl;
    }

    return 0;
}
