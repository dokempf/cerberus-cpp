if(BUILD_TESTING)
  add_executable(testcerberus testcerberus.cc)
  target_link_libraries(testcerberus PUBLIC cerberus-cpp Catch2::Catch2)
  include(../ext/Catch2/contrib/Catch.cmake)
  catch_discover_tests(testcerberus)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testdata.yml ${CMAKE_CURRENT_BINARY_DIR}/testdata.yml)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/illformedschemas.yml ${CMAKE_CURRENT_BINARY_DIR}/illformedschemas.yml)
