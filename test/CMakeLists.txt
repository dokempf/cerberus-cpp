if(BUILD_TESTING)
  add_executable(testcustom EXCLUDE_FROM_ALL testcustom.cc)
  target_link_libraries(testcustom yaml-cpp Catch2::Catch2)
  target_include_directories(testcustom PUBLIC ${CMAKE_SOURCE_DIR})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testdata.yml ${CMAKE_CURRENT_BINARY_DIR}/testdata.yml)
