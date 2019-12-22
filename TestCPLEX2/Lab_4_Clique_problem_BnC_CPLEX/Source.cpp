#include <ilcplex\ilocplex.h>
#include "..\Lab_3_Clique_problem_heuristic\GraphUtils.h"

#include <memory>

using namespace GraphUtils;
using std::cout;

IloNum objVal{ 0 };
IloEnv env;
IloModel model(env);
std::vector<int> clique;

std::vector<int> g_allNodeIndexes;


int solveBnB(IloCplex& solver, IloNumVarArray& X);

std::vector<int> getCandidatesFromConstraintInds(Graph& graph, std::vector<int> constraintInds);

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Please, enter file name\n";
        return -1;
    }

    Graph graph;
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

    std::unique_ptr<ColorisingHeuristic> heuristicInterface{ new ColorisingHeuristic };
    auto colorizeSeq = heuristicInterface->ColorizeGraph(graph);

    // Add initial constraints from colorised graph
    int curColor = colorizeSeq[0].color;
    std::vector<int> constraintInds;
    constraintInds.emplace_back(colorizeSeq[0].val);
    for (auto& el : colorizeSeq)
    {
        if (el.color != curColor || el.val == colorizeSeq.back().val)
        {
            if (el.val == colorizeSeq.back().val)
                constraintInds.emplace_back(el.val);

            std::vector<int> candidates = getCandidatesFromConstraintInds(graph, constraintInds);
            if (candidates.empty())
                continue;

            IloNumExprArg newConstraintExpr = X[candidates[0] - 1];
            for (int i = 1; i < candidates.size(); ++i)
            {
                std::cout << candidates[i] << " ";
                newConstraintExpr = newConstraintExpr + X[candidates[i] - 1];
                bool isValid = newConstraintExpr.isValid();
            }

            constraints.add(newConstraintExpr <= 1);
            constraintInds.clear();

            curColor = el.color;
            constraintInds.emplace_back(el.val);
        }
        else
        {
            constraintInds.emplace_back(el.val);
        }
    }

    // Model
    model.add(IloMaximize(env, objectiveExpr));
    model.add(constraints);

    // Solver
    IloCplex solver(model);
    solver.solve();
    env.setOut(env.getNullStream());
    solver.setOut(env.getNullStream());

    solveBnB(solver, X);


    std::cout << "=========FINAL SOLUTION========" << std::endl;
    std::cout << "Solution value  = " << objVal << std::endl;
    std::cout << "Clique: ";
    for (int i = 0; i < clique.size(); i++)
        std::cout << clique[i] << " ";
    std::cout << std::endl;
}

int getDecisionVar(IloCplex& solver, IloNumVarArray& X, bool& isXsInteger)
{
    isXsInteger = false;

    IloNumArray values{ env };
    IloNum maxVal = 0.0;
    int decI = 0;

    solver.getValues(values, X);
    for (int i = 0; i < values.getSize(); i++) {
        IloNum floatPart = (values[i] - (long)values[i]);

        if (floatPart > maxVal)
        {
            maxVal = floatPart;
            decI = i;
        }
    }

    if (maxVal == 0.0)
        isXsInteger = true;

    return decI;
}


int solveBnB(IloCplex& solver, IloNumVarArray& X)
{
    bool isXsInteger;
    int decisionI = getDecisionVar(solver, X, isXsInteger);
    IloNum maxVal = solver.getValue(X[decisionI]);

    if (isXsInteger)
    {
        if (solver.getObjValue() > objVal)
        {
            objVal = solver.getObjValue();
            std::cout << "\n===================================Current Solution:" << std::endl;
            std::cout << "=====================================objValue: " << objVal << std::endl;

            IloNumArray values{ env };
            solver.getValues(values, X);
            clique.clear();
            //env.out() << "Clique: " << std::endl;
            for (int i = 0; i < values.getSize(); i++)
                if (values[i] > 0) {
                    clique.emplace_back(i);
                    std::cout << i << ", ";
                }

            return 0;
        }
    }

    // go to branches recursively

    // add left branch
    IloRange newConstraint = X[decisionI] <= std::floor(maxVal);
    model.add(newConstraint);
    bool leftSolved = solver.solve();
    IloNum leftObjValue = solver.getObjValue();

    if (leftSolved && leftObjValue > objVal)
        solveBnB(solver, X);
    model.remove(newConstraint);

    // add right branch
    newConstraint = X[decisionI] >= std::ceil(maxVal);
    model.add(newConstraint);
    bool rightSolved = solver.solve();
    IloNum rightObjValue = solver.getObjValue();

    if (rightSolved && rightObjValue > objVal)
        solveBnB(solver, X);
    model.remove(newConstraint);
}

std::vector<int> getCandidatesFromConstraintInds(Graph & graph, std::vector<int> constraintInds)
{
    std::vector<int> neighbours;
    int neighbourSize = 0;
    for (int nodeInd : constraintInds)
        neighbourSize += graph.m_nodes[nodeInd].edges.size();
    neighbours.resize(neighbourSize);

    neighbourSize = 0;
    for (int nodeInd : constraintInds)
    {
        auto& edges = graph.m_nodes[nodeInd].edges;
        std::copy(edges.begin(), edges.end(), neighbours.begin() + neighbourSize);
        neighbourSize += edges.size();
    }

    std::vector<int> candidates;
    std::sort(neighbours.begin(), neighbours.end());
    std::set_difference(g_allNodeIndexes.begin(), g_allNodeIndexes.end(),
        neighbours.begin(), neighbours.end(),
        std::inserter(candidates, candidates.begin()));

    return candidates;
}
