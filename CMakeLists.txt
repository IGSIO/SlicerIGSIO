cmake_minimum_required(VERSION 3.5)

#-----------------------------------------------------------------------------
set(EXTENSION_NAME SlicerIGSIO)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "https://github.com/IGSIO/SlicerIGSIO")
set(EXTENSION_CATEGORY "IGT")
set(EXTENSION_CONTRIBUTORS "Kyle Sunderland (PerkLab, Queen's University), Andras Lasso (PerkLab, Queen's University)")
set(EXTENSION_DESCRIPTION "This is a utility extension that is developed as part of the Image Guided Surgery InterOperability (IGSIO) organization (see http://igsio.github.io/), and should be used as a dependency for other extensions (Ex. SlicerIGT) to provide access to tools and algorithms implemented by IGSIO (https://github.com/IGSIO/IGSIO).")
set(EXTENSION_ICONURL "https://github.com/IGSIO/SlicerIGSIO/raw/master/SlicerIGSIO.png")
set(EXTENSION_SCREENSHOTURLS "https://github.com/IGSIO/SlicerIGSIO/raw/master/Images/SlicerIGSIOScreenshot1.png")
set(EXTENSION_BUILD_SUBDIRECTORY inner-build)
set(SUPERBUILD_TOPLEVEL_PROJECT inner)

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer COMPONENTS ConfigurePrerequisites REQUIRED)
project(${EXTENSION_NAME})
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})
mark_as_superbuild(Slicer_DIR)

#-----------------------------------------------------------------------------
# Options

set (SlicerIGSIO_USE_VP9_DEFAULT ON)
if (APPLE)
  set (SlicerIGSIO_USE_VP9_DEFAULT OFF)
endif()

option(SlicerIGSIO_USE_VP9 "Build with VP9 support" ${SlicerIGSIO_USE_VP9_DEFAULT})
mark_as_superbuild(SlicerIGSIO_USE_VP9:BOOL)

set(SlicerIGSIO_USE_GPU_DEFAULT OFF)
find_package(OpenCL QUIET)
if(OpenCL_FOUND AND NOT APPLE)
  set(SlicerIGSIO_USE_GPU_DEFAULT ON)
endif()

option(SlicerIGSIO_USE_GPU "Build with GPU support" ${SlicerIGSIO_USE_GPU_DEFAULT})

#-----------------------------------------------------------------------------
# SuperBuild setup
option(${EXTENSION_NAME}_SUPERBUILD "Build ${EXTENSION_NAME} and the projects it depends on." ON)
mark_as_advanced(${EXTENSION_NAME}_SUPERBUILD)
if(${EXTENSION_NAME}_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

#-----------------------------------------------------------------------------
# Extension dependencies

find_package(Git REQUIRED)
mark_as_superbuild(GIT_EXECUTABLE)

mark_as_superbuild(VP9_DIR)
mark_as_superbuild(IGSIO_DIR)

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(SlicerIGSIOCommon)
add_subdirectory(SequenceIO)
add_subdirectory(VideoUtil)
add_subdirectory(MetafileImporter)

#-----------------------------------------------------------------------------
# Add IGSIO to SlicerIGSIOConfig
set(SlicerIGSIO_CUSTOM_CONFIG "include(\"${IGSIO_DIR}/IGSIOConfig.cmake\")")
include(${Slicer_EXTENSION_GENERATE_CONFIG})

#-----------------------------------------------------------------------------
set(EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS)
list(APPEND EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS "${IGSIO_DIR};IGSIO;RuntimeLibraries;/")
set(${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS "${EXTENSION_CPACK_INSTALL_CMAKE_PROJECTS}" CACHE STRING "List of external projects to install" FORCE)
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
list(APPEND CPACK_INSTALL_CMAKE_PROJECTS "${${EXTENSION_NAME}_CPACK_INSTALL_CMAKE_PROJECTS}")
include(${Slicer_EXTENSION_CPACK})
