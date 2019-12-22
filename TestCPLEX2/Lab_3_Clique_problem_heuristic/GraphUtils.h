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

#include <ctime>

namespace GraphUtils
{
    struct Node
    {
        int val;
        int color = 0;
        std::vector<int> edges;
        bool isNeighbour(int nodeInd) { return std::equal_range(edges.begin(), edges.end(), nodeInd).first != edges.end(); }
    };

    struct Graph
    {
        int loadFromFile(std::string pathToFile);
        int fromGraph(Graph& graph, std::vector<int>& nodes);

        size_t m_numEdges;
        std::vector<Node> m_nodes;
    };

    class HeuristicInterface
    {
    public:
        virtual std::vector<int> Apply(Graph& graph) = 0;
    };

    class ColorisingHeuristic : public HeuristicInterface
    {
    public:
        struct ValEdgeColor
        {
            int val;
            int color;
            int numEdges;
        };

        struct Clique
        {
            std::vector<int> nodes;
            std::vector<ValEdgeColor> colorizeSeq;
        };

        virtual std::vector<int> Apply(Graph & graph) override;
        std::vector<ValEdgeColor> ColorizeGraph(Graph & graph);

    private:

        void colorizeGraph(Graph & graph, std::vector<ValEdgeColor>& colorizeSeq, bool shuffle = false);

        void findCliqueReq(Graph & graph, std::vector<ValEdgeColor>& colorizeSeq, Clique& clique, Clique& maxClique, clock_t clockToCheck = 0);

        void localSearch(Graph& graph, Clique& clique, std::vector<int> cliqueNeighbours, std::vector<std::vector<int>>& alreadyChecked);

        /*
        * Return node to be deleted to expand clique with *it node
        * Return 0 if can expand without deleting
        * Return -1 if node can be added even with delete 1 node from clique
        */
        int canExpandClique(Graph& graph, Clique& c, int nodeInd); 
        int canExpandWithoutDeleting(Graph& graph, Clique& c, int nodeInd); // return NodeInd to be added. -1 if can't

        int m_maxCliqueSize;
    };
}

#endif // GRAPH_PARSER_H