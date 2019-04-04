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
set(EXTENSION_DEPENDS "Sequences")
set(EXTENSION_BUILD_SUBDIRECTORY inner-build)
set(SUPERBUILD_TOPLEVEL_PROJECT inner)

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer COMPONENTS ConfigurePrerequisites REQUIRED)
project(${EXTENSION_NAME})
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})
mark_as_superbuild(Slicer_DIR)
mark_as_superbuild(VP9_DIR)
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
add_subdirectory(SequenceIO)
add_subdirectory(VideoUtil)

#-----------------------------------------------------------------------------
set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
