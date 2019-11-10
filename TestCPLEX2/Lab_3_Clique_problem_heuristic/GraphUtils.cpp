#include "GraphUtils.h"

using namespace GraphUtils;
using std::cout;


int Graph::loadFromFile(fs::path pathToFile)
{
    std::ifstream graphFile(pathToFile, std::ifstream::in);
    if (graphFile.is_open() == false)
    {
        cout << "\nError: Failed to open file " << pathToFile << "\n";
        return -1;
    }

    std::string line;
    while (std::getline(graphFile, line))
    {
        if (line[0] == 'c')
            continue;

        if (line[0] == 'p')
        {
            int numNodes;
            std::stringstream ss(line.substr(line.find_first_of("1234567890")));
            ss >> numNodes >> m_numEdges;
            m_nodes.resize(numNodes + 1);

            continue;
        }

        if (line[0] == 'e')
        {
            std::stringstream ss(line.substr(2));
            int n1, n2;
            ss >> n1 >> n2;

            m_nodes[n1].edges.emplace_back(n2);
            m_nodes[n2].edges.emplace_back(n1);
        }
    }

    int i = 0;
    for (auto& node : m_nodes)
        node.val = i++;
    
    return 0;
}

std::vector<int>& ClolrisingHeuristic30::Apply(Graph & graph)
{
    m_maxCliqueSize = 0;

    std::vector<ValEdgeColor> colorizeSeq(graph.m_nodes.size());
    for (int i = 0; i < graph.m_nodes.size(); ++i)
    {
        colorizeSeq[i].val = i;
        colorizeSeq[i].numEdges = graph.m_nodes[i].edges.size();
    }

    // Sort by size of neighbours from small to large
    std::sort(colorizeSeq.begin(), colorizeSeq.end(), [](auto& lhd, auto& rhd) {
        return lhd.numEdges > rhd.numEdges;
    });
    colorizeSeq.erase(colorizeSeq.begin());
    colorizeSeq.pop_back();

    std::vector<Clique> vCliques(30);
    for (int i = 0; i < 30; ++i)
    {
        colorizeGraph(graph, colorizeSeq);

        // Sort by color
        std::sort(graph.m_nodes.begin(), graph.m_nodes.end(), [](auto& lhd, auto& rhd) {
            return lhd.color > rhd.color;
        });

        vCliques[0] = std::move(findClique(graph));
        
    }

    std::sort(vCliques.begin(), vCliques.end(), [](const auto& lhd, const auto& rhd) {
        return lhd.nodes.size() > rhd.nodes.size();
    });

    return vCliques[0].nodes;
}

void ClolrisingHeuristic30::colorizeGraph(Graph & graph, std::vector<ValEdgeColor>& colorizeSeq, bool shuffle)
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(colorizeSeq.begin(), colorizeSeq.end(), g);

    if (shuffle)
    {
        std::shuffle(colorizeSeq.begin(), colorizeSeq.begin() + colorizeSeq.size() * 0.2, g);
    }

    graph.m_nodes[1].color = 1;
    for (auto& el : colorizeSeq)
    {
        auto& curNode = graph.m_nodes[el.val];
        std::vector<int> existedColors;
        for (const auto& neighbour : curNode.edges)
        {
            existedColors.push_back(graph.m_nodes[neighbour].color);
        }
        std::sort(existedColors.begin(), existedColors.end());

        int color = existedColors.front();
        for (auto it = existedColors.begin(); it != std::unique(existedColors.begin(), existedColors.end()); ++it)
        {
            if (color != *it)
                break;

            color++;
        }

        el.color = curNode.color = color;
    }
}

ClolrisingHeuristic30::Clique& ClolrisingHeuristic30::findClique(Graph & graph)
{
    Clique c;
    c.nodes.push_back(graph.m_nodes[0].val);
    findCliqueReq(graph, graph.m_nodes[0].edges, c);
    return c;
}

void ClolrisingHeuristic30::findCliqueReq(Graph & graph, std::vector<int> vCandidates, ClolrisingHeuristic30::Clique& clique)
{
    auto checkNode = [graph, clique](int node) {
        for (const auto& cliqueNode : clique.nodes)
        {

        }
    };

    for (auto node : vCandidates)
    {

    }
}
