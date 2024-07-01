cmake_minimum_required(VERSION 3.11)
cmake_policy(SET CMP0077 NEW)

project(cerberus-cpp LANGUAGES CXX)

set(CERBERUS_CPP_MAIN_PROJECT OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(CERBERUS_CPP_MAIN_PROJECT ON)
endif()

option(CERBERUS_CPP_FIND_YAML_CPP "Enable find_package(yaml-cpp)." ON)
option(CERBERUS_CPP_INSTALL "Enable generation of cerberus-cpp install targets" ${CERBERUS_CPP_MAIN_PROJECT})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# When building documentation from readthedocs.io, we need to
# run CMake, but we do not need to build C++ code, so we should
# skip any required dependencies.
set(REQUIRED_STRING REQUIRED)
if(DOCS_ONLY)
  set(REQUIRED_STRING "")
endif()

if(${CERBERUS_CPP_FIND_YAML_CPP})
  find_package(yaml-cpp 0.6 ${REQUIRED_STRING})
endif()

# Add library target that can be linked against
add_library(cerberus-cpp INTERFACE)
target_include_directories(
  cerberus-cpp
  INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(cerberus-cpp INTERFACE yaml-cpp)

# Add an alias target for use if this project is included as a subproject in another project
add_library(cerberus-cpp::cerberus-cpp ALIAS cerberus-cpp)

# Add documentation building
add_subdirectory(doc)

# Add the testing subdirectories
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ext/Catch2/CMakeLists.txt)
  include(CTest)
  add_subdirectory(ext/Catch2)
  add_subdirectory(test)
endif()

# Installation rules
include(GNUInstallDirs)

if (CERBERUS_CPP_INSTALL)
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
endif()

include(FeatureSummary)
feature_summary(WHAT ALL)
