#include "GraphUtils.h"

using namespace GraphUtils;
using std::cout;


int Graph::loadFromFile(std::string pathToFile)
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
    // colorizeSeq.erase(colorizeSeq.begin());
    colorizeSeq.pop_back();

    std::vector<Clique> vCliques(30);
    for (int i = 0; i < 1; ++i)
    {
        colorizeGraph(graph, colorizeSeq);

        // Sort by color
        /*
        std::sort(graph.m_nodes.begin(), graph.m_nodes.end(), [](auto& lhd, auto& rhd) {
            return lhd.color > rhd.color;
        });
        */

        Clique c;
        c.nodes.push_back(8);
        c.nodes.push_back(15);
        c.nodes.push_back(16);

        std::vector<int> cliqueNeighbours;
        std::vector<std::vector<int>> alreadyChecked;
        localSearch(graph, c, cliqueNeighbours, alreadyChecked);
    }

    std::sort(vCliques.begin(), vCliques.end(), [](const auto& lhd, const auto& rhd) {
        return lhd.nodes.size() > rhd.nodes.size();
    });

    return vCliques[0].nodes;
}

void ClolrisingHeuristic30::colorizeGraph(Graph & graph, std::vector<ValEdgeColor>& colorizeSeq, bool shuffle)
{
    //std::shuffle(colorizeSeq.begin(), colorizeSeq.end(), g);

    if (shuffle)
    {
        std::random_device rd;
        std::mt19937 g(rd());
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

void ClolrisingHeuristic30::localSearch(Graph& graph, Clique& clique, std::vector<int> cliqueNeighbours, std::vector<std::vector<int>>& alreadyChecked)
{
    if (cliqueNeighbours.size() == 0)
    {
        /* Initialize:
         * For each node find candidates
        */
        for (auto cNodeInd : clique.nodes)
        {
            auto& cNode = graph.m_nodes[cNodeInd];
            std::vector<int> candidates;

            std::set_difference(cNode.edges.begin(), cNode.edges.end(),
                clique.nodes.begin(), clique.nodes.end(),
                std::inserter(candidates, candidates.begin()));

            // If the number of node edges is less than (clique size - 1)
            // therefore this node can't expand clique and we should take it into account in local search
            std::remove_if(candidates.begin(), candidates.end(), [&graph, cliqueSizeMin1 = clique.nodes.size() - 1](const auto& val) {
                std::cout << graph.m_nodes[val].edges.size() << "   " << cliqueSizeMin1 << "\n";
                return graph.m_nodes[val].edges.size() < cliqueSizeMin1;
            });

            cliqueNeighbours.insert(cliqueNeighbours.begin(), candidates.begin(), candidates.end());
            std::sort(cliqueNeighbours.begin(), cliqueNeighbours.end());
            auto endIt = std::unique(cliqueNeighbours.begin(), cliqueNeighbours.end());
            cliqueNeighbours.resize(std::distance(cliqueNeighbours.begin(), endIt));
        }
    }

 
    // for (auto it = cliqueNeighbours.begin(); it != cliqueNeighbours.end(); )
    //for (auto nodeToBeChecked : cliqueNeighbours)
    for (int i = 0; i < cliqueNeighbours.size(); ++i)
    {
        int nodeToBeChecked = cliqueNeighbours[i];
        int nodeToBeDeleted = canExpandClique(graph, clique, nodeToBeChecked);
        if (nodeToBeDeleted != -1)
        {
            clique.nodes.erase(std::find(clique.nodes.begin(), clique.nodes.end(), nodeToBeDeleted));

            //clique.nodes.insert(std::find(clique.nodes.begin(), clique.nodes.end(), *it), *it);
            clique.nodes.push_back(nodeToBeChecked);
            std::sort(clique.nodes.begin(), clique.nodes.end());

            int node2ToBeAdded = canExpandWithoutDeleting(graph, clique, nodeToBeChecked);
            if (node2ToBeAdded != -1)
            {
                // In this "if" we deleted "nodeToBeDeleted", added "*it" and can add "node2ToBeAdded" to increase current clique size
                // Besides that, we need to update "cliqueNeighbours" and call "localSearch" for new clique
                clique.nodes.push_back(node2ToBeAdded);

                if (std::find(alreadyChecked.begin(), alreadyChecked.end(), clique.nodes) == alreadyChecked.end())
                {
                    auto it = std::find(cliqueNeighbours.begin(), cliqueNeighbours.end(), nodeToBeChecked);
                    size_t newSize = std::distance(it + 1, cliqueNeighbours.end());
                    std::copy(it + 1, cliqueNeighbours.end(), cliqueNeighbours.begin()); // We already check this nodes
                    cliqueNeighbours.resize(newSize);

                    cliqueNeighbours.erase(std::find(cliqueNeighbours.begin(), cliqueNeighbours.end(), node2ToBeAdded));

                    std::vector<int> newNeighbours;
                    std::set_difference(cliqueNeighbours.begin(), cliqueNeighbours.end(),
                        graph.m_nodes[nodeToBeDeleted].edges.begin(), graph.m_nodes[nodeToBeDeleted].edges.end(),
                        std::inserter(newNeighbours, newNeighbours.begin()));
                    std::swap(cliqueNeighbours, newNeighbours);


                    // ---------- TODO: need to optimize ----- Add new candidates
                    {
                        int cNodeInd = nodeToBeChecked;
                        auto& cNode = graph.m_nodes[cNodeInd];
                        std::vector<int> candidates;

                        std::sort(clique.nodes.begin(), clique.nodes.end());
                        std::set_difference(cNode.edges.begin(), cNode.edges.end(),
                            clique.nodes.begin(), clique.nodes.end(),
                            std::inserter(candidates, candidates.begin()));

                        // If the number of node edges is less than (clique size - 1)
                        // therefore this node can't expand clique and we should take it into account in local search
                        std::remove_if(candidates.begin(), candidates.end(), [&graph, cliqueSizeMin1 = clique.nodes.size() - 1](const auto& val) {
                            return graph.m_nodes[val].edges.size() < cliqueSizeMin1;
                        });

                        cliqueNeighbours.insert(cliqueNeighbours.begin(), candidates.begin(), candidates.end());
                    }
                    {
                        int cNodeInd = node2ToBeAdded;
                        auto& cNode = graph.m_nodes[cNodeInd];
                        std::vector<int> candidates;

                        std::sort(clique.nodes.begin(), clique.nodes.end());
                        std::set_difference(cNode.edges.begin(), cNode.edges.end(),
                            clique.nodes.begin(), clique.nodes.end(),
                            std::inserter(candidates, candidates.begin()));

                        // If the number of node edges is less than (clique size - 1)
                        // therefore this node can't expand clique and we should take it into account in local search
                        std::remove_if(candidates.begin(), candidates.end(), [&graph, cliqueSizeMin1 = clique.nodes.size() - 1](const auto& val) {
                            return graph.m_nodes[val].edges.size() < cliqueSizeMin1;
                        });

                        cliqueNeighbours.insert(cliqueNeighbours.begin(), candidates.begin(), candidates.end());
                    }
                    // ----------

                    std::sort(clique.nodes.begin(), clique.nodes.end());
                    alreadyChecked.push_back(clique.nodes);
                    localSearch(graph, clique, cliqueNeighbours, alreadyChecked);

                    return; // We already found clique greater than clique on input, therefore no need to continue check neighbours
                }
            }
            else
            {
                if (std::find(alreadyChecked.begin(), alreadyChecked.end(), clique.nodes) == alreadyChecked.end())
                {
                    std::vector<int> cliqueNeighboursPrev = cliqueNeighbours;

                    auto it = std::find(cliqueNeighbours.begin(), cliqueNeighbours.end(), nodeToBeChecked);
                    size_t newSize = std::distance(it + 1, cliqueNeighbours.end());
                    std::copy(it + 1, cliqueNeighbours.end(), cliqueNeighbours.begin()); // We already check this nodes
                    cliqueNeighbours.resize(newSize);

                    std::vector<int> newNeighbours;
                    std::set_difference(cliqueNeighbours.begin(), cliqueNeighbours.end(),
                        graph.m_nodes[nodeToBeDeleted].edges.begin(), graph.m_nodes[nodeToBeDeleted].edges.end(),
                        std::inserter(newNeighbours, newNeighbours.begin()));
                    std::swap(cliqueNeighbours, newNeighbours);

                    // ---------- TODO: need to optimize ----- Add new candidates
                    {
                        int cNodeInd = nodeToBeChecked;
                        auto& cNode = graph.m_nodes[cNodeInd];
                        std::vector<int> candidates;

                        std::sort(clique.nodes.begin(), clique.nodes.end());
                        std::set_difference(cNode.edges.begin(), cNode.edges.end(),
                            clique.nodes.begin(), clique.nodes.end(),
                            std::inserter(candidates, candidates.begin()));

                        // If the number of node edges is less than (clique size - 1)
                        // therefore this node can't expand clique and we should take it into account in local search
                        std::remove_if(candidates.begin(), candidates.end(), [&graph, cliqueSizeMin1 = clique.nodes.size() - 1](const auto& val) {
                            return graph.m_nodes[val].edges.size() < cliqueSizeMin1;
                        });

                        cliqueNeighbours.insert(cliqueNeighbours.begin(), candidates.begin(), candidates.end());
                        std::sort(cliqueNeighbours.begin(), cliqueNeighbours.end());
                        auto endIt = std::unique(cliqueNeighbours.begin(), cliqueNeighbours.end());
                        cliqueNeighbours.resize(std::distance(cliqueNeighbours.begin(), endIt));
                    }
                    // ----------

                    int prevCliqueSize = clique.nodes.size();
                    std::sort(clique.nodes.begin(), clique.nodes.end());
                    alreadyChecked.push_back(clique.nodes);
                    localSearch(graph, clique, cliqueNeighbours, alreadyChecked);
                    std::swap(cliqueNeighboursPrev, cliqueNeighbours);
                    if (prevCliqueSize < clique.nodes.size())
                        return;
                }
            }

            clique.nodes.erase(std::find(clique.nodes.begin(), clique.nodes.end(), nodeToBeChecked));
            //clique.nodes.insert(std::find(clique.nodes.begin(), clique.nodes.end(), nodeToBeDeleted), nodeToBeDeleted);
            clique.nodes.emplace_back(nodeToBeDeleted);
            std::sort(clique.nodes.begin(), clique.nodes.end());
        }
    }

}

int GraphUtils::ClolrisingHeuristic30::canExpandClique(Graph& graph, Clique& c, int nodeInd)
{
    int numOfAbsentNodes = 0;
    int nodeToDelete = 0;

    // Must be sorted
    auto cNodeInd = c.nodes.begin();

    /*
    auto first1 = graph.m_nodes[nodeInd].edges.begin();
    auto last1 = graph.m_nodes[nodeInd].edges.end();
    auto first2 = c.nodes.begin();
    auto last2 = c.nodes.end();
    */
    auto first1 = c.nodes.begin();
    auto last1 = c.nodes.end();
    auto first2 = graph.m_nodes[nodeInd].edges.begin();
    auto last2 = graph.m_nodes[nodeInd].edges.end();

    while (first1 != last1) {
        if (first2 == last2) {
            numOfAbsentNodes += last1 - first1;
            nodeToDelete = *first1;
            break;
        }

        if (*first1 < *first2) {
            nodeToDelete = *first1;
            first1++;
            numOfAbsentNodes++;
            if (numOfAbsentNodes > 1) return -1;

        }
        else {
            if (!(*first2 < *first1)) {
                ++first1;
            }
            ++first2;
        }
    }
    //;

    return (numOfAbsentNodes < 2) ? nodeToDelete : -1;
}

int GraphUtils::ClolrisingHeuristic30::canExpandWithoutDeleting(Graph& graph, Clique& c, int nodeInd)
{
    // All neighbours of newly added candidate which are not in the clique
    std::vector<int> differences;
    std::set_difference(graph.m_nodes[nodeInd].edges.begin(), graph.m_nodes[nodeInd].edges.end(),
        c.nodes.begin(), c.nodes.end(), std::inserter(differences, differences.begin()));

    if (differences.empty())
        return -1;

    for (auto candidate : differences) {
        int nodeToBeDeleted = canExpandClique(graph, c, candidate);
        if (nodeToBeDeleted == 0)
            return candidate;
    }

    return -1;
}

void ClolrisingHeuristic30::findClique(Graph & graph, Clique& c)
{
    /*
    Clique c;
    c.nodes.push_back(graph.m_nodes[0].val);
    findCliqueReq(graph, graph.m_nodes[0].edges, c);
    return c;
    */
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
