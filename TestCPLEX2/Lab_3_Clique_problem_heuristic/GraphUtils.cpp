#include "GraphUtils.h"
#include <ctime>

using namespace GraphUtils;
using std::cout;

clock_t start;

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
	for (auto& node : m_nodes) {
		std::sort(node.edges.begin(), node.edges.end());
		node.val = i++;
	}

    return 0;
}

int Graph::fromGraph(Graph& graph, std::vector<int>& nodes) {
	m_numEdges = 0;
	m_nodes.resize(graph.m_nodes.size());
	if (nodes.size() == 1) {
		m_nodes[nodes[0]].val = nodes[0];
		return 1;
	}
	int count = 0;
	for (auto& node : nodes) {
		std::set_intersection(graph.m_nodes[node].edges.begin(), graph.m_nodes[node].edges.end(),
			nodes.begin(), nodes.end(),
			std::back_inserter(m_nodes[node].edges));
		m_nodes[node].val = node;
		int edgeNumber = m_nodes[node].edges.size();
		if (edgeNumber != 0) {
			count++;
			m_numEdges += edgeNumber;
		}
	}
	return count;
}

std::vector<int>& ColorisingHeuristic::Apply(Graph & graph)
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
    colorizeSeq.pop_back();

    colorizeGraph(graph, colorizeSeq);

    // Sort by color
    std::sort(colorizeSeq.begin(), colorizeSeq.end(), [](auto& lhd, auto& rhd) {
        return lhd.color > rhd.color;
    });

	start = clock();
	Clique clique;
	Clique maxClique;
	findCliqueReq(graph, colorizeSeq, clique, maxClique);
	clock_t end = clock();
	printf("\nTime: %f, finished.", (double)(end - start) / CLOCKS_PER_SEC);

	return maxClique.nodes;
}

void ColorisingHeuristic::colorizeGraph(Graph & graph, std::vector<ValEdgeColor>& colorizeSeq, bool shuffle)
{
    if (shuffle)
    {
		std::random_device rd;
		std::mt19937 g(rd());
        std::shuffle(colorizeSeq.begin(), colorizeSeq.begin() + colorizeSeq.size() * 0.2, g);
    }

	for (auto& el : graph.m_nodes) {
		if (el.val != 0) {
			el.color = 1;
			if (colorizeSeq[0].val == 0) {
				colorizeSeq[el.val].color = 1;
			}
			break;
		}
	}
   
    for (auto& el : colorizeSeq)
    {
		if (el.numEdges == 0) {
			return;
		}
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


void ColorisingHeuristic::findCliqueReq(Graph & graph, std::vector<ValEdgeColor>& colorizeSeq, ColorisingHeuristic::Clique& currentClique, ColorisingHeuristic::Clique& maxClique)
{
	colorizeGraph(graph, colorizeSeq);

	// Sort by color
	std::sort(colorizeSeq.begin(), colorizeSeq.end(), [](auto& lhd, auto& rhd) {
		return lhd.color > rhd.color;
	});

	for (int i = 0; i < colorizeSeq.size(); i++) {

		ValEdgeColor toClique = colorizeSeq[i];
		if (toClique.val == 0 || colorizeSeq[i].color == 0) {
			currentClique.nodes.pop_back();
			return;
		}
		currentClique.nodes.push_back(toClique.val);
		colorizeSeq.erase(colorizeSeq.begin() + i);
		if (colorizeSeq[i].color + currentClique.nodes.size() <= maxClique.nodes.size()) {
			currentClique.nodes.pop_back();
			return;
		}
		Graph childGraph;
		Node currentNode = graph.m_nodes[toClique.val];

		for (auto& el : currentClique.nodes) {
			currentNode.edges.erase(std::remove(currentNode.edges.begin(), currentNode.edges.end(), el), currentNode.edges.end());
		}
		if (colorizeSeq[i].color == 0 || childGraph.fromGraph(graph, currentNode.edges) == 0) {
			maxClique.nodes = currentClique.nodes;
			clock_t end = clock();
			printf("\nTime: %f, clique: %d", (double)(end - start) / CLOCKS_PER_SEC, maxClique.nodes.size());
			currentClique.nodes.pop_back();
			return;
		}
		std::vector<ValEdgeColor> childColorizeSeq(childGraph.m_nodes.size());
		for (int i = 0; i < childGraph.m_nodes.size(); ++i)
		{
			childColorizeSeq[i].val = i;
			childColorizeSeq[i].numEdges = childGraph.m_nodes[i].edges.size();
		}

		// Sort by size of neighbours from small to large
		std::sort(childColorizeSeq.begin(), childColorizeSeq.end(), [](auto& lhd, auto& rhd) {
			return lhd.numEdges > rhd.numEdges;
			});
		childColorizeSeq.pop_back();
		findCliqueReq(childGraph, childColorizeSeq, currentClique, maxClique);
		currentClique.nodes.pop_back();
		i--;
	}

}
