
  find_package(Sphinx REQUIRED)

  if(NOT DEFINED SPHINX_THEME)
    set(SPHINX_THEME default)
  endif()

  if(NOT DEFINED SPHINX_THEME_DIR)
    set(SPHINX_THEME_DIR)
  endif()


  # configured documentation tools and intermediate build results
  set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")

  # Sphinx cache with pickled ReST documents
  set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

  # HTML output directory
  set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
    "${BINARY_BUILD_DIR}/conf.py"
    @ONLY)


  add_custom_target(doc ALL
    ${SPHINX_EXECUTABLE}
    -q -b html
    -c "${BINARY_BUILD_DIR}"
    -d "${SPHINX_CACHE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/pages"
    "${SPHINX_HTML_DIR}"
    COMMENT "Generate HTML Documentation with Sphinx"
    VERBATIM)

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
    DESTINATION "${IC4SRC_INSTALL_DOC}"
    COMPONENT bin
    PATTERN ".nojekyll" EXCLUDE
    PATTERN ".buildinfo" EXCLUDE
    )

  # This adds the doxygen output directories to the 'make clean' command
  set_property(DIRECTORY APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/latex"
    "${CMAKE_CURRENT_BINARY_DIR}/html"
    "${CMAKE_CURRENT_BINARY_DIR}/man")
