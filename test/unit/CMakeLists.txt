ament_add_gtest(tfd_test tfd_test.cpp)
target_link_libraries(tfd_test
  ${PROJECT_NAME}
  ament_index_cpp::ament_index_cpp
  dl
)