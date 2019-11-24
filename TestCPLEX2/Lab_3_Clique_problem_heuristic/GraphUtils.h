#ifndef GRAPH_PARSER_H
#define GRAPH_PARSER_H

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include <vector>
#include <map>
#include <random>
#include <algorithm>
#include <iterator>

namespace fs = std::experimental::filesystem;

namespace GraphUtils
{
    struct Node
    {
        int val;
        int color = 0;
        std::vector<int> edges;
    };

    struct Graph
    {
        int loadFromFile(std::string pathToFile);

        size_t m_numEdges;
        std::vector<Node> m_nodes;
    };

    class HeuristicInterface
    {
    public:
        virtual std::vector<int>& Apply(Graph& graph) = 0;
    };

    class ClolrisingHeuristic30 : public HeuristicInterface
    {
    public:
        virtual std::vector<int>& Apply(Graph & graph) override;

    private:
        struct ValEdgeColor
        {
            int val;
            int numEdges;
            int color;
        };

        struct Clique
        {
            std::vector<int> nodes;
            std::vector<ValEdgeColor> colorizeSeq;
        };

        void colorizeGraph(Graph & graph, std::vector<ValEdgeColor>& colorizeSeq, bool shuffle = false);
        void localSearch(Graph& graph, Clique& clique, std::vector<int> cliqueNeighbours, std::vector<std::vector<int>>& alreadyChecked);

        /*
        * Return node to be deleted to expand clique with *it node
        * Return 0 if can expand without deleting
        * Return -1 if node can be added even with delete 1 node from clique
        */
        int canExpandClique(Graph& graph, Clique& c, int nodeInd); 
        int canExpandWithoutDeleting(Graph& graph, Clique& c, int nodeInd); // return NodeInd to be added. -1 if can't

        void findClique(Graph & graph, Clique & c);
        void findCliqueReq(Graph & graph, std::vector<int> vCandidates, Clique& clique);

        int m_maxCliqueSize;
    };
}

#endif // GRAPH_PARSER_H