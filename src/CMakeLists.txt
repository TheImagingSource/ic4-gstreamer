
find_package(GObject   REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)

find_package(GStreamer REQUIRED QUIET)


add_library(gsttcamic4src SHARED
  gst_tcam_ic4_src.cpp
  gst_tcam_ic4_src.h

  ic4_gst_conversions.h
  ic4_gst_conversions.cpp
  # mainsrc_tcamprop_impl.h
  # mainsrc_tcamprop_impl.cpp
  # mainsrc_device_state.h
  # mainsrc_device_state.cpp
  # tcamsrc_tcamprop_impl.h
  # tcamsrc_tcamprop_impl.cpp

  # gsttcambufferpool.h
  # gsttcambufferpool.cpp

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
  # tcamprop1::provider_gobject
)

set_project_warnings(gsttcamic4src)

install(TARGETS gsttcamic4src
  DESTINATION ${TCAM_INSTALL_GST_1_0}
  COMPONENT bin)
