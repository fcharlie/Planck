# Planck cmake
cmake_minimum_required(VERSION 3.12)

project(Planck)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
CMake would overwrite the makefiles distributed with Planck.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(PLANCK_VERSION_MAJOR 1)
set(PLANCK_VERSION_MINOR 0)
set(PLANCK_VERSION_PATCH 1)
set(PACKAGE_VERSION "${PLANCK_VERSION_MAJOR}.${PLANCK_VERSION_MINOR}.${PLANCK_VERSION_PATCH}")

set(CPACK_PACKAGE_NAME "Planck")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Planck")
set(CPACK_PACKAGE_VERSION_MAJOR ${PLANCK_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PLANCK_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PLANCK_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_PACKAGE_VENDOR "Planck")
set(CPACK_PACKAGE_CONTACT "Force Charlie")

include(CPack)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
  message(STATUS "IPO/LTO enabled")
else()
  message(STATUS "IPO/LTO not supported: <${error}>")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
# Your should add git under your path,
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

# Generate version code
configure_file(
  ${CMAKE_SOURCE_DIR}/include/baseversion.h.cmake
  ${CMAKE_BINARY_DIR}/include/baseversion.h
)

# include(ExternalProject)
# set(PLANCK_CLEANROOT "${CMAKE_BINARY_DIR}/cleanroot")
# ExternalProject_Add(zlib
#   URL https://github.com/madler/zlib/archive/v1.2.11.tar.gz
#   URL_HASH SHA256=629380c90a77b964d896ed37163f5c3a34f6e6d897311f1df2a7016355c45eff
#   CMAKE_ARGS
#     -DCMAKE_INSTALL_PREFIX:PATH=${PLANCK_CLEANROOT}
#     -DCMAKE_BUILD_TYPE:STRING=Release
#   BUILD_COMMAND ninja all
#   INSTALL_COMMAND ninja install
# )

add_definitions(
  -D_UNICODE=1
  -DUNICODE=1
)

include_directories(
  ${CMAKE_BINARY_DIR}/include
  ./vendor/bela/include
  ./include
  ./lib/inquisitive
)

add_subdirectory(vendor/bela)
add_subdirectory(lib/inquisitive)
add_subdirectory(tools)
add_subdirectory(utils)