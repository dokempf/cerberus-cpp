include_directories(${CMAKE_SOURCE_DIR})
link_libraries(yaml-cpp)

add_executable(first EXCLUDE_FROM_ALL first.cc)
