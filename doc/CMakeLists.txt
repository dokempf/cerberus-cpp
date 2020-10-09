find_package(Doxygen)

if(DOXYGEN_FOUND)
  doxygen_add_docs(doxygen
    ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT Building doxygen documentation
  )
endif()
