# ICFPC 2019 repository
 Team sanma
 
 # How to build
 
 ```
 $ cd src/
 $ make
 ```
 
 # How to use
 ## how to solve a problem
 
 ```
 $ ./src/solver run <engine_name> --desc ./dataset/problems/prob-001.desc --output ./solution/<engine_name>/prob-001.sol [--buy ./buy]
 ```
 Where `<engine_name>', which is listed in [engine_names.txt](https://github.com/nodchip/icfpc2019/blob/master/engine_names.txt). If the directory contains a file whose name is same with problem's file, i.e. `prob-001.buy`, it uses the file to buy boosters.
 
 ## how to solve all problems
 
 ```
 $ ENGINE_NAME=<engine_name> ./scripts/execute_engine.sh
 ```
 It runs the solver engine, and outputs solutions into `./solutions/<engine_name>`. If the new solution is better than the one under `./best_solution`, it updates the best solution.
 

# Team mates
- nodchip
- qhapaq
- peria
- fof
- tonagi
- tsuzuki
