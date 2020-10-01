add_executable(testvalidation EXCLUDE_FROM_ALL testvalidation.cc)
target_link_libraries(testvalidation yaml-cpp)
target_include_directories(testvalidation PUBLIC ${CMAKE_SOURCE_DIR})

# Gather a list of test cases fron the cases subdirectory
file(GLOB testcases "${CMAKE_CURRENT_SOURCE_DIR}/cases/*.yml")
foreach(testcase ${testcases})
  get_filename_component(testname "${testcase}" NAME_WE)
  add_test(NAME testvalidation_${testname}
           COMMAND testvalidation ${testcase})
endforeach()
