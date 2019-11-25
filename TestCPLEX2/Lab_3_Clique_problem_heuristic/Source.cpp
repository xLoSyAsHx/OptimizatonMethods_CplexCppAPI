#include <iostream>

#include "GraphUtils.h"
using namespace GraphUtils;
using std::cout;

int main(int argc, char** argv)
{
    Graph graph;
<<<<<<< HEAD
    if (graph.loadFromFile(argv[1]) != 0)
        return -1;

    HeuristicInterface* heuristic = new ColorisingHeuristic;
    std::vector<int> clique = heuristic->Apply(graph);
=======
     if (graph.loadFromFile("test.clq") != 0)
    // if (graph.loadFromFile("keller4.clq") != 0)
        return -1;

    HeuristicInterface* heuristic = new ClolrisingHeuristic30;
    std::vector<int> heuristicClique = heuristic->Apply(graph);
>>>>>>> draft_local_search_heuristic

    cout << "\nSize: " << clique.size();
    getchar();
    return 0;
}