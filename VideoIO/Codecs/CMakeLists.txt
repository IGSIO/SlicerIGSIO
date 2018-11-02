project(vtkSlicer${MODULE_NAME}Codecs)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_CODECS_EXPORT")


set(${KIT}_INCLUDE_DIRECTORIES
  ${VP9_INCLUDE_DIR}
  )

set(${KIT}_SRCS
  vtkVP9VolumeCodec.cxx
  vtkVP9VolumeCodec.h
  )

set(${KIT}_TARGET_LIBRARIES
  vtkAddon
  ${VP9_LIBRARY}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )