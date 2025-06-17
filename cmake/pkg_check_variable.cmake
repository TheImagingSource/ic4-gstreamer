
# helper funcion to receive the value of an arbitrary variable
function(pkg_check_variable _pkg _name)

  find_package(PkgConfig REQUIRED QUIET)

  string(TOUPPER ${_pkg} _pkg_upper)
  string(TOUPPER ${_name} _name_upper)
  string(REPLACE "-" "_" _pkg_upper ${_pkg_upper})
  string(REPLACE "-" "_" _name_upper ${_name_upper})
  set(_output_name "${_pkg_upper}_${_name_upper}")

  execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${_name} ${_pkg}
    OUTPUT_VARIABLE _pkg_result
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  # message(" ${PKG_CONFIG_EXECUTABLE} --variable=${_name} ${_pkg} ==> ${_pkg_result} in ${_output_name}")

  set("${_output_name}" "${_pkg_result}" CACHE STRING "pkg-config variable ${_name} of ${_pkg}" FORCE)
endfunction()
