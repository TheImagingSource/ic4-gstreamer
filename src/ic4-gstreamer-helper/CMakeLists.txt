
find_package(GStreamer REQUIRED QUIET)
find_package( CLI11 REQUIRED )

add_executable(ic4-gst-helper
  main.cpp
  base_functions.h
  base_functions.cpp
  print_caps.h
  print_caps.cpp
)
set_project_warnings(ic4-gst-helper)

target_include_directories(ic4-gst-helper PRIVATE ${GSTREAMER_INCLUDE_DIRS})


target_link_libraries( ic4-gst-helper
PRIVATE
	CLI11::CLI11
	${GSTREAMER_LIBRARIES}
	${GSTREAMER_BASE_LIBRARIES}
	${GSTREAMER_VIDEO_LIBRARIES}
)

install(TARGETS ic4-gst-helper
  DESTINATION ${IC4SRC_INSTALL_BIN}
  COMPONENT bin)
