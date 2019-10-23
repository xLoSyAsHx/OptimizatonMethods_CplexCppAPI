#include <iostream>
#include <array>

#include <ilcplex/ilocplex.h>

const int iSize = 3;
const int jSize = 4;

const IloNum eps = 1e-50;

using DecVarArray3 = std::array<IloNumVarArray, 3>;
using NumArray3 = std::array<IloNumArray, 3>;

struct SolvConstr {
    IloCplex solver;
    IloExpr objectiveExpr;
    IloRangeArray constraints;
};

void SolveWithBranchAndBounds(IloEnv& env, IloCplex& solver, DecVarArray3 &X, DecVarArray3 &Y, IloNum& bestObjVal, NumArray3& bestValsX, NumArray3& bestValsY);

int main()
{
    IloEnv env;
    IloModel model(env);

    // Variables
    /*
    IloNumArray capacity{ env,      iSize, 50, 30, 20 };
    IloNumArray numAircrafts{ env,  iSize,  5,  8, 10 };
    IloNumArray t_routes[] = {
        IloNumArray{env, jSize, 3, 2, 2, 1},
        IloNumArray{env, jSize, 4, 3, 3, 2},
        IloNumArray{env, jSize, 5, 5, 4, 2},
    };
    IloNumArray c_costs[] = {
        IloNumArray{env, jSize, 1000, 1100, 1200, 1500},
        IloNumArray{env, jSize, 800,   900, 1000, 1000},
        IloNumArray{env, jSize, 600,   800,  800,  900},
    };
    IloNumArray penalty{ env,  jSize,   40,   50,  45,   70 };
    IloNumArray d_goal{ env,  jSize, 1000, 2000, 900, 1200 };
    */

    
    IloNumArray capacity{ env,      iSize, 50, 30, 20 };
    IloNumArray numAircrafts{ env,  iSize, 27, 23, 23 };
    IloNumArray t_routes[] = {
        IloNumArray{env, jSize, 3, 2, 2, 1},
        IloNumArray{env, jSize, 4, 3, 3, 2},
        IloNumArray{env, jSize, 5, 5, 4, 2},
    };
    IloNumArray c_costs[] = {
        IloNumArray{env, jSize, 1000, 1100, 1200, 1500},
        IloNumArray{env, jSize, 800,   900, 1000, 1000},
        IloNumArray{env, jSize, 600,   800,  800,  900},
    };
    IloNumArray penalty{ env,  jSize,   40,   50,  45,   70 };
    IloNumArray d_goal{ env,  jSize,  1700,  200, 900, 1200 };
    

    // Decision variables
    DecVarArray3 X = {
        IloNumVarArray{ env, jSize, 0, numAircrafts[0], IloNumVar::Type::Float },
        IloNumVarArray{ env, jSize, 0, numAircrafts[1], IloNumVar::Type::Float },
        IloNumVarArray{ env, jSize, 0, numAircrafts[2], IloNumVar::Type::Float },
    };
    DecVarArray3 Y = {
        IloNumVarArray{ env, jSize, 0, numAircrafts[0], IloNumVar::Type::Float },
        IloNumVarArray{ env, jSize, 0, numAircrafts[1], IloNumVar::Type::Float },
        IloNumVarArray{ env, jSize, 0, numAircrafts[2], IloNumVar::Type::Float },
    };
    
    // Objective
    IloExpr objectiveExpr{ env };
    for (int i = 0; i < iSize; ++i)
    {
        for (int j = 0; j < jSize; ++j)
        {
            objectiveExpr -= c_costs[i][j] * t_routes[i][j] * Y[i][j]; // c * t * Y
            objectiveExpr += capacity[i] * t_routes[i][j] * penalty[j] * X[i][j]; // Q * t * p * X
        }
    }

    // Constraints
    IloRangeArray constraints{ env };

    // Sum Y <= N
    for (int i = 0; i < iSize; ++i)
    {
        IloExpr expr{ env };
        for (int j = 0; j < jSize; ++j)
        {
            expr += Y[i][j];
        }
        constraints.add(expr <= numAircrafts[i]);
    }

    // Sum Q x t <= d
    for (int j = 0; j < jSize; ++j)
    {
        IloExpr expr{ env };
        for (int i = 0; i < iSize; ++i)
        {
            expr += capacity[i] * t_routes[i][j] * X[i][j];
        }
        constraints.add(expr <= d_goal[j]);
    }

    // x - y <= 0
    for (int i = 0; i < iSize; ++i)
    {
        for (int j = 0; j < jSize; ++j)
        {
            constraints.add(X[i][j] - Y[i][j] <= 0);
        }
    }

    // Model
    model.add(IloMaximize(env, objectiveExpr));
    model.add(constraints);

    IloNum bestObjVal{ 0 };
    NumArray3 bestValsX, bestValsY;

    IloCplex solver{ model };
    solver.solve();

    SolveWithBranchAndBounds(env, solver, X, Y, bestObjVal, bestValsX, bestValsY);

    env.out() << "\n\n\n\n\n\n\n\n";
    env.out() << "======= Final solution =======\n\n";
    env.out() << "Best object value: " << bestObjVal << "\n\n";

    for (int i = 0; i < iSize; ++i)
        env.out() << "X[" << i << "] = " << bestValsX[i] << std::endl;

    env.out() << std::endl;

    for (int i = 0; i < iSize; ++i)
        env.out() << "Y[" << i << "] = " << bestValsY[i] << std::endl;

    getchar();
}

inline IloNum getFract(IloNum d) { return d - (long)d; }

struct PairIJ { int i; int j; };
PairIJ getDecisionVarWithMaxFractional(IloEnv& env, IloCplex& solver, DecVarArray3 &X, DecVarArray3 &Y, bool& isAllYInteger)
{
    isAllYInteger = false;

    IloNumArray vals{ env };
    IloNum maxVal = 0.0;
    PairIJ pairIJ = { 0, 0 };

    for (int i = 0; i < iSize; ++i)
    {
        solver.getValues(vals, Y[i]);
        for (int j = 0; j < jSize; ++j)
        {
            if (getFract(vals[j]) > maxVal)
            {
                maxVal = getFract(vals[j]);
                pairIJ.i = i;
                pairIJ.j = j;
            }
        }
        vals.clear();
    }

    // If all X values is Integer - solution found. Need to stop
    if (maxVal == 0.0)
    {
        isAllYInteger = true;
    }

    return pairIJ;
}

bool isXYSatisfyConstraints(IloEnv& env, IloCplex& solver, DecVarArray3 &X, DecVarArray3 &Y)
{
    IloNumArray valsX{ env };
    IloNumArray valsY{ env };
    for (int i = 0; i < iSize; ++i)
    {
        solver.getValues(valsX, X[i]);
        solver.getValues(valsY, Y[i]);
        for (int j = 0; j < jSize; ++j)
        {
            if (valsY[j] != std::ceil(valsX[j]))
                return false;
        }
    }

    return true;
}

void SolveWithBranchAndBounds(IloEnv& env, IloCplex& solver, DecVarArray3 &X, DecVarArray3 &Y, IloNum& bestObjVal, NumArray3& bestValsX, NumArray3& bestValsY)
{

    bool isAllYInteger = false;
    PairIJ pairIJ = getDecisionVarWithMaxFractional(env, solver, X, Y, isAllYInteger);
    IloNum maxVal = solver.getValue(Y[pairIJ.i][pairIJ.j]);

    IloNum objVal = solver.getObjValue();
    if (isAllYInteger && isXYSatisfyConstraints(env, solver, X, Y))
    {
        if (solver.getObjValue() > bestObjVal)
        {
            bestObjVal = solver.getObjValue();
            env.out() << "\nbestObjVal: " << bestObjVal << std::endl;

            IloNumArray vals{ env };
            for (int i = 0; i < iSize; ++i)
            {
                solver.getValues(vals, X[i]);
                bestValsX[i] = vals.copy();

                env.out() << "X[" << i << "] = " << vals << std::endl;
            }

            for (int i = 0; i < iSize; ++i)
            {
                solver.getValues(vals, Y[i]);
                bestValsY[i] = vals.copy();
                env.out() << "Y[" << i << "] = " << vals << std::endl;
            }

            return;
        }
    }
    else if (isAllYInteger)
    {
        return;
    }

    IloModel model2{ env };
    for (IloModel::Iterator it(solver.getModel()); it.ok(); ++it)
        model2.add(*it);


    // Solve child1 model
    solver.getModel().add(Y[pairIJ.i][pairIJ.j] <= floor(maxVal));
    bool isSolver1Sucseed = solver.solve();
    IloNum objValue1 = solver.getObjValue();

    // Solve child2 model
    model2.add(Y[pairIJ.i][pairIJ.j] >= ceil(maxVal));

    IloCplex solver2(model2);
    bool isSolver2Sucseed = solver2.solve();
    IloNum objValue2 = solver2.getObjValue();

    if (isSolver1Sucseed && objValue1 > bestObjVal)
        SolveWithBranchAndBounds(env, solver, X, Y, bestObjVal, bestValsX, bestValsY);

    if (isSolver2Sucseed && objValue2 > bestObjVal)
        SolveWithBranchAndBounds(env, solver2, X, Y, bestObjVal, bestValsX, bestValsY);

}