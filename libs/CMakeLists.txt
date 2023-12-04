

find_package(fmt CONFIG REQUIRED)
add_subdirectory(outcome )

# find_package(outcome CONFIG REQUIRED)
add_library( outcome::outcome ALIAS outcome )
add_library( outcome::hl ALIAS outcome )
# add_subdirectory(dutils_image )
add_subdirectory( tcam-property EXCLUDE_FROM_ALL )

add_subdirectory(gst-helper EXCLUDE_FROM_ALL)
