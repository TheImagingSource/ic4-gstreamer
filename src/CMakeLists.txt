

add_subdirectory(ic4-gstreamer-helper)


find_package(GObject   REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)

find_package(GStreamer REQUIRED QUIET)
find_package(fmt REQUIRED QUIET)


add_library(gstic4src SHARED
  gst_tcam_ic4_src.cpp
  gst_tcam_ic4_src.h

  ic4_gst_conversions.h
  ic4_gst_conversions.cpp

  ic4_device_state.h
  ic4_device_state.cpp
  ic4_tcam_property.h
  ic4_tcam_property.cpp

  ic4src_gst_device_provider.cpp
  ic4src_gst_device_provider.h
  ic4src_gst_device.cpp
  ic4src_gst_device.h
)

target_include_directories(gstic4src
  PRIVATE
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER_BASE_INCLUDE_DIRS}
  ${GSTREAMER_VIDEO_INCLUDE_DIRS}

  ${GLIB2_INCLUDE_DIR}
  ${GObject_INCLUDE_DIR}

)

target_link_libraries( gstic4src

  PRIVATE
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_VIDEO_LIBRARIES}

  ${GLIB2_LIBRARIES}
  ${GObject_LIBRARIES}
  ${INTROSPECTION_LIBS}

  ic4::core
  fmt::fmt
  tcam::gst-helper
  tcam::tcam-property
  tcam::tcamgststatistics
  tcamprop1::provider_gobject

  -static-libstdc++
  -static-libgcc
)

target_compile_definitions(gstic4src PRIVATE -DIC4SRC_VERSION="${IC4SRC_VERSION}")

set_property(TARGET gstic4src PROPERTY VERSION "${IC4SRC_VERSION}")
set_property(TARGET gstic4src PROPERTY SOVERSION "${IC4SRC_VERSION}")

set_project_warnings(gstic4src)



install(TARGETS gstic4src
  DESTINATION ${IC4SRC_INSTALL_GST_1_0}
  COMPONENT bin)
