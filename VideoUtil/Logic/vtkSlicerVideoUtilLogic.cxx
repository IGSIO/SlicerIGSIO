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

// Slicer includes
#include <vtkObjectFactory.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLColorLogic.h>

// MRML includes
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLStreamingVolumeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLColorTableNode.h>

// Volumes includes
#include <vtkStreamingVolumeCodecFactory.h>

// VideoUtil includes
#include "vtkSlicerVideoUtilLogic.h"

// VTK includes
#include <vtkMatrix4x4.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVideoUtilLogic);

//---------------------------------------------------------------------------
class vtkSlicerVideoUtilLogic::vtkInternal
{
public:
  //---------------------------------------------------------------------------
  vtkInternal(vtkSlicerVideoUtilLogic* external);
  ~vtkInternal();

  vtkSlicerVideoUtilLogic* External;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkSlicerVideoUtilLogic::vtkInternal::vtkInternal(vtkSlicerVideoUtilLogic* external)
  : External(external)
{
}

//---------------------------------------------------------------------------
vtkSlicerVideoUtilLogic::vtkInternal::~vtkInternal()
{
}

//----------------------------------------------------------------------------
// vtkSlicerVideoUtilLogic methods

//---------------------------------------------------------------------------
vtkSlicerVideoUtilLogic::vtkSlicerVideoUtilLogic()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkSlicerVideoUtilLogic::~vtkSlicerVideoUtilLogic()
{
  if (this->Internal)
  {
    delete this->Internal;
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerVideoUtilLogic::RegisterNodes()
{
  if (this->GetMRMLScene() == NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerVideoUtilLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkSlicerVideoUtilLogic:             " << this->GetClassName() << "\n";
}
