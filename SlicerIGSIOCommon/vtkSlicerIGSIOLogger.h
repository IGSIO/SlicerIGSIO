/*==============================================================================

Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
Queen's University, Kingston, ON, Canada. All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
and was supported through CANARIE's Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

#ifndef __vtkSlicerIGSIOLogger_h
#define __vtkSlicerIGSIOLogger_h

// IGSIO includes
#include <vtkIGSIOLogger.h>

// vtkSlicerIGSIOCommon includes
#include "vtkSlicerIGSIOCommon.h"

/*!
\class vtkSlicerIGSIOLogger
\brief Class to abstract away specific sequence file read/write details
\ingroup PlusLibCommon
*/
class VTK_SLICERIGSIOCOMMON_EXPORT vtkSlicerIGSIOLogger : public vtkIGSIOLogger
{
public:
  static vtkIGSIOLogger* Instance();

  void LogMessage(LogLevelType level, const char* msg, const char* fileName, int lineNumber, const char* optionalPrefix = NULL) override;
  void LogMessage(LogLevelType level, const wchar_t* msg, const char* fileName, int lineNumber, const wchar_t* optionalPrefix = NULL) override;

private:
  vtkSlicerIGSIOLogger();
  ~vtkSlicerIGSIOLogger();

  vtkSlicerIGSIOLogger(const vtkSlicerIGSIOLogger&); // Not implemented
  void operator=(const vtkSlicerIGSIOLogger&);       // Not implemented
};

#endif // __vtkSlicerIGSIOLogger_h
