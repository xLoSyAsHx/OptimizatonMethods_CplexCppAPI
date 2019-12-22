#include <ilcplex\ilocplex.h>
#include "..\Lab_3_Clique_problem_heuristic\GraphUtils.h"
using namespace GraphUtils;
using std::cout;

IloNum objVal{ 0 };
IloEnv env;
IloModel model(env);
std::vector<int> clique;


int solveBnB(IloCplex& solver, IloNumVarArray& X);

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
    IloNumVarArray X{ env, num_vertex, 0, 1, IloNumVar::Type::Float };

    // Objective
    IloExpr objectiveExpr{ env };
    for (int i = 0; i < num_vertex; i++)
        objectiveExpr += X[i];

    // Constraints
    IloRangeArray constraints{ env };

    for (int i = 0; i < num_vertex; i++) {
        std::vector<int> edges = graph.m_nodes[i].edges;
        for (int j = i + 1; j < num_vertex; j++) {
            if (std::find(edges.begin(), edges.end(), j) == edges.end())
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