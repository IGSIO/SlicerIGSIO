cmake_minimum_required(VERSION 2.8)

find_package(IGSIO REQUIRED)

set(MODULE_NAME "VideoIO")
set(MODULE_TITLE "VideoIO")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${VTKSEQUENCEIO_INCLUDES}
  ${VTKIGSIOCOMMON_INCLUDES}
  ${vtkSlicerSequenceBrowserModuleMRML_INCLUDES}
  ${SlicerIGSIOCommon_INCLUDE_DIRS}
  )

set(MODULE_SRCS
  qSlicer${MODULE_NAME}Module.cxx
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.cxx
  qSlicer${MODULE_NAME}ModuleWidget.h
  qSlicerVideoReader.cxx
  qSlicerVideoReader.h
  )

set(MODULE_MOC_SRCS
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.h
  qSlicerVideoReader.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerVideoIOModule.ui
)

set(MODULE_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  ${MRML_LIBRARIES}
  vtkIGSIOCommon
  vtkSequenceIO
  vtkSlicerIGSIOCommon
  vtkSlicerSequencesModuleMRML
  vtkSlicerSequenceBrowserModuleMRML
  )

set(MODULE_RESOURCES
  Resources/qSlicer${MODULE_NAME}Module.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildLoadableModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  )

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
