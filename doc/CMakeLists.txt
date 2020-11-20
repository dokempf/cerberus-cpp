add_subdirectory(examples)

find_package(Doxygen)

if(DOXYGEN_FOUND)
  set(DOXYGEN_GENERATE_XML YES)
  doxygen_add_docs(doxygen
    ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT Building doxygen documentation
  )

  # TODO: Properly find sphinx and other Python dependencies
  add_custom_target(sphinx-doc
                    COMMAND
                    sphinx-build -b html
                    -Dbreathe_projects.cerberus-cpp="${CMAKE_CURRENT_BINARY_DIR}/xml"
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/sphinx
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMENT "Generating documentation with Sphinx")

  add_dependencies(sphinx-doc doxygen)
endif()
