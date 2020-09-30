project(cerberus-cpp LANGUAGES CXX)

cmake_minimum_required(VERSION 3.11)

find_package(yaml-cpp REQUIRED)
find_package(PythonLibs)

# link_libraries(yaml-cpp)
#include_directories(${yaml-cpp_INCLUDE_DIR})

add_subdirectory(cerberus-cpp)
add_subdirectory(test)

include(FeatureSummary)
feature_summary(WHAT ALL)
