#include <ilcplex/ilocplex.h>


int main()
{
    IloEnv env;
    IloModel model(env);

    // Env
    IloNumVar x1{ env, 0, 120 };
    x1.setName("X1: Num of produced chairs");
    IloNumVar x2{ env, 0, 60 };
    x2.setName("X2: Num of produced desks");

    // Con
    IloRangeArray constraints{ env };
    constraints.add(x1 + 15.f / 11.f * x2 <= 150);
    constraints[0].setName("Constraint 1: from paint department");

    constraints.add(x1 + 2.5f * x2 <= 200);
    constraints[1].setName("Constraint 2: from sawing department");

    // Model
    model.add(IloMaximize(env, 50 * x1 + 100 * x2));
    model.add(constraints);

    // Solver
    IloCplex solver{ model };
    solver.solve();


    IloNumArray vals{ env };
    env.out() << "\nSolution status = " << solver.getStatus() << std::endl;
    env.out() << "Solution value  = " << solver.getObjValue() << std::endl;

    env.out() << "X1 value: " << solver.getValue(x1) << std::endl;
    env.out() << "X2 value: " << solver.getValue(x2) << std::endl;

    getchar();
}