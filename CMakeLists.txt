# FileView cmake
cmake_minimum_required(VERSION 3.12)

project(FileViewer)

if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
CMake would overwrite the makefiles distributed with FileView.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if(NOT DEFINED FILEVIEW_VERSION_MAJOR)
  set(FILEVIEW_VERSION_MAJOR 1)
endif()
if(NOT DEFINED FILEVIEW_VERSION_MINOR)
  set(FILEVIEW_VERSION_MINOR 0)
endif()
if(NOT DEFINED FILEVIEW_VERSION_PATCH)
  set(FILEVIEW_VERSION_PATCH 1)
endif()

if (NOT PACKAGE_VERSION)
  set(PACKAGE_VERSION
    "${FILEVIEW_VERSION_MAJOR}.${FILEVIEW_VERSION_MINOR}.${FILEVIEW_VERSION_PATCH}")
endif()

set(CPACK_PACKAGE_NAME "FileView")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "FileView")
set(CPACK_PACKAGE_VERSION_MAJOR ${FILEVIEW_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${FILEVIEW_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${FILEVIEW_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_PACKAGE_VENDOR "FileView")
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

add_definitions(
  -D_UNICODE=1
  -DUNICODE=1
)

include_directories(
  ./include
)

add_subdirectory(lib/inquisitive)
