find_package(PkgConfig REQUIRED QUIET)

find_package(GObject   REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)

find_package(GStreamer REQUIRED QUIET)


add_library(gsttcamic4src SHARED
  gst_tcam_ic4_src.cpp
  gst_tcam_ic4_src.h

  ic4_gst_conversions.h
  ic4_gst_conversions.cpp

  # gsttcambufferpool.h
  # gsttcambufferpool.cpp
  ic4_device_state.h
  ic4_device_state.cpp
  ic4_tcam_property.h
  ic4_tcam_property.cpp

  ic4src_gst_device_provider.cpp
  ic4src_gst_device_provider.h
  ic4src_gst_device.cpp
  ic4src_gst_device.h

  tcamic4src_plugin.cpp

  # tcambind.h
  # tcambind.cpp
)

target_include_directories(gsttcamic4src
  PRIVATE
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER_BASE_INCLUDE_DIRS}
  ${GSTREAMER_VIDEO_INCLUDE_DIRS}

   ${GLIB2_INCLUDE_DIR}
   ${GObject_INCLUDE_DIR}

   ${ic4_INCLUDE_PATH}

)

target_link_libraries( gsttcamic4src
PUBLIC
outcome::hl

  PRIVATE
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_VIDEO_LIBRARIES}

  ${GLIB2_LIBRARIES}
  ${GObject_LIBRARIES}
  ${INTROSPECTION_LIBS}

  ${ic4_LIBRARIES}
  # tcamgstbase
  tcam::gst-helper
  tcam::tcam-property
  # tcam::tcamgststatistics
  tcamprop1::provider_gobject
)

set_project_warnings(gsttcamic4src)

# helper funcion to receive the value of an arbitrary variable
function(pkg_check_variable _pkg _name)
  string(TOUPPER ${_pkg} _pkg_upper)
  string(TOUPPER ${_name} _name_upper)
  string(REPLACE "-" "_" _pkg_upper ${_pkg_upper})
  string(REPLACE "-" "_" _name_upper ${_name_upper})
  set(_output_name "${_pkg_upper}_${_name_upper}")

  execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${_name} ${_pkg}
    OUTPUT_VARIABLE _pkg_result
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(" ${PKG_CONFIG_EXECUTABLE} --variable=${_name} ${_pkg} ==> ${_pkg_result} in ${_output_name}")

  set("${_output_name}" "${_pkg_result}" CACHE STRING "pkg-config variable ${_name} of ${_pkg}" FORCE)
endfunction()


pkg_check_variable(gstreamer-1.0 pluginsdir)

if (GSTREAMER_1.0_PLUGINSDIR)
  message ("got pluginsdir")
  set(TCAM_INSTALL_GST_1_0 "${GSTREAMER_1.0_PLUGINSDIR}"
    CACHE PATH "gstreamer-1.0 module installation path")
else()
  message("using std path")
  set(TCAM_INSTALL_GST_1_0 "${CMAKE_INSTALL_PREFIX}/lib/gstreamer-1.0"
    CACHE PATH "gstreamer-1.0 module installation path")
endif()


install(TARGETS gsttcamic4src
  DESTINATION ${TCAM_INSTALL_GST_1_0}
  COMPONENT bin)
