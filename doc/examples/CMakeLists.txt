include_directories(${CMAKE_SOURCE_DIR})
link_libraries(yaml-cpp)

add_executable(first EXCLUDE_FROM_ALL first.cc)
add_executable(oddrule EXCLUDE_FROM_ALL oddrule.cc)
add_executable(datetype EXCLUDE_FROM_ALL datetype.cc)
add_executable(registerschema EXCLUDE_FROM_ALL registerschema.cc)
add_executable(normalization EXCLUDE_FROM_ALL normalization.cc)

add_custom_target(examples)
add_dependencies(examples first oddrule datetype registerschema normalization)
