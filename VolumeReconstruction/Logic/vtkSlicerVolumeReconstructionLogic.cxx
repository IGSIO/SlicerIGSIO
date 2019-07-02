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
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLAnnotationROINode.h>

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

// Volumes includes
#include <vtkStreamingVolumeCodecFactory.h>

// VolumeReconstruction includes
#include "vtkSlicerVolumeReconstructionLogic.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerVolumeReconstructionLogic);

//---------------------------------------------------------------------------
class vtkSlicerVolumeReconstructionLogic::vtkInternal
{
public:
  //---------------------------------------------------------------------------
  vtkInternal(vtkSlicerVolumeReconstructionLogic* external);
  ~vtkInternal();

  vtkNew<vtkIGSIOVolumeReconstructor> Reconstructor;

  vtkSlicerVolumeReconstructionLogic* External;
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
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::StartReconstruction(vtkMRMLAnnotationROINode* roiNode)
{
}

//---------------------------------------------------------------------------
bool vtkSlicerVolumeReconstructionLogic::AddVolumeNodeToReconstructedVolume(vtkMRMLVolumeNode* volumeNode)
{
  if (!volumeNode->GetImageData())
  {
    vtkErrorMacro("Input volume node has no image data!");
    return false;
  }

  vtkNew<vtkTransform> imageToWorldTransform;
  imageToWorldTransform->Identity();
  imageToWorldTransform->PreMultiply();
  vtkNew<vtkMatrix4x4> ijkToRASMatrix;
  volumeNode->GetIJKToRASMatrix(ijkToRASMatrix);
  imageToWorldTransform->Concatenate(ijkToRASMatrix);

  vtkNew<vtkMatrix4x4> parentToWorldMatrix;
  vtkMRMLTransformNode* transformNode = volumeNode->GetParentTransformNode();
  if (transformNode)
  {
    transformNode->GetMatrixTransformToWorld(parentToWorldMatrix);
    imageToWorldTransform->Concatenate(parentToWorldMatrix);
  }

  vtkNew<vtkIGSIOTransformRepository> transformRepository;
  transformRepository->SetTransform(igsioTransformName("ImageToWorld"), imageToWorldTransform->GetMatrix());

  std::string errorDetail;
  igsioTrackedFrame trackedFrame;
  trackedFrame.GetImageData()->DeepCopyFrom(volumeNode->GetImageData());

  bool insertedIntoVolume = false;
  if (this->Internal->Reconstructor->AddTrackedFrame(&trackedFrame, transformRepository, &insertedIntoVolume) != IGSIO_SUCCESS)
  {
    return false;
  }

  this->SetVolumeNodesAddedToReconstruction(this->GetVolumeNodesAddedToReconstruction() + 1);
  this->InvokeEvent(vtkSlicerVolumeReconstructionLogic::VolumeAddedToReconstruction);
  return  true;
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::ReconstructVolume(
  vtkMRMLSequenceBrowserNode* inputSequenceBrowser,
  vtkMRMLVolumeNode* inputVolumeNode,
  vtkMRMLScalarVolumeNode* outputVolumeNode,
  vtkMRMLAnnotationROINode* roiNode,
  int clipRectangleOrigin[2],
  int clipRectangleSize[2],
  double outputSpacing[3],
  int interpolationMode,
  int optimizationMode,
  int compoundingMode,
  bool fillHoles,
  int numberOfThreads)
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

  if (!outputVolumeNode)
  {
    vtkErrorMacro("Invalid output volume node!");
    return;
  }

  vtkMRMLSequenceNode* masterSequence = inputSequenceBrowser->GetMasterSequenceNode();
  if (!masterSequence)
  {
  vtkErrorMacro("Invalid master sequence node!")
  return;
  }

  const int numberOfFrames = masterSequence->GetNumberOfDataNodes();
  this->SetNumberOfVolumeNodesForReconstructionInInput(numberOfFrames);
  this->SetVolumeNodesAddedToReconstruction(0);
  this->InvokeEvent(VolumeReconstructionStarted);

  this->Internal->Reconstructor->SetOutputSpacing(outputSpacing);
  this->Internal->Reconstructor->SetCompoundingMode(vtkIGSIOPasteSliceIntoVolume::CompoundingType(compoundingMode));
  this->Internal->Reconstructor->SetOptimization(vtkIGSIOPasteSliceIntoVolume::OptimizationType(optimizationMode));
  this->Internal->Reconstructor->SetInterpolation(vtkIGSIOPasteSliceIntoVolume::InterpolationType(interpolationMode));
  this->Internal->Reconstructor->SetNumberOfThreads(numberOfThreads);
  this->Internal->Reconstructor->SetFillHoles(fillHoles);
  this->Internal->Reconstructor->SetImageCoordinateFrame("Image");
  this->Internal->Reconstructor->SetReferenceCoordinateFrame("World");
  this->Internal->Reconstructor->SetClipRectangleOrigin(clipRectangleOrigin);
  this->Internal->Reconstructor->SetClipRectangleSize(clipRectangleSize);

  if (roiNode)
  {
    double bounds[6];
    roiNode->GetBounds(bounds);
    int extent[6] = {
      0, static_cast<int>(std::ceil((bounds[1] - bounds[0]) / outputSpacing[0])),
      0, static_cast<int>(std::ceil((bounds[3] - bounds[2]) / outputSpacing[1])),
      0, static_cast<int>(std::ceil((bounds[5] - bounds[4]) / outputSpacing[2]))
    };
    this->Internal->Reconstructor->SetOutputExtent(extent);
    double outputOrigin[3] = { bounds[0], bounds[2], bounds[4] };
    this->Internal->Reconstructor->SetOutputOrigin(outputOrigin);
  }
  else
  {
    vtkNew<vtkIGSIOTrackedFrameList> trackedFrameList;
    vtkSlicerIGSIOCommon::SequenceBrowserToTrackedFrameList(inputSequenceBrowser, trackedFrameList);
    std::string errorDescription;
    this->Internal->Reconstructor->SetOutputExtentFromFrameList(trackedFrameList, vtkNew<vtkIGSIOTransformRepository>(), errorDescription);
  }

  int selectedItemNumber = inputSequenceBrowser->GetSelectedItemNumber();

  this->Internal->Reconstructor->Reset();

  for (int i = 0; i < numberOfFrames; ++i)
  {
    vtkMRMLSequenceNode* imageSequence = masterSequence;
    inputSequenceBrowser->SetSelectedItemNumber(i);
    this->AddVolumeNodeToReconstructedVolume(inputVolumeNode);
  }

  if (!outputVolumeNode->GetImageData())
  {
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    outputVolumeNode->SetAndObserveImageData(imageData);
  }

  if (this->Internal->Reconstructor->GetReconstructedVolume(outputVolumeNode->GetImageData()) != IGSIO_SUCCESS)
  {
    vtkErrorMacro("Could not retreive reconstructed image");
  }

  double spacing[3];
  outputVolumeNode->GetImageData()->GetSpacing(spacing);
  outputVolumeNode->GetImageData()->SetSpacing(1, 1, 1);
  outputVolumeNode->SetSpacing(spacing);

  double origin[3];
  outputVolumeNode->GetImageData()->GetOrigin(origin);
  outputVolumeNode->GetImageData()->SetOrigin(0, 0, 0);
  outputVolumeNode->SetOrigin(origin);

  inputSequenceBrowser->SetSelectedItemNumber(selectedItemNumber);

  this->InvokeEvent(VolumeReconstructionFinished);
}

//---------------------------------------------------------------------------
void vtkSlicerVolumeReconstructionLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkSlicerVolumeReconstructionLogic:             " << this->GetClassName() << "\n";
}
