
find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)


add_executable( test-tcamcam
	main.cpp

	tcampimipi_property_test.cpp

	doctest.h
)

set_target_properties( test-tcamcam PROPERTIES 
    CXX_STANDARD 17
)

target_include_directories( test-tcamcam
PRIVATE
	${GSTREAMER_INCLUDE_DIRS}
)

target_link_libraries( test-tcamcam
PRIVATE
	fmt::fmt

	${GSTREAMER_LIBRARIES}

	gst-helper-libs-warning-base

	tcam::gst-helper
	tcam::tcam-property
)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0") 
    target_precompile_headers( test-tcamcam PRIVATE
		<vector>
		<string>
		<string_view>
		<algorithm>
		<fmt/format.h>
	)
endif()
