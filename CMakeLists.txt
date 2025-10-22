

#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 3.16)

#Name your project here
project(ic4-gstreamer LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# include(version.cmake)
set(IC4SRC_VERSION_MAJOR 1)
set(IC4SRC_VERSION_MINOR 0)
set(IC4SRC_VERSION_PATCH 0)
set(IC4SRC_VERSION_MODIFIER "")

set(IC4SRC_VERSION "${IC4SRC_VERSION_MAJOR}.${IC4SRC_VERSION_MINOR}.${IC4SRC_VERSION_PATCH}${IC4SRC_VERSION_MODIFIER}" CACHE STRING "Version number")


set(IC4SRC_ORIGIN "https://theimagingsource.com")

option(IC4_ENABLE_TESTS "Enable unit/integration tests" OFF)
option(IC4_ENABLE_TCAM_PROP  "Compile ic4src with tcam-property interface." ON)
option(IC4_ENABLE_TCAM_STATS "Compile ic4src with gsttcamstatistics." ON)

### installation paths


set(CMAKE_INSTALL_PREFIX "/usr")

set(IC4SRC_INSTALL_BIN "${CMAKE_INSTALL_PREFIX}/bin"
  CACHE PATH "Executable installation directory." FORCE)
set(IC4SRC_INSTALL_DATA "${CMAKE_INSTALL_PREFIX}/share/theimagingsource/ic4src/"
  CACHE PATH "Data prefix.")

set(IC4SRC_INSTALL_DOC "${CMAKE_INSTALL_PREFIX}/share/theimagingsource/ic4src/doc/"
  CACHE PATH "Documentation installation directory.")
set(IC4SRC_INSTALL_LICENCE_DIR "${IC4SRC_INSTALL_DATA}/copyright/"
  CACHE PATH "Licence installation directory.")

include("cmake/pkg_check_variable.cmake")

pkg_check_variable(gstreamer-1.0 pluginsdir)

if (GSTREAMER_1.0_PLUGINSDIR)
  message ("got pluginsdir")
  set(IC4SRC_INSTALL_GST_1_0 "${GSTREAMER_1.0_PLUGINSDIR}"
    CACHE PATH "gstreamer-1.0 module installation path" FORCE)
else()
  message("using std path")
  set(IC4SRC_INSTALL_GST_1_0 "${CMAKE_INSTALL_PREFIX}/lib/gstreamer-1.0"
    CACHE PATH "gstreamer-1.0 module installation path" FORCE)
endif()

### end paths

include( StandardProjectSettings )

include("cmake/CompilerWarnings.cmake")
include("cmake/git-helper.cmake")

set(default_build_type "Release")   # Override the default build type to be 'Release' and not 'RelWithDebInfo'
include("cmake/StandardProjectSettings.cmake")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)


set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_git_settings() # exports GIT_COMMIT_COUNT, GIT_TAG, ...

set(IC4SRC_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(IC4SRC_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${IC4SRC_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${IC4SRC_BINARY_DIR}/lib)

if( ${GIT_REPO_PRESENT} )
  message(STATUS "IC4SRC_VERSION: ${IC4SRC_VERSION}, Git: commit#=${GIT_COMMIT_COUNT}, branch=${GIT_BRANCH} hash=${GIT_COMMIT_HASH}" )

  set(IC4SRC_VERSION_FULL "${IC4SRC_VERSION}.${GIT_COMMIT_COUNT}" CACHE STRING "Full version number")

else()
  message(STATUS "IC4SRC_VERSION: ${IC4SRC_VERSION}, Git: No repo found" )

  set(IC4SRC_VERSION_FULL "${IC4SRC_VERSION}" CACHE STRING "Full version number")
endif()

# All option for configuration come from here
# include(CMakeOptions.cmake)

add_compile_options( -Wno-psabi )   # disable ABI change message of g++8
add_compile_options( -Wall -Wextra -pedantic -pthread )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options( -fsized-deallocation )
endif()

set( CMAKE_CXX_STANDARD 23 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_POSITION_INDEPENDENT_CODE ON )


install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doc/ic4src.md
   DESTINATION "${IC4SRC_INSTALL_DOC}")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/doc/ic4-gst-helper.md
   DESTINATION "${IC4SRC_INSTALL_DOC}")


install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
  DESTINATION "${IC4SRC_INSTALL_LICENCE_DIR}")

find_package(ic4 REQUIRED)

set(ic4_INCLUDE_PATH "${IC4_INCLUDE_PATH}")
set(ic4_LIBRARY_PATH "")
set(ic4_LIBRARIES "${IC4_LIBRARIES}")


add_subdirectory(libs)
add_subdirectory(src)

if (IC4SRC_ENABLE_TESTS)
  include(CTest)
  add_subdirectory(test)
endif (IC4SRC_ENABLE_TESTS)


configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/env.sh.in"
  "${CMAKE_CURRENT_BINARY_DIR}/env.sh"
  IMMEDIATE @ONLY)



# create target uninstall to deinstall tiscamera
# if tiscamera is a subproject/module
# create uninstall-tiscamera instead
if (TARGET uninstall)
  add_custom_target(uninstall-tiscamera
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
else (TARGET uninstall)
  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif (TARGET uninstall)

include(CPackComponent)

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "The Imaging Source Europe GmbH <support@theimagingsource.com>")

string(CONCAT PACKAGE_DESCRIPTION "IC4 GStreamer Source\n"
  "  Allows accessing ic4 devices via gstreamer.")

set(CPACK_PACKAGE_DESCRIPTION "The Imaging Source Linux Software")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IC4 GStreamer Source")
set(CPACK_PACKAGE_VENDOR "The Imaging Source Europe GmbH")
set(CPACK_PACKAGE_CONTACT "support@theimagingsource.com")
set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)

set(CPACK_PACKAGE_CHECKSUM "SHA512")
set(CPACK_SET_DESTDIR "ON")
set(CPACK_GENERATOR "DEB")

set(CPACK_PACKAGE_VERSION_MAJOR "${IC4SRC_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${IC4SRC_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${IC4SRC_VERSION_PATCH}")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgstreamer1.0-0 (>= 1.8.0), tiscamera-tcamproperty (>= 1.1.0), ic4 (>= 1.4.0), libc6 (>=2.27)")

git_commit_count(GIT_COMMIT_COUNT)

set(CPACK_DEBIAN_PACKAGE_VERSION "${IC4SRC_VERSION}.${GIT_COMMIT_COUNT}")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

set(CPACK_DEBIAN_BIN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_DEBUGINFO_PACKAGE "ON")

include(package-name)

create_package_name(CPACK_DEBIAN_FILE_NAME "ic4-gstreamer" "${IC4SRC_VERSION}")
set(CPACK_DEBIAN_FILE_NAME "${CPACK_DEBIAN_FILE_NAME}.deb")

# always last
include(CPack)
