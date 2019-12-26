#include <ilcplex\ilocplex.h>
#include "..\Lab_3_Clique_problem_heuristic\GraphUtils.h"
using namespace GraphUtils;
using std::cout;

IloNum objVal{ 0 };
IloEnv env;
IloModel model(env);
std::vector<int> clique;
const int REC_LIMIT = 50;


int solveBnB(IloCplex& solver, IloNumVarArray& X, int ind);

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

    int num_vertex = graph.m_nodes.size();

    // Decision variables
    IloNumVarArray X{ env, num_vertex - 1, 0, 1, IloNumVar::Type::Float };

    // Objective
    IloExpr objectiveExpr{ env };
    for (int i = 0; i < num_vertex - 1; i++)
        objectiveExpr += X[i];

    // Constraints
    IloRangeArray constraints{ env };

    for (int i = 0; i < num_vertex - 1; i++) {
        std::vector<int> edges = graph.m_nodes[i + 1].edges;
        for (int j = i + 1; j < num_vertex - 1; j++) {
            if (std::find(edges.begin(), edges.end(), j + 1) == edges.end())
            {
                constraints.add(X[i] + X[j] <= 1);
            }
        }
    }

    // Model
    model.add(IloMaximize(env, objectiveExpr));
    model.add(constraints);

    // Solver
    IloCplex solver(model);
    env.setOut(env.getNullStream());
    solver.setOut(env.getNullStream());

    solver.solve();

    solveBnB(solver, X, 0);


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
    float maxVal = 0.0f;
    int decI = 0;

    solver.getValues(values, X);
    for (int i = 0; i < values.getSize(); i++) {
        float floatPart = (values[i] - (long)values[i]);

        if ((floatPart - maxVal) > FLT_EPSILON)
        {
            maxVal = floatPart;
            decI = i;
        }
    }

    if (abs(maxVal) <= FLT_EPSILON)
        isXsInteger = true;

    return decI;
}


int solveBnB(IloCplex& solver, IloNumVarArray& X, int ind)
{
    if (ind >= REC_LIMIT) return 0;

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
                    clique.emplace_back(i + 1);
                    //env.out() << i << ", ";
                }

            return 0;
        }
    }

    // go to branches recursively

    // add left branch
    IloRange newConstraint = X[decisionI] <= std::floor(maxVal);
    model.add(newConstraint);
    bool leftSolved = solver.solve();
    float leftObjValue = solver.getObjValue();

    if (leftSolved && (leftObjValue - objVal) > FLT_EPSILON)
        solveBnB(solver, X, ind + 1);
    model.remove(newConstraint);

    // add right branch
    newConstraint = X[decisionI] >= std::ceil(maxVal);
    model.add(newConstraint);
    bool rightSolved = solver.solve();
    float rightObjValue = solver.getObjValue();

    if (rightSolved && (rightObjValue - objVal) > FLT_EPSILON)
        solveBnB(solver, X, ind + 1);
    model.remove(newConstraint);
}