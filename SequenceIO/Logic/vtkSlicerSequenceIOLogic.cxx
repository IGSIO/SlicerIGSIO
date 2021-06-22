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

// SequenceIO includes
#include "vtkSlicerSequenceIOLogic.h"

// Sequences includes
#include <vtkMRMLSequenceNode.h>

// vtkSequenceIOMRML includes
#include "vtkMRMLStreamingVolumeSequenceStorageNode.h"

// VTK includes
#include <vtkMatrix4x4.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSequenceIOLogic);

//---------------------------------------------------------------------------
class vtkSlicerSequenceIOLogic::vtkInternal
{
public:
  //---------------------------------------------------------------------------
  vtkInternal(vtkSlicerSequenceIOLogic* external);
  ~vtkInternal();

  vtkSlicerSequenceIOLogic* External;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkSlicerSequenceIOLogic::vtkInternal::vtkInternal(vtkSlicerSequenceIOLogic* external)
  : External(external)
{
}

//---------------------------------------------------------------------------
vtkSlicerSequenceIOLogic::vtkInternal::~vtkInternal()
{
}

//----------------------------------------------------------------------------
// vtkSlicerSequenceIOLogic methods

//---------------------------------------------------------------------------
vtkSlicerSequenceIOLogic::vtkSlicerSequenceIOLogic()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkSlicerSequenceIOLogic::~vtkSlicerSequenceIOLogic()
{
  if (this->Internal)
  {
    delete this->Internal;
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerSequenceIOLogic::RegisterNodes()
{
  if (this->GetMRMLScene() == NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return;
  }

  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer<vtkMRMLStreamingVolumeSequenceStorageNode>::New());

  vtkSmartPointer<vtkMRMLStreamingVolumeNode> streamingVolumeNode = vtkSmartPointer<vtkMRMLStreamingVolumeNode>::Take(
    vtkMRMLStreamingVolumeNode::SafeDownCast(this->GetMRMLScene()->CreateNodeByClass("vtkMRMLStreamingVolumeNode")));
  streamingVolumeNode->SetDefaultSequenceStorageNodeClassName("vtkMRMLStreamingVolumeSequenceStorageNode");
  this->GetMRMLScene()->AddDefaultNode(streamingVolumeNode);
}

//---------------------------------------------------------------------------
void vtkSlicerSequenceIOLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkSlicerSequenceIOLogic:             " << this->GetClassName() << "\n";
}
