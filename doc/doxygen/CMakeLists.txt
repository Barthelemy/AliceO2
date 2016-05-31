#
# Build the documentation
#
# @author  Barthélémy von Haller

# Add an option for the user to enable or not the documentation generation.
# Either set it on the command line or use ccmake. It is an advanced option.
option(BUILD_DOXYGEN "Build doxygen documentation" OFF)
mark_as_advanced(BUILD_DOXYGEN)
if (BUILD_DOXYGEN)
  find_package(Doxygen)

  if (NOT DOXYGEN_DOT_FOUND)
    message(WARNING "Graphviz doesn't seem to be installed. Doxygen will not be able to generate graphs. Consider installing this package.")
  endif (NOT DOXYGEN_DOT_FOUND)

  if (DOXYGEN_FOUND)
    message(STATUS "Documentation enabled")
    # Configure the doxygen config file with current settings:
    configure_file(doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY)

    add_custom_target(doxygen ALL
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation using doxygen" VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION share/o2)
  endif (DOXYGEN_FOUND)

endif ()
