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
#ifndef __vtkSlicerVideoUtilLogic_h
#define __vtkSlicerVideoUtilLogic_h

#include "vtkSlicerVideoUtilModuleLogicExport.h"

// Slicer includes
#include <vtkSlicerBaseLogic.h>
#include <vtkSlicerModuleLogic.h>

// VTK includes
#include <vtkCallbackCommand.h>

// Sequences MRML includes
#include <vtkMRMLSequenceBrowserNode.h>

class vtkMRMLIGTLConnectorNode;

/// \ingroup Slicer_QtModules_VideoUtil
class VTK_SLICER_VIDEOUTIL_MODULE_LOGIC_EXPORT vtkSlicerVideoUtilLogic : public vtkSlicerModuleLogic
{
 public:
  static vtkSlicerVideoUtilLogic *New();
  vtkTypeMacro(vtkSlicerVideoUtilLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream&, vtkIndent) override;

  void RegisterNodes() override;

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

  vtkSlicerVideoUtilLogic();
  virtual ~vtkSlicerVideoUtilLogic();

 private:

private:
  class vtkInternal;
  vtkInternal * Internal;

  vtkSlicerVideoUtilLogic(const vtkSlicerVideoUtilLogic&); // Not implemented
  void operator=(const vtkSlicerVideoUtilLogic&);               // Not implemented
};

#endif
