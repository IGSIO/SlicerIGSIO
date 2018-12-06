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

/// Manages the logic associated with encoded video input/output.
#ifndef __vtkSlicerVideoIOLogic_h
#define __vtkSlicerVideoIOLogic_h

#include "vtkSlicerVideoIOModuleLogicExport.h"

// Slicer includes
#include <vtkSlicerBaseLogic.h>
#include <vtkSlicerModuleLogic.h>

// VTK includes
#include <vtkCallbackCommand.h>

// IGSIO includes
#include <vtkIGSIOTrackedFrameList.h>

// Sequences MRML includes
#include <vtkMRMLSequenceBrowserNode.h>

class vtkMRMLIGTLConnectorNode;

/// \ingroup Slicer_QtModules_VideoIO
class VTK_SLICER_VIDEOIO_MODULE_LOGIC_EXPORT vtkSlicerVideoIOLogic : public vtkSlicerModuleLogic
{
 public:
  static vtkSlicerVideoIOLogic *New();
  vtkTypeMacro(vtkSlicerVideoIOLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream&, vtkIndent) VTK_OVERRIDE;

  void RegisterNodes();

  //----------------------------------------------------------------
  // Events
  //----------------------------------------------------------------

  //----------------------------------------------------------------
  // Connector and device Management
  //----------------------------------------------------------------

  //----------------------------------------------------------------
  // MRML Management
  //----------------------------------------------------------------

 protected:

  //----------------------------------------------------------------
  // Constructor, destructor etc.
  //----------------------------------------------------------------

  vtkSlicerVideoIOLogic();
  virtual ~vtkSlicerVideoIOLogic();

 private:
  
private:
  class vtkInternal;
  vtkInternal * Internal;

  vtkSlicerVideoIOLogic(const vtkSlicerVideoIOLogic&); // Not implemented
  void operator=(const vtkSlicerVideoIOLogic&);               // Not implemented
};

#endif
