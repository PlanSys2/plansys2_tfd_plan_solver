# Temporal Fast Downward Plan solver

[![GitHub Action
Status](https://github.com/IntelligentRoboticsLabs/plansys2_tfd_plan_solver/workflows/master/badge.svg)](https://github.com/IntelligentRoboticsLabs/plansys2_tfd_plan_solver)
[![codecov](https://codecov.io/gh/IntelligentRoboticsLabs/plansys2_tfd_plan_solver/master/graph/badge.svg)](https://codecov.io/gh/IntelligentRoboticsLabs/plansys2_tfd_plan_solver)

This package contains a plan solver that uses [Temporal Fast Downward](http://gki.informatik.uni-freiburg.de/tools/tfd/) for solving PDDL plans.

To install `Temporal Fast TFD` in a the $TFD_HOME directory:

1. `mkdir -p $TFD_HOME`
2. `cd $TFD_HOME`
3. `wget "http://gki.informatik.uni-freiburg.de/tools/tfd/downloads/version-0.4/tfd-src-0.4.tgz"`
4. `tar xzf "tfd-src-0.4.tgz"`
5. `cd "tfd-src-0.4"`
6. `sed -e s/"-Werror"//g -i ./downward/search/Makefile`
7. `./build`
8. Add an `export TFD_HOME=` to ~.bashrc that points to the `downward` directory.

### Manual execution of TFD

1. `python2.7 $TFD_HOME/translate/translate.py [domain_name].pddl [problem_name].pddl`
2. `$TFD_HOME/preprocess/preprocess < output.sas`
3. `$TFD_HOME/search/search y Y a T 10 t 5 e r O 1 C 1 p $TFD_HOME/plan < output`
