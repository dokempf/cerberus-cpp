add_executable(testvalidation EXCLUDE_FROM_ALL testvalidation.cc)
target_link_libraries(testvalidation yaml-cpp)
target_include_directories(testvalidation PUBLIC ${CMAKE_SOURCE_DIR})

add_test(NAME testvalidation
         COMMAND testvalidation)
