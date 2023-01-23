

#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 3.16)

#Name your project here
project(tcamic4src)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# include(version.cmake)

set(IC4SRC_VERSION "${TCAM_VERSION_MAJOR}.${TCAM_VERSION_MINOR}.${TCAM_VERSION_PATCH}${TCAM_VERSION_MODIFIER}" CACHE STRING "Version number")


set(IC4SRC_ORIGIN "https://theimagingsource.com")

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
  message(STATUS "IC4SRC_VERSION: ${TCAM_VERSION}, Git: commit#=${GIT_COMMIT_COUNT}, branch=${GIT_BRANCH} hash=${GIT_COMMIT_HASH}" )
else()
  message(STATUS "IC4SRC_VERSION: ${TCAM_VERSION}, Git: No repo found" )
endif()

# All option for configuration come from here
# include(CMakeOptions.cmake)

add_compile_options( -Wno-psabi )   # disable ABI change message of g++8
add_compile_options( -Wall -Wextra -pedantic -pthread )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options( -fsized-deallocation )
endif()

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_POSITION_INDEPENDENT_CODE ON )

if (NOT DEFINED ic4_PATH)
  message (FATAL_ERROR "Define ic4_PATH. -Dic4_PATH=<..>/ic4/build/")
endif (NOT DEFINED ic4_PATH)

set(ic4_INCLUDE_PATH "${ic4_PATH}/include")
set(ic4_LIBRARY_PATH "${ic4_PATH}/lib")
set(ic4_LIBRARIES "${ic4_PATH}/bin/libic4core.so")

add_subdirectory(libs)
add_subdirectory(src)

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
