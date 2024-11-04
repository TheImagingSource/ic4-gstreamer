

#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 3.16)

#Name your project here
project(tcamic4src LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# include(version.cmake)
set(IC4SRC_VERSION_MAJOR 0)
set(IC4SRC_VERSION_MINOR 1)
set(IC4SRC_VERSION_PATCH 0)
set(IC4SRC_VERSION_MODIFIER "")

set(IC4SRC_VERSION "${IC4SRC_VERSION_MAJOR}.${IC4SRC_VERSION_MINOR}.${IC4SRC_VERSION_PATCH}${IC4SRC_VERSION_MODIFIER}" CACHE STRING "Version number")


set(IC4SRC_ORIGIN "https://theimagingsource.com")

include( StandardProjectSettings )

include("cmake/CompilerWarnings.cmake")
include("cmake/git-helper.cmake")

set(default_build_type "Release")   # Override the default build type to be 'Release' and not 'RelWithDebInfo'
include("cmake/StandardProjectSettings.cmake")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

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

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_POSITION_INDEPENDENT_CODE ON )


set(IC4SRC_INSTALL_DOC "/usr/share/theimagingsource/tcamic4src/doc/")


find_package(ic4 REQUIRED)

set(ic4_INCLUDE_PATH "${IC4_INCLUDE_PATH}")
set(ic4_LIBRARY_PATH "")
set(ic4_LIBRARIES "${IC4_LIBRARIES}")


add_subdirectory(libs)
add_subdirectory(src)
add_subdirectory(doc)

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

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgstreamer1.0-0 (>= 1.8.0), tiscamera-tcamproperty (>= 1.1.0), ic4 (>= 1.1.0), libc6 (>=2.27)")

git_commit_count(GIT_COMMIT_COUNT)

set(CPACK_DEBIAN_PACKAGE_VERSION "${IC4SRC_VERSION}.${GIT_COMMIT_COUNT}")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

set(CPACK_DEBIAN_BIN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_DEBUGINFO_PACKAGE "ON")

include(package-name)

create_package_name(CPACK_DEBIAN_FILE_NAME "tcamic4src" "${IC4SRC_VERSION}")
set(CPACK_DEBIAN_FILE_NAME "${CPACK_DEBIAN_FILE_NAME}.deb")

# always last
include(CPack)
