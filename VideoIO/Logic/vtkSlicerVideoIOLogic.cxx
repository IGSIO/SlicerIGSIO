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

// MRML includes
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLStreamingVolumeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLLinearTransformNode.h>

// Volumes includes
#include <vtkStreamingVolumeCodecFactory.h>

// VideoIO includes
#include "vtkSlicerVideoIOLogic.h"

// Sequences includes
#include <vtkMRMLNodeSequencer.h>
#include <vtkMRMLSequenceNode.h>

// vtkVideoIOMRML includes
#include "vtkMRMLStreamingVolumeSequenceStorageNode.h"

// VTK includes
#include <vtkMatrix4x4.h>

// vtkVideoIO includes
#include <vtkMKVUtil.h>
#include <vtkMKVReader.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVideoIOLogic);

//---------------------------------------------------------------------------
class vtkSlicerVideoIOLogic::vtkInternal
{
public:
  //---------------------------------------------------------------------------
  vtkInternal(vtkSlicerVideoIOLogic* external);
  ~vtkInternal();

  vtkSlicerVideoIOLogic* External;
};

//----------------------------------------------------------------------------
class StreamingVolumeNodeSequencer : public vtkMRMLNodeSequencer::NodeSequencer
{
public:
  StreamingVolumeNodeSequencer()
  {
    this->SupportedNodeClassName = "vtkMRMLStreamingVolumeNode";
    this->RecordingEvents->InsertNextValue(vtkMRMLVolumeNode::ImageDataModifiedEvent);
    this->SupportedNodeParentClassNames.push_back("vtkMRMLVectorVolumeNode");
    this->SupportedNodeParentClassNames.push_back("vtkMRMLScalarVolumeNode");
    this->SupportedNodeParentClassNames.push_back("vtkMRMLDisplayableNode");
    this->SupportedNodeParentClassNames.push_back("vtkMRMLVolumeNode");
    this->SupportedNodeParentClassNames.push_back("vtkMRMLTransformableNode");
    this->SupportedNodeParentClassNames.push_back("vtkMRMLStorableNode");
    this->SupportedNodeParentClassNames.push_back("vtkMRMLNode");
  }

  virtual void CopyNode(vtkMRMLNode* source, vtkMRMLNode* target, bool shallowCopy /* =false */)
  {
    int oldModified = target->StartModify();
    vtkSmartPointer<vtkMRMLStreamingVolumeNode> targetStreamNode = vtkMRMLStreamingVolumeNode::SafeDownCast(target);
    vtkSmartPointer<vtkMRMLStreamingVolumeNode> sourceStreamNode = vtkMRMLStreamingVolumeNode::SafeDownCast(source);
    if (targetStreamNode && sourceStreamNode)
    {
      vtkStreamingVolumeFrame* frame = sourceStreamNode->GetFrame();
      if (frame)
      {
        targetStreamNode->SetAndObserveFrame(sourceStreamNode->GetFrame());
      }
      else
      {
        if (shallowCopy)
        {
          targetStreamNode->SetAndObserveImageData(sourceStreamNode->GetImageData());
        }
        else
        {
          vtkSmartPointer<vtkImageData> newImage = vtkSmartPointer<vtkImageData>::New();
          newImage->DeepCopy(sourceStreamNode->GetImageData());
          targetStreamNode->SetAndObserveImageData(newImage);
        }
      }

      vtkSmartPointer<vtkMatrix4x4> ijkToRASMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      sourceStreamNode->GetIJKToRASMatrix(ijkToRASMatrix);
      targetStreamNode->SetIJKToRASMatrix(ijkToRASMatrix);
    }

    target->EndModify(oldModified);
  }

  virtual void AddDefaultSequenceStorageNode(vtkMRMLSequenceNode* node)
  {
    if (node == NULL)
    {
      // not a sequence node, there is nothing to do
      return;
    }

    vtkMRMLScene* scene = node->GetScene();
    if (scene == NULL)
    {
      return;
    }

    if (node->GetStorageNode())
    {
      // Storage node already exists
      return;
    }

    vtkMRMLNode* storageNode = scene->AddNewNodeByClass("vtkMRMLStreamingVolumeSequenceStorageNode");
    if (storageNode == NULL)
    {
      return;
    }
    node->SetAndObserveStorageNodeID(storageNode->GetID());
  }
};


//----------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkSlicerVideoIOLogic::vtkInternal::vtkInternal(vtkSlicerVideoIOLogic* external)
  : External(external)
{
}

//---------------------------------------------------------------------------
vtkSlicerVideoIOLogic::vtkInternal::~vtkInternal()
{
}

//----------------------------------------------------------------------------
// vtkSlicerVideoIOLogic methods

//---------------------------------------------------------------------------
vtkSlicerVideoIOLogic::vtkSlicerVideoIOLogic()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkSlicerVideoIOLogic::~vtkSlicerVideoIOLogic()
{
}

//-----------------------------------------------------------------------------
void vtkSlicerVideoIOLogic::RegisterNodes()
{
  if (this->GetMRMLScene() == NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return;
  }

  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer<vtkMRMLStreamingVolumeSequenceStorageNode>::New());

  vtkMRMLNodeSequencer* sequencer = vtkMRMLNodeSequencer::GetInstance();
  sequencer->RegisterNodeSequencer(new StreamingVolumeNodeSequencer());
}

//---------------------------------------------------------------------------
void vtkSlicerVideoIOLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkSlicerVideoIOLogic:             " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
bool vtkSlicerVideoIOLogic::WriteVideo(std::string fileName, vtkTrackedFrameList* trackedFrameList)
{

  return true;
}
