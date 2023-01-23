
find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)


add_executable( gst-simple-appsink
	main.cpp
)

set_target_properties( gst-simple-appsink PROPERTIES 
    CXX_STANDARD 17
)


target_include_directories( gst-simple-appsink
PRIVATE
	${GSTREAMER_INCLUDE_DIRS}
)

target_link_libraries( gst-simple-appsink
PRIVATE
	fmt::fmt

	stdc++fs

	${GSTREAMER_LIBRARIES}

	tcam::tcam-property
	tcam::gst-helper
)

