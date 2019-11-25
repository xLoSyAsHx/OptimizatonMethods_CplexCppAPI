#include <iostream>

#include "GraphUtils.h"
using namespace GraphUtils;
using std::cout;

int main(int argc, char** argv)
{
    Graph graph;
    if (graph.loadFromFile(argv[1]) != 0)
        return -1;

    HeuristicInterface* heuristic = new ClolrisingHeuristic30;
    int heuristicClique = heuristic->Apply(graph);

    cout << "\nSize: " << heuristicClique;
    getchar();
    return 0;
}