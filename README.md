# BB-Hungarian
This project implements the Branch-and-Bound algorithm for solving the Traveling Salesman Problem (TSP) in polynomial time.

Allowing subtours to the nature of the problem, it is treated as an Assignment problem and solved using the Hungarian algorithm.  
When branching the node, each son will prohibit one edge of the shorter subtour in the father's solution.  
A feasible solution for the TSP will be a node which has a solution with only one subtour, and the optimal solution will be the node with the best cost.

Compiling and Running
----------------------
A serial executable is obtained by using your favourite C++ compiler to compile and link the downloaded files.

On Ubuntu:

**make rebuild**

On Windows:

**g++ -O3 src/main.cpp src/data.cpp src/hungarian.cpp -o bb**

After done compiling, execute the 'bb' program passing the required arguments, like this:

**./bb ARG1 ARG2**

Arguments
----------
**ARG1**: Instance path, it should be like this: instances/name.tsp.  
**ARG2**: Search type, 1 for Depth-First Search (DFS) with backtracking and 2 for Breadth-First Search (BFS).  
