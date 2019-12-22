#include <iostream>

#include "GraphUtils.h"

#include <chrono>
#include <filesystem>

using namespace GraphUtils;
using std::cout;

int main(int argc, char** argv)
{
    auto start = std::chrono::system_clock::now();
    // Some computation here
    auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Start program at " << std::ctime(&end_time) << "\n";

    Graph graph;
    if (graph.loadFromFile(argv[1]) != 0)
        return -1;

    HeuristicInterface* heuristic = new ColorisingHeuristic;
    std::vector<int> clique = heuristic->Apply(graph);

    cout << "\nSize: " << clique.size();
    return 0;
}