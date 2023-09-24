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

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>

#include "plansys2_msgs/msg/plan_item.hpp"
#include "plansys2_tfd_plan_solver/tfd_plan_solver.hpp"

namespace plansys2
{

TFDPlanSolver::TFDPlanSolver()
{
}

void
TFDPlanSolver::configure(
  rclcpp_lifecycle::LifecycleNode::SharedPtr lc_node,
  const std::string & plugin_name)
{
  lc_node_ = lc_node;

  output_dir_parameter_name_ = plugin_name + ".output_dir";
  lc_node_->declare_parameter<std::string>(
    output_dir_parameter_name_, std::filesystem::temp_directory_path());

  char * planner_path = getenv("TFD_HOME");
  if (planner_path == NULL) {
    RCLCPP_FATAL(
      lc_node_->get_logger(), "'TFD_HOME' environment variable not defined for %s",
      plugin_name.c_str());
    exit(-1);
  }
  tfd_path_ = std::string(planner_path);
}

std::optional<plansys2_msgs::msg::Plan>
TFDPlanSolver::getPlan(
  const std::string & domain, const std::string & problem,
  const std::string & node_namespace)
{
  if (system(nullptr) == 0) {
    return {};
  }

  auto output_dir = std::filesystem::path(
    lc_node_->get_parameter(output_dir_parameter_name_).value_to_string()
  );

  if (node_namespace != "") {
    for (auto p : std::filesystem::path(node_namespace) ) {
      if (p != std::filesystem::current_path().root_directory()) {
        output_dir /= p;
      }
    }
    std::filesystem::create_directories(output_dir);
  }
  RCLCPP_INFO(
    lc_node_->get_logger(), "Writing planning results to %s.", output_dir.string().c_str());

  int status;
  plansys2_msgs::msg::Plan ret;

  const auto domain_file_path = output_dir / std::filesystem::path("domain.pddl");
  std::ofstream domain_out(domain_file_path);
  domain_out << domain;
  domain_out.close();

  const auto problem_file_path = output_dir / std::filesystem::path("problem.pddl");
  std::ofstream problem_out(problem_file_path);
  problem_out << problem;
  problem_out.close();

  // Translate the domain and problem files to SAS.
  const auto output_sas_file_path = output_dir / std::filesystem::path("output.sas");
  status = system(
    (tfd_path_ + "/translate/translate.py " +
    domain_file_path.string() + " " + problem_file_path.string()).c_str()
  );
  if (status == -1) {
    RCLCPP_ERROR_STREAM(lc_node_->get_logger(), "Failed to translate domain and problem files.");
    return {};
  }
  system(("mv output.sas " + output_sas_file_path.string()).c_str());

  // Preprocess the translated files.
  const auto processed_output_file_path = output_dir / std::filesystem::path("output");
  status = system(
    (tfd_path_ + "/preprocess/preprocess < " + output_sas_file_path.string()).c_str()
  );
  if (status == -1) {
    RCLCPP_ERROR_STREAM(lc_node_->get_logger(), "Failed to preprocess files.");
    return {};
  }
  system(("mv output " + processed_output_file_path.string()).c_str());

  // Search for a plan using TFD
  const auto pddlplan_file_path = output_dir / std::filesystem::path("pddlplan");
  status = system(
    (tfd_path_ + "/search/search y Y a T 10 t 5 e r O 1 C 1 p " +
    pddlplan_file_path.string() + " < " + processed_output_file_path.string()).c_str()
  );
  if (status == -1) {
    RCLCPP_ERROR_STREAM(lc_node_->get_logger(), "Failed to search for a plan.");
    return {};
  }

  std::string line;
  std::ifstream plan_file(pddlplan_file_path.string() + ".1");
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

  // Save all the output files with a ".last" mangle.
  system(
    ("mv " + output_sas_file_path.string() + " " +
    output_sas_file_path.string() + ".last").c_str());
  system(
    ("mv " + processed_output_file_path.string() + " " +
    processed_output_file_path.string() + ".last").c_str());
  system(
    ("mv " + pddlplan_file_path.string() + ".1 " +
    pddlplan_file_path.string() + ".1.last").c_str());

  if (ret.items.empty()) {
    return {};
  } else {
    return ret;
  }
}

bool
TFDPlanSolver::isDomainValid(
  const std::string & domain,
  const std::string & node_namespace)
{
  if (system(nullptr) == 0) {
    return false;
  }

  auto output_dir = std::filesystem::path(
    lc_node_->get_parameter(output_dir_parameter_name_).value_to_string()
  );

  if (node_namespace != "") {
    for (auto p : std::filesystem::path(node_namespace) ) {
      if (p != std::filesystem::current_path().root_directory()) {
        output_dir /= p;
      }
    }
    std::filesystem::create_directories(output_dir);
  }
  RCLCPP_INFO(
    lc_node_->get_logger(), "Writing domain validation results to %s.",
    output_dir.string().c_str());

  int status;
  plansys2_msgs::msg::Plan ret;

  const auto domain_file_path = output_dir / std::filesystem::path("domain.pddl");
  std::ofstream domain_out(domain_file_path);
  domain_out << domain;
  domain_out.close();

  // TFD requires the problem to match the domain's problem name.
  // Get the domain name to put together a valid problem.
  std::regex regexp("\\(\\s*domain\\s*([^\\)\\s]+)\\s*\\)");
  std::smatch regex_matches;
  std::regex_search(domain, regex_matches, regexp);
  if (regex_matches.size() != 2u) {
    RCLCPP_ERROR(lc_node_->get_logger(), "Could not extract domain name from domain file.");
    return false;
  }
  const std::string domain_name = regex_matches[1];

  const auto problem_file_path = output_dir / std::filesystem::path("problem.pddl");
  std::ofstream problem_out(problem_file_path);
  problem_out << "(define (problem void) (:domain "
              << domain_name << ") (:objects) (:init) (:goal none))";
  problem_out.close();

  // Translate the domain and problem files to SAS.
  // If this is successful, the domain is considered validated.
  const auto output_val_file_path = output_dir / std::filesystem::path("output.sas.validation");
  status = system(
    (tfd_path_ + "/translate/translate.py " +
    domain_file_path.string() + " " + problem_file_path.string()).c_str()
  );
  if (status == -1) {
    RCLCPP_ERROR_STREAM(lc_node_->get_logger(), "Failed to translate domain and problem files.");
    return false;
  }
  system(("mv output.sas " + output_val_file_path.string()).c_str());

  return true;
}

}  // namespace plansys2

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(plansys2::TFDPlanSolver, plansys2::PlanSolverBase);
