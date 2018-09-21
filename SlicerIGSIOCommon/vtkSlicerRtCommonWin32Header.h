///  vtkSlicerIGSIOCommonWin32Header - manage Windows system differences
/// 
/// The vtkSlicerIGSIOCommonWin32Header captures some system differences between Unix
/// and Windows operating systems. 


#ifndef __vtkSlicerIGSIOCommonWin32Header_h
#define __vtkSlicerIGSIOCommonWin32Header_h

#include <vtkSlicerIGSIOCommonConfigure.h>

#if defined(WIN32) && !defined(vtkSlicerIGSIOCommon_STATIC)
#if defined(vtkSlicerIGSIOCommon_EXPORTS)
#define VTK_SLICERRTCOMMON_EXPORT __declspec( dllexport ) 
#else
#define VTK_SLICERRTCOMMON_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_SLICERRTCOMMON_EXPORT
#endif

#endif