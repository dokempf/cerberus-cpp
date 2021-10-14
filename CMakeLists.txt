project(cerberus-cpp LANGUAGES CXX)

cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ext/Catch2)
include(CTest)

# When building documentation from readthedocs.io, we need to
# run CMake, but we do not need to build C++ code, so we should
# skip any required dependencies.
set(REQUIRED_STRING REQUIRED)
if(DOCS_ONLY)
  set(REQUIRED_STRING "")
endif()

find_package(yaml-cpp 0.6 ${REQUIRED_STRING})

# Add library target that can be linked against
add_library(cerberus-cpp INTERFACE)
target_include_directories(
  cerberus-cpp
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(cerberus-cpp INTERFACE yaml-cpp)

# Add an alias target for use if this project is included as a subproject in another project
add_library(cerberus-cpp::cerberus-cpp ALIAS cerberus-cpp)

# Documentation and testing subdirectories
add_subdirectory(doc)
add_subdirectory(test)

# Installation rules
include(GNUInstallDirs)

install(
  TARGETS cerberus-cpp
  EXPORT cerberus-cpp-config
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  EXPORT cerberus-cpp-config
  NAMESPACE cerberus-cpp::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cerberus-cpp
)

install(
  DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

include(FeatureSummary)
feature_summary(WHAT ALL)
