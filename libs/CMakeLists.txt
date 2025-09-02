
# find_package(tcam-property REQUIRED)
# find_package(PkgConfig)

find_package(fmt CONFIG REQUIRED)
add_subdirectory(outcome )

add_library( outcome::outcome ALIAS outcome )
add_library( outcome::hl ALIAS outcome )

add_subdirectory(gst-helper EXCLUDE_FROM_ALL)
