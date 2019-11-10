#include <iostream>

#include "GraphUtils.h"
using namespace GraphUtils;
using std::cout;

int main()
{
    Graph graph;
    if (graph.loadFromFile("test.clq") != 0)
        return -1;

    HeuristicInterface* heuristic = new ClolrisingHeuristic1;
    std::vector<int> heuristicClique = heuristic->Apply(graph);

    cout << "\nSize: " << heuristicClique.size();
    getchar();
    return 0;
}