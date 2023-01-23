
find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)


add_executable( check-tcamprop
	main.cpp
	cli-parser.h
	cli-parser.cpp
	validate_prop_names.h
	validate_prop_names.cpp

	stuff.h
	stuff.cpp
)

set_target_properties( check-tcamprop PROPERTIES 
    CXX_STANDARD 17
)

target_include_directories( check-tcamprop
PRIVATE
	${GSTREAMER_INCLUDE_DIRS}
)

target_link_libraries( check-tcamprop
PRIVATE
	fmt::fmt

	stdc++fs

	${GSTREAMER_LIBRARIES}

	tcamprop1::consumer

	gst-helper-libs-warning-base

	pugixml::pugixml
)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0") 
    target_precompile_headers( check-tcamprop PRIVATE
		"../lib/CLI11.hpp"
		<vector>
		<string>
		<string_view>
		<algorithm>
		<fmt/format.h>
	)
endif()

add_custom_command( TARGET check-tcamprop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/*.xml" $<TARGET_FILE_DIR:check-tcamprop>
    )