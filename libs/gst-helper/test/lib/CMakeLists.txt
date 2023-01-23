
find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)

#add_library( test-helper STATIC
#	
#	
#)
#
#set_target_properties( test-helper PROPERTIES 
#    CXX_STANDARD 17
#)
#
#target_include_directories( test-helper
#PUBLIC
#	${GSTREAMER_INCLUDE_DIRS}
#)
#
#target_link_libraries( test-helper
#PRIVATE
#	fmt::fmt
#
#	${GSTREAMER_LIBRARIES}
#
#	tcam::tcamprop
#PUBLIC
#	tcam::gst-helper
#)
#