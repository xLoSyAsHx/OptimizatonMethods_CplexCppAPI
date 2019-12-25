#include <ilcplex\ilocplex.h>
#include "..\Lab_3_Clique_problem_heuristic\GraphUtils.h"

#include <chrono>
#include <memory>
#include <array>
#include <set>

#define DEBUG_ENABLE 0
#define ALL_PRINT_ENABLE 1

#define MAX_BNB_RECURSIVE_DEPTH 30
static int curBnbDepth = 0;

using namespace GraphUtils;
using std::cout;

IloNum objVal{ 0 };
IloEnv env;
IloModel model(env);
Graph graph;
std::vector<int> clique;

static const int CANDIDATE_ARRAY_SIZE = 2;

std::vector<int> g_allNodeIndexes;
std::shared_ptr<ColorisingHeuristic> g_pHeurInterface;
int upperBound;

int solveBnB(IloCplex& solver, IloNumVarArray& X);
void solveBnC(IloCplex& solver, IloNumVarArray& X);

std::array<std::vector<int>, CANDIDATE_ARRAY_SIZE> getCandidatesFromConstraintInds(
    Graph& graph,
    std::shared_ptr<ColorisingHeuristic> g_pHeurInterface,
    std::vector<int> constraintInds);

std::vector<int> getMaxIndependentSet(
    Graph& graph,
    int nodeInd);

IloNumExprArg make_constraint(IloNumVarArray & X, std::vector<int> & vecIndexes, bool enablePrint = false)
{
    IloNumExprArg newConstraintExpr = X[vecIndexes[0] - 1];
    for (int i = 1; i < vecIndexes.size(); ++i)
    {
        newConstraintExpr = newConstraintExpr + X[vecIndexes[i] - 1];
    }

#if DEBUG_ENABLE
    if (enablePrint)
    {
        std::cout << "\nConstraint: ";
        std::sort(vecIndexes.begin(), vecIndexes.end());
        for (auto& el : vecIndexes)
        {
            std::cout << el << " ";
        }
        std::cout << ";";
    }
#endif

    return newConstraintExpr;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Please, enter file name\n";
        return -1;
    }

    if (graph.loadFromFile(argv[1]) != 0)
        return -1;
    for (int i = 1; i < graph.m_nodes.size(); ++i)
        g_allNodeIndexes.emplace_back(i);

    int num_vertex = graph.m_nodes.size();
    

    // Decision variables
    IloNumVarArray X{ env, num_vertex - 1, 0, 1, IloNumVar::Type::Float };

    // Objective
    IloExpr objectiveExpr{ env };
    for (int i = 0; i < num_vertex - 1; i++)
        objectiveExpr += X[i];

    // Constraints
    IloRangeArray constraints{ env };
    g_pHeurInterface.reset(new ColorisingHeuristic);

    
    std::vector<int> tmp;
    auto colorizeSeq = g_pHeurInterface->ColorizeGraph(graph, tmp);
    // Add initial constraints from colorised graph
    int curColor = colorizeSeq[0].color;

    std::vector<int> constraintInds;
    for (auto& el : colorizeSeq)
    {
        if (el.color != curColor || el.val == colorizeSeq.back().val)
        {
            if (el.val == colorizeSeq.back().val)
                constraintInds.emplace_back(el.val);

            std::array<std::vector<int>, CANDIDATE_ARRAY_SIZE> candidates = getCandidatesFromConstraintInds(graph, g_pHeurInterface, constraintInds);
            if (candidates[0].empty())
            {
                constraints.add(make_constraint(X, constraintInds) <= 1);
                constraintInds.clear();

                curColor = el.color;
                constraintInds.emplace_back(el.val);
                continue;
            }

            for (auto& el : candidates)
            {
                if (!el.empty())
                    constraints.add(make_constraint(X, el) <= 1);
            }

            constraintInds.clear();

            curColor = el.color;
            constraintInds.emplace_back(el.val);
        }
        else
        {
            constraintInds.emplace_back(el.val);
        }
    }


    std::set<std::vector<int>> constraintIndxSet;
    for (int i = 1; i < graph.m_nodes.size(); ++i)
        constraintIndxSet.insert(getMaxIndependentSet(graph, i));

    for (auto el : constraintIndxSet)
    {
        if (el.empty())
            continue;

        constraints.add(make_constraint(X, el) <= 1);
    }

    /*
    tmp.clear();
    for (int i = 1; i < graph.m_nodes.size(); ++i)
        tmp.emplace_back(i);
    constraints.add(make_constraint(X, tmp) <= colorizeSeq[0].color);
    */

    // Model
    model.add(IloMaximize(env, objectiveExpr));
    model.add(constraints);

    // =============================  START  =============================
    auto start = std::chrono::high_resolution_clock::now();

    // Solver
    IloCplex solver(model);
    env.setOut(env.getNullStream());
    solver.setOut(env.getNullStream());
    upperBound = colorizeSeq[0].color;

    bool isClique = false;
    while (!isClique)
    {
        solveBnC(solver, X);

#if ALL_PRINT_ENABLE
        std::cout << "=========INTEGER SOLUTION========" << std::endl;
        std::cout << "Solution value  = " << objVal << std::endl;
        std::cout << "Clique: ";
        for (auto& el : clique)
            std::cout << el << " ";
        std::cout << std::endl;
        std::cout << "\n=================" << std::endl;
#endif

        int counter = 0;
        isClique = true;
        for (int i = 0; i < clique.size(); i++)
        {
            for (int j = i + 1; j < clique.size(); j++)
            {
                if (!graph.m_nodes[clique[i]].isNeighbour(clique[j]))
                {
                    isClique = false;
                    std::array<std::vector<int>, CANDIDATE_ARRAY_SIZE> candidates = getCandidatesFromConstraintInds(
                        graph, g_pHeurInterface, { clique[i], clique[j] }
                    );

                    for (auto& el : candidates)
                    {
                        if (!el.empty())
                        {
                            model.add(make_constraint(X, el, counter < 5) <= 1);
                            counter++;
                        }
                    }
                }
            }
        }

        if (isClique == false)
        {
            objVal = 0.0f;
        }

        std::cout << "\n\n";
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "=========FINAL SOLUTION========" << std::endl;
    std::cout << "Solution value  = " << objVal << std::endl;
    std::cout << "Clique: ";
    for (auto& el : clique)
        std::cout << el << " ";
    std::cout << std::endl;
    std::cout << "Time: " << (double)std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count() / 1000.0;
    std::cout << std::endl;
}

int getDecisionVar(IloCplex& solver, IloNumVarArray& X, bool& isXsInteger)
{
    isXsInteger = false;

    IloNumArray values{ env };
    IloNum minVal = 0.5;
    int decI = 0;

    solver.getValues(values, X);
    for (int i = 0; i < values.getSize(); i++) {
        IloNum floatPart = (values[i] - (long)values[i]);

        if (abs(floatPart - 0.5) < minVal && (floatPart > 0.05f || floatPart < 0.95f))
        {
            minVal = abs(floatPart - 0.5);
            decI = i;
        }
    }

    if (minVal == 0.5)
        isXsInteger = true;

    return decI;
}

int solveBnB(IloCplex& solver, IloNumVarArray& X)
{
    ++curBnbDepth;
    if (curBnbDepth > MAX_BNB_RECURSIVE_DEPTH)
    {
        --curBnbDepth;
        return 0;
    }

    bool isXsInteger;

    IloNumArray values{ env };
    solver.getValues(values, X);
    int decisionI = getDecisionVar(solver, X, isXsInteger);
    IloNum maxVal = solver.getValue(X[decisionI]);

    if (isXsInteger)
    {
        IloNum newObjVal = solver.getObjValue();
        if (solver.getObjValue() > objVal + 0.001f)
        {
            objVal = solver.getObjValue();

            IloNumArray values{ env };
            solver.getValues(values, X);
            clique.clear();
            //env.out() << "Clique: " << std::endl;
            for (int i = 0; i < values.getSize(); i++)
                if (values[i] > 0.05f) {
                    clique.emplace_back(i + 1);
                }

            --curBnbDepth;
            return 0;
        }
    }

    // go to branches recursively

    // add left branch
    IloRange newConstraint = X[decisionI] == 0;// <= std::floor(maxVal);
    model.add(newConstraint);
    bool leftSolved = solver.solve();
    IloNum leftObjValue = solver.getObjValue();

    int retVal = 1;
    if (leftSolved && leftObjValue > objVal + 0.01)
        solveBnB(solver, X);
    model.remove(newConstraint);
    if (retVal == 0)
    {
        --curBnbDepth;
        return 0;
    }

    // add right branch
    newConstraint = X[decisionI] == 1;// >= std::ceil(maxVal);
    model.add(newConstraint);
    bool rightSolved = solver.solve();
    IloNum rightObjValue = solver.getObjValue();

    if (rightSolved && rightObjValue > objVal + 0.01)
        solveBnB(solver, X);
    model.remove(newConstraint);
    if (retVal == 0)
    {
        --curBnbDepth;
        return 0;
    }

    --curBnbDepth;
    return 0;
}

std::array<std::vector<int>, CANDIDATE_ARRAY_SIZE> getCandidatesFromConstraintInds(Graph & graph, std::shared_ptr<ColorisingHeuristic> g_pHeurInterface, std::vector<int> constraintInds)
{
    std::vector<int> neighbours;
    int neighbourSize = 0;
    for (int nodeInd : constraintInds)
        neighbourSize += graph.m_nodes[nodeInd].edges.size();
    neighbours.resize(neighbourSize + constraintInds.size());

    neighbourSize = 0;
    for (int nodeInd : constraintInds)
    {
        auto& edges = graph.m_nodes[nodeInd].edges;
        std::copy(edges.begin(), edges.end(), neighbours.begin() + neighbourSize);
        neighbourSize += edges.size();
    }
    std::copy(constraintInds.begin(), constraintInds.end(), neighbours.begin() + neighbourSize);
    std::sort(neighbours.begin(), neighbours.end());
    neighbours.resize(
        std::distance(neighbours.begin(), std::unique(neighbours.begin(), neighbours.end()))
    );

    auto colorizeSeq = g_pHeurInterface->ColorizeGraph(graph, neighbours);
    std::sort(colorizeSeq.begin(), colorizeSeq.end(), [](auto& lhd, auto& rhd) {
        return lhd.color < rhd.color;
    });

    if (colorizeSeq.empty())
        return std::array<std::vector<int>, CANDIDATE_ARRAY_SIZE>();

    struct DistanceArray {
        int size;
        std::pair<decltype(colorizeSeq)::iterator, decltype(colorizeSeq)::iterator> range; // iterators to range begin and end
    };
    std::array<DistanceArray, CANDIDATE_ARRAY_SIZE> maxDistances{ 0 };
    for (int color = 1; color <= colorizeSeq.back().color; ++color)
    {
        ColorisingHeuristic::ValEdgeColor valEdgeColor = { -1, color, -1 };
        auto range = std::equal_range(colorizeSeq.begin(), colorizeSeq.end(), valEdgeColor, [](const auto& lhd, const auto& rhd) {
            return lhd.color < rhd.color;
        });
        int size = std::distance(range.first, range.second);

        for (int i = 0; i < CANDIDATE_ARRAY_SIZE; ++i)
        {
            if (size > maxDistances[i].size)
            {
                if (i != CANDIDATE_ARRAY_SIZE)
                    std::copy(maxDistances.begin() + i, maxDistances.end() - 1, maxDistances.begin() + i + 1);
                maxDistances[i].size = size;
                maxDistances[i].range = range;
                break;
            }
        }
    }

#if DEBUG_ENABLE && 0
    std::cout << "\n\ngetCandidatesFromConstraintInds result:\n";
    for (int i = 0; i < CANDIDATE_ARRAY_SIZE; ++i)
    {
        auto range = maxDistances[i].range;
        std::for_each(range.first, range.second, [](const auto& el) {
            std::cout << el.val << " ";
        });
        std::cout << "\n";
    }
    std::cout << "\n\n";
#endif

    std::array<std::vector<int>, CANDIDATE_ARRAY_SIZE> candidates;
    for (int i = 0; i < CANDIDATE_ARRAY_SIZE; ++i)
    {
        if (maxDistances[i].size)
        {
            candidates[i].resize(maxDistances[i].size + constraintInds.size());

            auto range = maxDistances[i].range;
            auto it_out = candidates[i].begin();
            std::for_each(range.first, range.second, [&it_out](const auto& el) {
                *it_out = el.val;
                ++it_out;
            });
            std::copy(constraintInds.begin(), constraintInds.end(), it_out);
        }
    }
    return candidates;
}

std::vector<int> getMaxIndependentSet(Graph& graph, int nodeInd)
{
    std::vector<int> neighbours{ graph.m_nodes[nodeInd].edges };
    neighbours.push_back(nodeInd);

    std::vector<int> vMaxIndepNodeIndx();
    
    std::vector<int> candidatesTmp;
    std::vector<int> constraintInds({ nodeInd });
    std::sort(neighbours.begin(), neighbours.end());
    std::set_difference(g_allNodeIndexes.begin(), g_allNodeIndexes.end(),
        neighbours.begin(), neighbours.end(),
        std::inserter(candidatesTmp, candidatesTmp.begin()));

    for (int i = 0; i < candidatesTmp.size(); i++) {
        bool isNeighbour = false;
        for (int j = 0; j < constraintInds.size(); j++) {
            isNeighbour = isNeighbour || graph.m_nodes[candidatesTmp[i]].isNeighbour(constraintInds[j]);
        }
        if (!isNeighbour) {
            constraintInds.push_back(candidatesTmp[i]);
        }
    }

    std::sort(constraintInds.begin(), constraintInds.end());
    return constraintInds;
}

void solveBnC(IloCplex& solver, IloNumVarArray& X)
{
    static int asd = 0;
    float sumX = 0.0f;
    do
    {
        solver.solve();
        float curValue = solver.getObjValue();
        asd++;
#if DEBUG_ENABLE
        {
            IloNumArray values{ env };
            std::vector<int> solution;
            solver.getValues(values, X);
            for (int i = 0; i < values.getSize(); i++)
                if (values[i] > 0)
                    solution.emplace_back(i + 1);

            std::cout << "\n\nClique: ";
            for (auto& el : solution)
                std::cout << el << " ";
        }
#endif
        if (upperBound < curValue)
            continue;

        IloNumArray values{ env };
        solver.getValues(values, X);
        std::vector<int> solution;

        int size = values.getSize();
        for (int i = 0; i < values.getSize(); i++)
            if (values[i] > 0.1f)
                solution.emplace_back(i + 1);

        std::vector<int> S;
        std::vector<int> maxS;
        for (int i = 0; i < solution.size(); i++)
        {
            S.emplace_back(solution[i]);
            for (int j = i + 1; j < solution.size(); j++)
            {
                bool ind = true;
                for (auto& el : S)
                {
                    if (graph.m_nodes[el].isNeighbour(solution[j]))
                    {
                        ind = false;
                        break;
                    }
                }

                if (ind)
                    S.emplace_back(solution[j]);
            }

            if (S.size() > maxS.size())
                std::swap(maxS, S);

            S.clear();
        }

        sumX = 0.0;
        for (auto& el : maxS)
            sumX += values[el - 1];

        if (sumX <= 1.0f)
            break;

        std::array<std::vector<int>, CANDIDATE_ARRAY_SIZE> candidates = getCandidatesFromConstraintInds(graph, g_pHeurInterface, maxS);

        if (candidates[0].empty())
            break;

        for (auto& el : candidates)
        {
            if (!el.empty())
                model.add(make_constraint(X, el) <= 1);
        }
    } while (sumX > 1.0f);

    solveBnB(solver, X);
}

