#include <ilcplex/ilocplex.h>


const int iSize = 3;
const int jSize = 4;

int main()
{
    IloEnv env;
    IloModel model(env);


    // Variables
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


    // Decision variables
    IloNumVarArray X[] = {
        IloNumVarArray{ env, jSize, 0, numAircrafts[0], IloNumVar::Type::Float },
        IloNumVarArray{ env, jSize, 0, numAircrafts[1], IloNumVar::Type::Float },
        IloNumVarArray{ env, jSize, 0, numAircrafts[2], IloNumVar::Type::Float },
    };
    IloNumVarArray Y[] = {
        IloNumVarArray{ env, jSize, 0, numAircrafts[0], IloNumVar::Type::Int },
        IloNumVarArray{ env, jSize, 0, numAircrafts[1], IloNumVar::Type::Int },
        IloNumVarArray{ env, jSize, 0, numAircrafts[2], IloNumVar::Type::Int },
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
    IloRangeArray const_NumAircrafts{ env };
    IloRangeArray const_Qxt_Less_d{ env };
    IloRangeArray const_integer{ env };

    // Sum Y <= N
    for (int i = 0; i < iSize; ++i)
    {
        IloExpr expr{ env };
        for (int j = 0; j < jSize; ++j)
        {
            expr += Y[i][j];
        }
        const_NumAircrafts.add(expr <= numAircrafts[i]);
    }

    // Sum Q x t <= d
    for (int j = 0; j < jSize; ++j)
    {
        IloExpr expr{ env };
        for (int i = 0; i < iSize; ++i)
        {
            expr += capacity[i] * t_routes[i][j] * X[i][j];
        }
        const_Qxt_Less_d.add(expr <= d_goal[j]);
    }

    // x - y <= 0
    for (int i = 0; i < iSize; ++i)
    {
        for (int j = 0; j < jSize; ++j)
        {
            const_integer.add(X[i][j] - Y[i][j] <= 0);
        }
    }

    // Model
    model.add(IloMaximize(env, objectiveExpr));
    model.add(const_NumAircrafts);
    model.add(const_Qxt_Less_d);
    model.add(const_integer);

    // Solver
    IloCplex solver(model);
    solver.solve();

    env.out() << "Solution status = " << solver.getStatus() << std::endl;
    env.out() << "Solution value  = " << solver.getObjValue() << std::endl;


    IloNumArray vals(env);
    for (int i = 0; i < iSize; ++i)
    {
        solver.getValues(vals, X[i]);
        env.out() << "X[" << i << "] = " << vals << std::endl;
    }
    for (int i = 0; i < iSize; ++i)
    {
        solver.getValues(vals, Y[i]);
        env.out() << "Y[" << i << "] = " << vals << std::endl;
    }


    solver.getSlacks(vals, const_NumAircrafts);
    env.out() << "Slacks NumAircrafts  = " << vals << std::endl;
    solver.getSlacks(vals, const_Qxt_Less_d);
    env.out() << "Slacks Penalty       = " << vals << std::endl;
    solver.getSlacks(vals, const_integer);
    env.out() << "Slacks Integer       = " << vals << std::endl;

    getchar();
}