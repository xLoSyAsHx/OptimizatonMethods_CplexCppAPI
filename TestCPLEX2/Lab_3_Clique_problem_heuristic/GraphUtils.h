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
		int fromGraph(Graph& graph, std::vector<int>& nodes);

        size_t m_numEdges;
        std::vector<Node> m_nodes;
    };

    class HeuristicInterface
    {
    public:
        virtual std::vector<int>& Apply(Graph& graph) = 0;
    };

    class ColorisingHeuristic : public HeuristicInterface
    {
    public:
        virtual std::vector<int>& Apply(Graph & graph) override;

    private:
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

        void colorizeGraph(Graph & graph, std::vector<ValEdgeColor>& colorizeSeq, bool shuffle = false);
        void findCliqueReq(Graph & graph, std::vector<ValEdgeColor>& colorizeSeq, Clique& clique, Clique& maxClique);

        int m_maxCliqueSize;
    };
}

#endif // GRAPH_PARSER_H