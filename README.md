# Temporal Fast Downward Plan solver

[![GitHub Action
Status](https://github.com/IntelligentRoboticsLabs/plansys2_tfd_plan_solver/workflows/master/badge.svg)](https://github.com/IntelligentRoboticsLabs/plansys2_tfd_plan_solver)
[![codecov](https://codecov.io/gh/IntelligentRoboticsLabs/plansys2_tfd_plan_solver/master/graph/badge.svg)](https://codecov.io/gh/IntelligentRoboticsLabs/plansys2_tfd_plan_solver)

This package contains a plan solver that uses [Temporal Fast Downward](http://gki.informatik.uni-freiburg.de/tools/tfd/) for solving PDDL plans.

To install Temporal Fast Downward (TFD) to work with PlanSys2:

1. Pick an installation folder, e.g., `${HOME}/plansys2`
2. `mkdir -p ${HOME}/plansys2`
3. `cd ${HOME}/plansys2`
4. `git clone https://github.com/sea-bass/TemporalFastDownward.git`
5. `cd TemporalFastDownward`
6. `./build`
7. Export a `$TFD_HOME` environment variable that points to the `downward` directory, e.g., `export TFD_HOME=${HOME}/plansys2/TemporalFastDownward/downward`.
8. (Optional) Add the above export statement to your `~/.bashrc` file.

### Manual execution of TFD

1. `python3 $TFD_HOME/translate/translate.py [domain_name].pddl [problem_name].pddl`
2. `$TFD_HOME/preprocess/preprocess < output.sas`
3. `$TFD_HOME/search/search y Y a T 10 t 5 e r O 1 C 1 p $TFD_HOME/plan < output`
4. `cat pddlplan.1`
