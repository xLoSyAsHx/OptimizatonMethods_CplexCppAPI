#ifndef GRAPH_PARSER_H
#define GRAPH_PARSER_H

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include <vector>
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
        int loadFromFile(fs::path pathToFile);

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
        Clique& findClique(Graph & graph);
        void findCliqueReq(Graph & graph, std::vector<int> vCandidates, Clique& clique);

        int m_maxCliqueSize;
    };
}

#endif // GRAPH_PARSER_H