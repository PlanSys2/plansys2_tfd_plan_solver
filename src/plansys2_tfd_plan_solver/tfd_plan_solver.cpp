// Copyright 2019 Intelligent Robotics Lab
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <sys/stat.h>
#include <sys/types.h>

#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "plansys2_msgs/msg/plan_item.hpp"
#include "plansys2_tfd_plan_solver/tfd_plan_solver.hpp"

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(plansys2::TFDPlanSolver, plansys2::PlanSolverBase);

namespace plansys2
{

TFDPlanSolver::TFDPlanSolver()
{
}

void
TFDPlanSolver::configure(rclcpp_lifecycle::LifecycleNode::SharedPtr & node, const std::string & id)
{
  char * planner_path = getenv("TFD_HOME");

  if (planner_path == NULL) {
    RCLCPP_FATAL(node->get_logger(), "'TFD_HOME' end not defined for %s", id.c_str());
    exit(-1);
  }

  tfd_path_ = std::string(planner_path);

  RCLCPP_INFO(node->get_logger(), "Planner path set to: %s", tfd_path_.c_str());
}

std::optional<plansys2_msgs::msg::Plan>
TFDPlanSolver::getPlan(
  const std::string & domain, const std::string & problem,
  const std::string & node_namespace)
{
  if (node_namespace != "") {
    //  This doesn't work as cxx flags must apper at end of link options, and I didn't
    //  find a way
    // std::experimental::filesystem::create_directories("/tmp/" + node_namespace);
    mkdir(("/tmp/" + node_namespace).c_str(), ACCESSPERMS);
  }

  plansys2_msgs::msg::Plan ret;
  std::ofstream domain_out("/tmp/" + node_namespace + "/domain.pddl");
  domain_out << domain;
  domain_out.close();

  std::ofstream problem_out("/tmp/" + node_namespace + "/problem.pddl");
  problem_out << problem;
  problem_out.close();

  system(
    (tfd_path_ + "/translate/translate.py " +
    "/tmp/" + node_namespace + "/domain.pddl " +
    "/tmp/" + node_namespace + "/problem.pddl").c_str()
  );

  system(
    ("mv output.sas /tmp/" + node_namespace).c_str()
  );

  system(
    (tfd_path_ + "/preprocess/preprocess < /tmp/output.sas").c_str()
  );

  system(
    ("mv output /tmp/" + node_namespace).c_str()
  );

  system(
    (tfd_path_ + "/search/search y Y a T 10 t 5 e r O 1 C 1 p /tmp/" +
    node_namespace + "/pddlplan < /tmp/" + node_namespace + "/output").c_str()
  );

  std::string line;
  std::ifstream plan_file("/tmp/" + node_namespace + "/pddlplan.1");
  bool solution = false;

  if (plan_file.is_open()) {
    while (getline(plan_file, line)) {
      plansys2_msgs::msg::PlanItem item;
      size_t colon_pos = line.find(":");
      size_t colon_par = line.find(")");
      size_t colon_bra = line.find("[");

      std::string time = line.substr(0, colon_pos);
      std::string action = line.substr(colon_pos + 2, colon_par - colon_pos - 1);
      std::string duration = line.substr(colon_bra + 1);
      duration.pop_back();

      item.time = std::stof(time);
      item.action = action;
      item.duration = std::stof(duration);

      ret.items.push_back(item);
    }
    plan_file.close();
  }

  system("mv /tmp/output /tmp/output.last");
  system("mv /tmp/pddlplan.1 /tmp/pddlplan.1.last");
  system("mv /tmp/output.sas /tmp/output.sas.last");

  if (ret.items.empty()) {
    return {};
  } else {
    return ret;
  }
}

}  // namespace plansys2
