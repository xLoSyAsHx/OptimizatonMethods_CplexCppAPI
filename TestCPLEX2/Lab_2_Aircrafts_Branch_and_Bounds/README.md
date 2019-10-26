## Branch & Bounds. CPLEX C++ API


### Algorithm

+ Solve current model
+ Check if the model satisfy all constraints - possible solution found. Need to stop
+ Otherwise:
 + Find decision var with greatest fractional part
 + Create two new models in which founded variable will be:
   + X <= floor(X_value)
   + X >= ceil(X_value)
  + Recursevily execute algorithm for 2 new models

### Output parameters
**bestObjVal** - The greatest objective value (in maximization case)  
**bestValsX** - Array of IloNumArray. Contains solutions for X  
**bestValsY** - Array of IloNumArray. Contains solutions for Y  

### Result for modified values and constraints

Best object value: 102000

X[0] = [11, 2, 9, 5]  
X[1] = [0,  0, 0, 15.8(3)]  
X[2] = [0,  0, 0, 0]

Y[0] = [11, 2, 9,  5]  
Y[1] = [0,  0, 0, 16]  
Y[2] = [0,  0, 0,   0]
