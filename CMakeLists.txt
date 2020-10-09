project(cerberus-cpp LANGUAGES CXX)

cmake_minimum_required(VERSION 3.11)

include(CTest)

find_package(yaml-cpp REQUIRED)
find_package(PythonLibs)

add_subdirectory(cerberus-cpp)
add_subdirectory(doc)
add_subdirectory(test)

include(FeatureSummary)
feature_summary(WHAT ALL)
