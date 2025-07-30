

add_executable(test_ic4src
  main.cpp
  test_helper.h
  test_helper.cpp
  test_device_monitor.cpp
  test_caps_negotiation.cpp
  test_properties.cpp
)

find_package(doctest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

find_package(GObject   REQUIRED QUIET)
find_package(GLIB2     REQUIRED QUIET)
find_package(GObjectIntrospection REQUIRED QUIET)

find_package(GStreamer REQUIRED QUIET)

target_link_libraries(test_ic4src
  PRIVATE
  doctest::doctest
  fmt::fmt
  ${GSTREAMER_LIBRARIES}
  ${GSTREAMER_BASE_LIBRARIES}
  ${GSTREAMER_VIDEO_LIBRARIES}

  ${GLIB2_LIBRARIES}
  ${GObject_LIBRARIES}
  ${INTROSPECTION_LIBS}
  tcam::tcam-property

)


target_include_directories(test_ic4src
  PRIVATE
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER_BASE_INCLUDE_DIRS}
  ${GSTREAMER_VIDEO_INCLUDE_DIRS}

  ${GLIB2_INCLUDE_DIR}
  ${GObject_INCLUDE_DIR}

)

add_test(
  NAME Test_ic4src
  COMMAND test_ic4src
)
