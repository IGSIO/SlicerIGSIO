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

// Sequences includes
#include <vtkMRMLNodeSequencer.h>
#include <vtkMRMLSequenceNode.h>

// vtkVideoUtilMRML includes
#include "vtkMRMLStreamingVolumeSequenceStorageNode.h"

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
    this->DefaultSequenceStorageNodeClassName = "vtkMRMLStreamingVolumeSequenceStorageNode";
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

  virtual void AddDefaultDisplayNodes(vtkMRMLNode* node)
  {
    if (node == NULL)
    {
      return;
    }

    vtkMRMLStreamingVolumeNode* streamingNode = vtkMRMLStreamingVolumeNode::SafeDownCast(node);
    if (streamingNode == NULL)
    {
      // not a streaming volume node, there is nothing to do
      return;
    }

    if (streamingNode->GetDisplayNode())
    {
      return;
    }

    int numberOfComponents = 0;
    vtkStreamingVolumeFrame* frame = streamingNode->GetFrame();
    if (frame)
    {
      numberOfComponents = frame->GetNumberOfComponents();
    }
    else
    {
      vtkImageData* image = streamingNode->GetImageData();
      numberOfComponents = image->GetNumberOfScalarComponents();
    }

    if (numberOfComponents == 1)
    {
      bool scalarDisplayNodeRequired = (numberOfComponents == 1);
      vtkSmartPointer<vtkMRMLVolumeDisplayNode> displayNode;
      displayNode = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
      if (node->GetScene())
      {
        node->GetScene()->AddNode(displayNode);
      }

      const char* colorTableId = vtkMRMLColorLogic::GetColorTableNodeID(vtkMRMLColorTableNode::Grey);
      displayNode->SetAndObserveColorNodeID(colorTableId);
      streamingNode->SetAndObserveDisplayNodeID(displayNode->GetID());
    }
    else
    {
      streamingNode->CreateDefaultDisplayNodes();
    }
  }
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
}

//-----------------------------------------------------------------------------
void vtkSlicerVideoUtilLogic::RegisterNodes()
{
  if (this->GetMRMLScene() == NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return;
  }

  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer<vtkMRMLStreamingVolumeSequenceStorageNode>::New());
  vtkMRMLNodeSequencer::GetInstance()->RegisterNodeSequencer(new StreamingVolumeNodeSequencer());
}

//---------------------------------------------------------------------------
void vtkSlicerVideoUtilLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkSlicerVideoUtilLogic:             " << this->GetClassName() << "\n";
}
