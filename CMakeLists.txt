project(cerberus-cpp LANGUAGES CXX)

cmake_minimum_required(VERSION 3.11)

include(CTest)

# When building documentation from readthedocs.io, we need to
# run CMake, but we do not need to build C++ code, so we should
# skip any required dependencies.
set(REQUIRED_STRING REQUIRED)
if(DOCS_ONLY)
  set(REQUIRED_STRING "")
endif()

find_package(yaml-cpp 0.6 ${REQUIRED_STRING})
message("-- yaml-cpp version detected: ${yaml-cpp_VERSION}")

add_subdirectory(cerberus-cpp)
add_subdirectory(doc)
add_subdirectory(test)

include(FeatureSummary)
feature_summary(WHAT ALL)
