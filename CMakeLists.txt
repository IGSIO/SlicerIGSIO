cmake_minimum_required(VERSION 3.5)
set(EXTENSION_NAME SlicerIGSIO)
project(${EXTENSION_NAME})

#-----------------------------------------------------------------------------
# Extension meta-information
#set(EXTENSION_HOMEPAGE "http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/SlicerIGSIO")
set(EXTENSION_CATEGORY "IGT")
set(EXTENSION_CONTRIBUTORS "Kyle Sunderland (PerkLab, Queen's University)")
set(EXTENSION_DESCRIPTION "This is an extension that provides access to the tools implemented in IGSIO")
set(EXTENSION_ICONURL "https://github.com/IGSIO/SlicerIGSIO/raw/master/SlicerIGSIO.png")
#set(EXTENSION_SCREENSHOTURLS "")
set(EXTENSION_DEPENDS "Sequences")
set(EXTENSION_BUILD_SUBDIRECTORY inner-build)

set(SUPERBUILD_TOPLEVEL_PROJECT inner)

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})
mark_as_superbuild(Slicer_DIR)

mark_as_superbuild(IGSIO_DIR)

find_package(Sequences REQUIRED)
mark_as_superbuild(Sequences_DIR)

find_package(Git REQUIRED)
mark_as_superbuild(GIT_EXECUTABLE)

#-----------------------------------------------------------------------------
# SuperBuild setup
option(${EXTENSION_NAME}_SUPERBUILD "Build ${EXTENSION_NAME} and the projects it depends on." ON)
mark_as_advanced(${EXTENSION_NAME}_SUPERBUILD)
if(${EXTENSION_NAME}_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(SlicerIGSIOCommon)
add_subdirectory(VideoIO)

#-----------------------------------------------------------------------------
set(EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS)
list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${IGSIO_DIR};vtkVideoIO;RuntimeLibraries;/")
set(${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS "${EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS}" CACHE STRING "List of external projects to install" FORCE)
message(${EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS})
#-----------------------------------------------------------------------------
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS}")
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
