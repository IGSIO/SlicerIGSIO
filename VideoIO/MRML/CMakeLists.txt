project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSequencesModuleMRML_INCLUDE_DIRS}
  ${VTKSEQUENCEIO_INCLUDE_DIRS}
  ${SlicerIGSIOCommon_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkMRMLStreamingVolumeSequenceStorageNode.cxx
  vtkMRMLStreamingVolumeSequenceStorageNode.h
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicerSequencesModuleMRML
  vtkSlicerVolumesModuleLogic
  vtkSlicerIGSIOCommon
  ${VTKSEQUENCEIO_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
