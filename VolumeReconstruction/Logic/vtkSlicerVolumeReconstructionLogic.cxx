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

// MRML includes
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVolumeNode.h>

// Sequence MRML includes
#include <vtkMRMLSequenceNode.h>

// IGSIO Common includes
#include <vtkIGSIOTrackedFrameList.h>
#include <vtkIGSIOTransformRepository.h>
#include <igsioTrackedFrame.h>

// IGSIO VolumeReconstructor includes
#include <vtkIGSIOVolumeReconstructor.h>

// SlicerIGSIOCommon includes
#include "vtkSlicerIGSIOCommon.h"

// vtkAddon includes
#include <vtkStreamingVolumeCodecFactory.h>

// VolumeReconstructor MRML includes
#include "vtkMRMLVolumeReconstructionNode.h"

// VolumeReconstruction Logic includes
#include "vtkSlicerVolumeReconstructionLogic.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkTimerLog.h>
#include <vtkTransform.h>
#include <vtkSmartPointer.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVolumeReconstructionLogic);

struct ReconstructionInfo
{
  vtkSmartPointer<vtkIGSIOVolumeReconstructor> Reconstructor;
  double LastUpdateTimeSeconds;
};

typedef std::map<vtkMRMLVolumeReconstructionNode*, ReconstructionInfo> VolumeReconstuctorMap;

//---------------------------------------------------------------------------
class vtkSlicerVolumeReconstructionLogic::vtkInternal
{
public:
  //---------------------------------------------------------------------------
  vtkInternal(vtkSlicerVolumeReconstructionLogic* external);
  ~vtkInternal();

  vtkSlicerVolumeReconstructionLogic* External;

  VolumeReconstuctorMap Reconstructors;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkSlicerVolumeReconstructionLogic::vtkInternal::vtkInternal(vtkSlicerVolumeReconstructionLogic* external)
  : External(external)
{
}

//---------------------------------------------------------------------------
vtkSlicerVolumeReconstructionLogic::vtkInternal::~vtkInternal()
{
}

//----------------------------------------------------------------------------
// vtkSlicerVolumeReconstructionLogic methods

//---------------------------------------------------------------------------
vtkSlicerVolumeReconstructionLogic::vtkSlicerVolumeReconstructionLogic()
  : NumberOfVolumeNodesForReconstructionInInput(0)
  , VolumeNodesAddedToReconstruction(0)
  , Internal(new vtkInternal(this))
{
}

//---------------------------------------------------------------------------
vtkSlicerVolumeReconstructionLogic::~vtkSlicerVolumeReconstructionLogic()
{
  if (this->Internal)
  {
    delete this->Internal;
  }
}

//-----------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::RegisterNodes()
{
  if (this->GetMRMLScene() == NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer<vtkMRMLVolumeReconstructionNode>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> sceneEvents;
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  sceneEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, sceneEvents.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkMRMLVolumeReconstructionNode* volumeReconstructionNode = vtkMRMLVolumeReconstructionNode::SafeDownCast(node);
  if (!node || !this->GetMRMLScene())
  {
    return;
  }

  ReconstructionInfo info;
  info.Reconstructor = vtkSmartPointer<vtkIGSIOVolumeReconstructor>::New();
  info.LastUpdateTimeSeconds = vtkTimerLog::GetUniversalTime();

  this->Internal->Reconstructors[volumeReconstructionNode] = info;
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkMRMLVolumeReconstructionNode* volumeReconstructionNode = vtkMRMLVolumeReconstructionNode::SafeDownCast(node);
  if (!volumeReconstructionNode || !this->GetMRMLScene())
  {
    return;
  }

  VolumeReconstuctorMap::iterator volumeReconstructorIt = this->Internal->Reconstructors.find(volumeReconstructionNode);
  if (volumeReconstructorIt != this->Internal->Reconstructors.end())
  {
    this->Internal->Reconstructors.erase(volumeReconstructorIt);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  vtkMRMLVolumeReconstructionNode* volumeReconstructionNode = vtkMRMLVolumeReconstructionNode::SafeDownCast(caller);
  if (volumeReconstructionNode && event == vtkMRMLVolumeReconstructionNode::InputVolumeModified && volumeReconstructionNode->GetLiveVolumeReconstructionInProgress())
  {
    this->AddVolumeNodeToReconstructedVolume(volumeReconstructionNode, volumeReconstructionNode->GetNumberOfVolumesAddedToReconstruction() == 0, false);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::UpdateLiveVolumeReconstruction()
{
  vtkNew<vtkTimerLog> timer;
  for (VolumeReconstuctorMap::iterator it = this->Internal->Reconstructors.begin(); it != this->Internal->Reconstructors.end(); ++it)
  {
    vtkMRMLVolumeReconstructionNode* volumeReconstructionNode = it->first;
    ReconstructionInfo* info = &(it->second);
    vtkIGSIOVolumeReconstructor* reconstructor = info->Reconstructor;
    if (!volumeReconstructionNode || !reconstructor || !volumeReconstructionNode->GetLiveVolumeReconstructionInProgress())
    {
      continue;
    }

    double currentTime = timer->GetUniversalTime();
    if (currentTime - info->LastUpdateTimeSeconds < volumeReconstructionNode->GetLiveUpdateIntervalSeconds())
    {
      continue;
    }

    this->GetReconstructedVolume(volumeReconstructionNode);
    info->LastUpdateTimeSeconds = currentTime;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::StartVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    vtkErrorMacro("Invalid volume reconstruction node!");
    return;
  }

  vtkMRMLVolumeNode* inputVolumeNode = volumeReconstructionNode->GetInputVolumeNode();
  if (!inputVolumeNode)
  {
    vtkErrorMacro("Invalid input volume node!");
    return;
  }

  vtkMRMLAnnotationROINode* inputROINode = volumeReconstructionNode->GetInputROINode();
  if (!inputROINode)
  {
    vtkErrorMacro("Invalid input ROI node!");
    return;
  }

  vtkIGSIOVolumeReconstructor* reconstructor = this->Internal->Reconstructors[volumeReconstructionNode].Reconstructor;
  if (!reconstructor)
  {
    vtkErrorMacro("Invalid volume reconstructor!");
    return;
  }

  double bounds[6] = { 0 };
  inputROINode->GetBounds(bounds);
  double outputSpacing[3] = { 0 };
  volumeReconstructionNode->GetOutputSpacing(outputSpacing);
  int outputExtent[6] = {
    0, static_cast<int>(std::ceil((bounds[1] - bounds[0]) / outputSpacing[0])),
    0, static_cast<int>(std::ceil((bounds[3] - bounds[2]) / outputSpacing[1])),
    0, static_cast<int>(std::ceil((bounds[5] - bounds[4]) / outputSpacing[2]))
  };
  double outputOrigin[3] = { bounds[0], bounds[2], bounds[4] };

  reconstructor->SetOutputExtent(outputExtent);
  reconstructor->SetOutputOrigin(outputOrigin);
  reconstructor->SetOutputSpacing(outputSpacing);
  reconstructor->SetCompoundingMode(vtkIGSIOPasteSliceIntoVolume::CompoundingType(volumeReconstructionNode->GetCompoundingMode()));
  reconstructor->SetOptimization(vtkIGSIOPasteSliceIntoVolume::OptimizationType(volumeReconstructionNode->GetOptimizationMode()));
  reconstructor->SetInterpolation(vtkIGSIOPasteSliceIntoVolume::InterpolationType(volumeReconstructionNode->GetInterpolationMode()));
  reconstructor->SetNumberOfThreads(volumeReconstructionNode->GetNumberOfThreads());
  reconstructor->SetFillHoles(volumeReconstructionNode->GetFillHoles());
  reconstructor->SetImageCoordinateFrame("Image");
  reconstructor->SetReferenceCoordinateFrame("World");
  reconstructor->SetClipRectangleOrigin(volumeReconstructionNode->GetClipRectangleOrigin());
  reconstructor->SetClipRectangleSize(volumeReconstructionNode->GetClipRectangleSize());
  reconstructor->Reset();

  volumeReconstructionNode->SetNumberOfVolumesAddedToReconstruction(0);
  volumeReconstructionNode->InvokeEvent(vtkMRMLVolumeReconstructionNode::VolumeReconstructionStarted);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::StartLiveVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    return;
  }
  this->StartVolumeReconstruction(volumeReconstructionNode);
  this->ResumeLiveVolumeReconstruction(volumeReconstructionNode);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::ResumeLiveVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    return;
  }
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLVolumeReconstructionNode::InputVolumeModified);
  vtkObserveMRMLNodeEventsMacro(volumeReconstructionNode, events);
  volumeReconstructionNode->LiveVolumeReconstructionInProgressOn();
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::StopLiveVolumeReconstruction(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    return;
  }
  volumeReconstructionNode->LiveVolumeReconstructionInProgressOff();
  vtkUnObserveMRMLNodeMacro(volumeReconstructionNode);
  this->GetReconstructedVolume(volumeReconstructionNode);
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeReconstructionLogic::AddVolumeNodeToReconstructedVolume(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode, bool isFirst, bool isLast)
{
  if (!volumeReconstructionNode)
  {
    vtkErrorMacro("Invalid volume reconstruction node!");
    return false;
  }

  vtkMRMLVolumeNode* inputVolumeNode = volumeReconstructionNode->GetInputVolumeNode();
  if (!inputVolumeNode)
  {
    vtkErrorMacro("Invalid input volume node!");
    return false;
  }

  vtkMRMLAnnotationROINode* inputROINode = volumeReconstructionNode->GetInputROINode();
  if (!inputROINode)
  {
    vtkErrorMacro("Invalid input ROI node!");
    return false;
  }

  vtkIGSIOVolumeReconstructor* reconstructor = this->Internal->Reconstructors[volumeReconstructionNode].Reconstructor;
  if (!reconstructor)
  {
    vtkErrorMacro("Invalid volume reconstructor!");
    return false;
  }

  vtkNew<vtkTransform> imageToWorldTransform;
  imageToWorldTransform->Identity();
  imageToWorldTransform->PreMultiply();
  vtkNew<vtkMatrix4x4> ijkToRASMatrix;
  inputVolumeNode->GetIJKToRASMatrix(ijkToRASMatrix);
  imageToWorldTransform->Concatenate(ijkToRASMatrix);

  vtkNew<vtkMatrix4x4> parentToWorldMatrix;
  vtkMRMLTransformNode* transformNode = inputVolumeNode->GetParentTransformNode();
  if (transformNode)
  {
    transformNode->GetMatrixTransformToWorld(parentToWorldMatrix);
    imageToWorldTransform->Concatenate(parentToWorldMatrix);
  }

  vtkNew<vtkIGSIOTransformRepository> transformRepository;
  transformRepository->SetTransform(igsioTransformName("ImageToWorld"), imageToWorldTransform->GetMatrix());

  std::string errorDetail;
  igsioTrackedFrame trackedFrame;
  trackedFrame.GetImageData()->DeepCopyFrom(inputVolumeNode->GetImageData());

  bool insertedIntoVolume = false;
  if (reconstructor->AddTrackedFrame(&trackedFrame, transformRepository, isFirst, isLast, &insertedIntoVolume) != IGSIO_SUCCESS)
  {
    return false;
  }

  int numberOfVolumesAddedToReconstruction = volumeReconstructionNode->GetNumberOfVolumesAddedToReconstruction();
  volumeReconstructionNode->SetNumberOfVolumesAddedToReconstruction(numberOfVolumesAddedToReconstruction + 1);
  volumeReconstructionNode->InvokeEvent(vtkMRMLVolumeReconstructionNode::VolumeAddedToReconstruction);
  return  true;
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::GetReconstructedVolume(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  vtkIGSIOVolumeReconstructor* reconstructor = this->Internal->Reconstructors[volumeReconstructionNode].Reconstructor;
  if (!reconstructor)
  {
    vtkErrorMacro("Invalid volume reconstructor!");
    return;
  }

  vtkMRMLVolumeNode* outputVolumeNode = this->GetOrAddOutputVolumeNode(volumeReconstructionNode);
  if (!outputVolumeNode)
  {
    vtkErrorMacro("Invalid output volume node!");
    return;
  }

  MRMLNodeModifyBlocker blocker(outputVolumeNode);
  if (!outputVolumeNode->GetImageData())
  {
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    outputVolumeNode->SetAndObserveImageData(imageData);
  }

  if (reconstructor->GetReconstructedVolume(outputVolumeNode->GetImageData()) != IGSIO_SUCCESS)
  {
    vtkErrorMacro("Could not retrieve reconstructed image");
  }

  double spacing[3];
  outputVolumeNode->GetImageData()->GetSpacing(spacing);
  outputVolumeNode->GetImageData()->SetSpacing(1, 1, 1);
  outputVolumeNode->SetSpacing(spacing);

  double origin[3];
  outputVolumeNode->GetImageData()->GetOrigin(origin);
  outputVolumeNode->GetImageData()->SetOrigin(0, 0, 0);
  outputVolumeNode->SetOrigin(origin);

  volumeReconstructionNode->InvokeEvent(vtkMRMLVolumeReconstructionNode::VolumeReconstructionFinished);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::ReconstructVolumeFromSequence(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    vtkErrorMacro("Invalid volume reconstruction node!");
    return;
  }

  vtkMRMLSequenceBrowserNode* inputSequenceBrowser = volumeReconstructionNode->GetInputSequenceBrowserNode();
  if (!inputSequenceBrowser)
  {
    vtkErrorMacro("Invalid input sequence browser!");
    return;
  }

  vtkMRMLVolumeNode* inputVolumeNode = volumeReconstructionNode->GetInputVolumeNode();
  if (!inputVolumeNode)
  {
    vtkErrorMacro("Invalid input volume node!");
    return;
  }

  vtkIGSIOVolumeReconstructor* reconstructor = this->Internal->Reconstructors[volumeReconstructionNode].Reconstructor;
  if (!reconstructor)
  {
    vtkErrorMacro("Invalid volume reconstructor!");
    return;
  }

  vtkMRMLSequenceNode* masterSequence = inputSequenceBrowser->GetMasterSequenceNode();
  if (!masterSequence)
  {
    vtkErrorMacro("Invalid master sequence node!");
    return;
  }

  this->GetApplicationLogic()->PauseRender();

  vtkSmartPointer<vtkMRMLAnnotationROINode> inputROINode = volumeReconstructionNode->GetInputROINode();
  if (!inputROINode)
  {
    inputROINode = vtkSmartPointer<vtkMRMLAnnotationROINode>::New();
    if (inputVolumeNode->GetName())
    {
      std::string roiNodeName = inputVolumeNode->GetName();
      roiNodeName += "_Bounds";
      inputROINode->SetName(roiNodeName.c_str());
    }
    if (this->GetMRMLScene())
    {
      this->GetMRMLScene()->AddNode(inputROINode);
    }
    volumeReconstructionNode->SetAndObserveInputROINode(inputROINode);
    this->CalculateROIFromVolumeSequence(inputSequenceBrowser, inputVolumeNode, inputROINode);
  }

  // Begin volume reconstruction
  this->StartVolumeReconstruction(volumeReconstructionNode);

  // Save the currently selected item to restore later
  int selectedItemNumber = inputSequenceBrowser->GetSelectedItemNumber();

  const int numberOfFrames = masterSequence->GetNumberOfDataNodes();
  for (int i = 0; i < numberOfFrames; ++i)
  {
    inputSequenceBrowser->SetSelectedItemNumber(i);
    this->AddVolumeNodeToReconstructedVolume(volumeReconstructionNode, i == 0, i == numberOfFrames - 1);
  }

  this->GetReconstructedVolume(volumeReconstructionNode);
  inputSequenceBrowser->SetSelectedItemNumber(selectedItemNumber);
  this->GetApplicationLogic()->ResumeRender();
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::CalculateROIFromVolumeSequence(vtkMRMLSequenceBrowserNode* inputSequenceBrowser,
  vtkMRMLVolumeNode* inputVolumeNode, vtkMRMLAnnotationROINode* outputROINodeRAS)
{
  if (!inputSequenceBrowser)
  {
    vtkErrorMacro("Invalid input sequence browser!");
    return;
  }

  if (!inputVolumeNode)
  {
    vtkErrorMacro("Invalid input volume node!");
    return;
  }

  vtkMRMLSequenceNode* masterSequence = inputSequenceBrowser->GetMasterSequenceNode();
  if (!masterSequence)
  {
    vtkErrorMacro("Invalid master sequence node!");
      return;
  }

  double rasBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  inputVolumeNode->GetRASBounds(rasBounds);

  const int numberOfFrames = masterSequence->GetNumberOfDataNodes();
  int selectedItemNumber = inputSequenceBrowser->GetSelectedItemNumber();
  for (int i = 0; i < numberOfFrames; ++i)
  {
    inputSequenceBrowser->SetSelectedItemNumber(i);

    double selectedRASBounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    inputVolumeNode->GetRASBounds(selectedRASBounds);
    for (int i = 0; i < 3; ++i)
    {
      rasBounds[2 * i] = std::min(selectedRASBounds[2 * i], rasBounds[2 * i]);
      rasBounds[2 * i + 1] = std::max(selectedRASBounds[2 * i + 1], rasBounds[2 * i + 1]);
    }
  }
  inputSequenceBrowser->SetSelectedItemNumber(selectedItemNumber);

  double radiusRAS[3] = { 0 };
  double centerRAS[3] = { 0 };
  for (int i = 0; i < 3; ++i)
  {
    radiusRAS[i] = 0.5 * (rasBounds[2 * i + 1] - rasBounds[2 * i]);
    centerRAS[i] = (rasBounds[2 * i + 1] + rasBounds[2 * i]) / 2.0;
  }
  outputROINodeRAS->SetXYZ(centerRAS);
  outputROINodeRAS->SetRadiusXYZ(radiusRAS);
}

//---------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumeReconstructionLogic::GetOrAddOutputVolumeNode(vtkMRMLVolumeReconstructionNode* volumeReconstructionNode)
{
  if (!volumeReconstructionNode)
  {
    vtkErrorMacro("Invalid volume reconstructor node!");
    return nullptr;
  }

  vtkSmartPointer<vtkMRMLVolumeNode> outputVolumeNode = volumeReconstructionNode->GetOutputVolumeNode();
  if (outputVolumeNode)
  {
    return outputVolumeNode;
  }
  outputVolumeNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();

  vtkMRMLVolumeNode* inputVolumeNode = volumeReconstructionNode->GetInputVolumeNode();
  if (inputVolumeNode && inputVolumeNode->GetName())
  {
    std::string roiNodeName = inputVolumeNode->GetName();
    roiNodeName += "_ReconstructedVolume";
    outputVolumeNode->SetName(roiNodeName.c_str());
  }
  else
  {
    outputVolumeNode->SetName("ReconstructedVolume");
  }

  if (this->GetMRMLScene())
  {
    this->GetMRMLScene()->AddNode(outputVolumeNode);
  }
  volumeReconstructionNode->SetAndObserveOutputVolumeNode(outputVolumeNode);
  return outputVolumeNode;
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "vtkSlicerVolumeReconstructionLogic: " << this->GetClassName() << "\n";
}
