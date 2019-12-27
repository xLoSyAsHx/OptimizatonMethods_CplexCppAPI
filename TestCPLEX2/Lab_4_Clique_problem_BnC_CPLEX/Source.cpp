#include <ilcplex\ilocplex.h>
#include "..\Lab_3_Clique_problem_heuristic\GraphUtils.h"

#include <chrono>
#include <memory>
#include <array>
#include <set>

#define DEBUG_ENABLE 0
#define DEBUG_VARIABLES_ENABLE 0
#define ALL_PRINT_ENABLE 1

#define MAX_BNB_RECURSIVE_DEPTH 30
static int curBnbDepth = 0;
static std::vector<int> asd;

using namespace GraphUtils;
using std::cout;

IloNum objVal{ 0 };
IloEnv env;
IloModel model(env);
Graph graph;
std::vector<int> clique;
#if DEBUG_VARIABLES_ENABLE
std::vector<std::string> g_lastAddedConstr;
std::vector<double> g_lastAddedObjVal;
#endif

static const int CANDIDATE_ARRAY_SIZE = 2;

std::vector<int> g_allNodeIndexes;
std::vector<int> g_allNodeDegrees;
std::vector<int> g_allNodeDegreesSorted;
std::shared_ptr<ColorisingHeuristic> g_pHeurInterface;
int upperBound;

int solveBnB(IloCplex& solver, IloNumVarArray& X);
void solveBnC2(IloCplex& solver, IloNumVarArray& X);
void getConstraints(Graph& graph, std::vector<int>& maxS);

IloNumExprArg make_constraint(IloNumVarArray & X, std::vector<int> & vecIndexes, bool enablePrint = true)
{
    IloNumExprArg newConstraintExpr = X[vecIndexes[0] - 1];
    std::string tmp = std::to_string(vecIndexes[0]);
    for (int i = 1; i < vecIndexes.size(); ++i)
    {
        newConstraintExpr = newConstraintExpr + X[vecIndexes[i] - 1];
        tmp += " + " + std::to_string(vecIndexes[i]);
    }
#if DEBUG_VARIABLES_ENABLE
    g_lastAddedConstr.push_back(tmp);
#endif

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
    {
        g_allNodeIndexes.emplace_back(i);
        g_allNodeDegrees.emplace_back(graph.m_nodes[i].edges.size());
    }
    g_allNodeDegreesSorted = g_allNodeIndexes;
    std::sort(g_allNodeDegreesSorted.begin(), g_allNodeDegreesSorted.end(), [](auto& lhd, auto& rhd) {
        return graph.m_nodes[lhd].edges.size() < graph.m_nodes[rhd].edges.size();
     });

    

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

            std::sort(constraintInds.begin(), constraintInds.end());
            getConstraints(graph, constraintInds);
            if (constraintInds.empty())
            {
                constraintInds.clear();

                curColor = el.color;
                constraintInds.emplace_back(el.val);
                continue;
            }

            constraints.add(make_constraint(X, constraintInds) <= 1);
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
        try
        {
            solveBnC2(solver, X);
        }
        catch (IloException e)
        {
            std::cout << "\n\n\nExceptions:\n" << e << "\n";
            exit(1);
        }

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
                    std::vector<int> candidates = { clique[i], clique[j] };
                    getConstraints(graph, candidates);
                    model.add(make_constraint(X, candidates, counter < 5) <= 1);
                    counter++;
                }
            }
        }

        clique_check:
        if (isClique == false)
        {
            objVal = 0.0f;
        }

        std::cout << "\n\n";
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "=========FINAL SOLUTION========" << std::endl;
    std::cout << "Clique name: " << argv[1] << std::endl;
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
    IloNum maxVal = 0.0;
    int decI = 0;

    solver.getValues(values, X);
    for (int i = 0; i < values.getSize(); i++) {
        IloNum floatPart = (values[i] - (long)values[i]);

        if (floatPart > maxVal && (floatPart > FLT_EPSILON || floatPart < 1.0 - FLT_EPSILON))
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

    IloNumArray values{ env };
    solver.getValues(values, X);
    int decisionI = getDecisionVar(solver, X, isXsInteger);
    IloNum maxVal = solver.getValue(X[decisionI]);

    if (isXsInteger)
    {
        IloNum newObjVal = solver.getObjValue();
        if (abs(solver.getObjValue() - objVal) > FLT_EPSILON)
        {
            objVal = solver.getObjValue();

            IloNumArray values{ env };
            solver.getValues(values, X);
            clique.clear();
            for (int i = 0; i < values.getSize(); i++)
                if (values[i] > FLT_EPSILON) {
                    clique.emplace_back(i + 1);
                }

            return 0;
        }
    }

    // go to branches recursively
    // add left branch
    IloRange newConstraint = X[decisionI]  <= std::floor(maxVal);
    model.add(newConstraint);
    bool leftSolved = solver.solve();
    IloNum leftObjValue = solver.getObjValue();

    if (leftSolved && abs(leftObjValue - objVal) > FLT_EPSILON)
        solveBnC2(solver, X);
    model.remove(newConstraint);

    // add right branch
    newConstraint = X[decisionI] >= std::ceil(maxVal);
    model.add(newConstraint);
    bool rightSolved = solver.solve();
    IloNum rightObjValue = solver.getObjValue();

    if (rightSolved && abs(rightObjValue - objVal) > FLT_EPSILON)
        solveBnC2(solver, X);
    model.remove(newConstraint);

    return 0;
}

void getConstraints(Graph& graph, std::vector<int>& maxS)
{
    // return;
    std::vector<int> candidates;
    std::copy_if(g_allNodeDegreesSorted.begin(), g_allNodeDegreesSorted.end(), std::back_inserter(candidates),
        [&maxS](auto& el) {
            auto it = std::equal_range(maxS.begin(), maxS.end(), el).first;
            if (it == maxS.end() || *it != el) return true;
            return false;
    });

    for (auto& elToAdd : candidates)
    {
        bool canAdd = true;
        for (auto& el : maxS)
        {
            if (graph.m_nodes[el].isNeighbour(elToAdd))
            {
                canAdd = false;
                break;
            }
        }

        if (canAdd)
            maxS.push_back(elToAdd);
    }
}

void solveBnC2(IloCplex& solver, IloNumVarArray& X)
{
    static int asd = 0;
    float sumX = 0.0f;
    std::vector<double> lastAddedObjVals;
    do
    {
        solver.solve();
        float curValue = solver.getObjValue();
#if DEBUG_VARIABLES_ENABLE
        g_lastAddedObjVal.push_back(curValue);
#endif
        lastAddedObjVals.push_back(curValue);
        asd++;

        if (lastAddedObjVals.size() == 15)
        {
            double sum = 0.0;
            for (auto& el : lastAddedObjVals) sum += el;

            double mean = sum / lastAddedObjVals.size();
            if (abs(abs(lastAddedObjVals.front() - mean) - abs(lastAddedObjVals.back() - mean)) < 0.5)
                break;
        }

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
            if (values[i] > FLT_EPSILON)
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
        std::sort(maxS.begin(), maxS.end());
        for (auto& el : maxS)
            sumX += values[el - 1];

        if (sumX <= 1.01f)
            break;

        std::sort(maxS.begin(), maxS.begin());
        getConstraints(graph, maxS);

        if (maxS.empty())
            break;

        model.add(make_constraint(X, maxS) <= 1);
    } while (sumX > 1.0f);

    solveBnB(solver, X);
}
