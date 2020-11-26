add_executable(testvalidation EXCLUDE_FROM_ALL testvalidation.cc)
target_link_libraries(testvalidation yaml-cpp)
target_include_directories(testvalidation PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(testcustom EXCLUDE_FROM_ALL testcustom.cc)
target_link_libraries(testcustom yaml-cpp)
target_include_directories(testcustom PUBLIC ${CMAKE_SOURCE_DIR})

# Add a meta target build_tests to save typing
add_custom_target(build_tests)
add_dependencies(build_tests testvalidation)
add_dependencies(build_tests testcustom)

# Gather a list of test cases fron the cases subdirectory
file(GLOB testcases "${CMAKE_CURRENT_SOURCE_DIR}/cases/*.yml")
foreach(testcase ${testcases})
  get_filename_component(testname "${testcase}" NAME_WE)
  add_test(NAME testvalidation_${testname}
           COMMAND testvalidation ${testcase})
  add_test(NAME testcustom_${testname}
           COMMAND testcustom ${testcase})
endforeach()
