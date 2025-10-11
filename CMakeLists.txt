project(plansys2_tfd_plan_solver)

cmake_minimum_required(VERSION 3.5)

find_package(Threads REQUIRED)

find_package(ament_cmake REQUIRED)
find_package(plansys2_core REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} SHARED
  src/plansys2_tfd_plan_solver/tfd_plan_solver.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
)
target_link_libraries(${PROJECT_NAME}
  PUBLIC
  rclcpp::rclcpp
  rclcpp_lifecycle::rclcpp_lifecycle
  plansys2_core::plansys2_core
  PRIVATE
  pluginlib::pluginlib
)

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_index_cpp REQUIRED)
  add_subdirectory(test)
endif()

ament_export_include_directories(include/${PROJECT_NAME})
ament_export_libraries(${PROJECT_NAME})
ament_export_targets(${PROJECT_NAME})
ament_export_dependencies(
  ament_index_cpp
  plansys2_core
  pluginlib
  rclcpp
  rclcpp_lifecycle
)

pluginlib_export_plugin_description_file(plansys2_core plansys2_tfd_plan_solver_plugin.xml)

ament_package()