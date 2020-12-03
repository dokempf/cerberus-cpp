if(BUILD_TESTING)
  add_executable(testcerberus EXCLUDE_FROM_ALL testcerberus.cc)
  target_link_libraries(testcerberus yaml-cpp Catch2::Catch2)
  target_include_directories(testcerberus PUBLIC ${CMAKE_SOURCE_DIR})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testdata.yml ${CMAKE_CURRENT_BINARY_DIR}/testdata.yml)
