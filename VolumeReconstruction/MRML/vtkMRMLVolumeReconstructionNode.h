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

#ifndef __vtkMRMLVolumeReconstructionNode_h
#define __vtkMRMLVolumeReconstructionNode_h

// MRML includes
#include <vtkMRMLNode.h>

// vtkAddon includes  
#include <vtkAddonSetGet.h>

// Volume Reconstruction includes
#include "vtkSlicerVolumeReconstructionModuleMRMLExport.h"

// IGSIO includes
#include <vtkIGSIOPasteSliceIntoVolume.h>;

class vtkMRMLAnnotationROINode;
class vtkMRMLSequenceBrowserNode;
class vtkMRMLVolumeNode;

/// \ingroup VolumeReconstruction
/// \brief Parameter set node for volume reconstruction
///
class VTK_SLICER_VOLUMERECONSTRUCTION_MODULE_MRML_EXPORT vtkMRMLVolumeReconstructionNode : public vtkMRMLNode
{

public:

  static vtkMRMLVolumeReconstructionNode* New();
  vtkTypeMacro(vtkMRMLVolumeReconstructionNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Standard MRML node methods
  virtual vtkMRMLNode* CreateNodeInstance() override;
  virtual void ReadXMLAttributes(const char** atts) override;
  virtual void WriteXML(ostream& of, int indent) override;
  virtual void Copy(vtkMRMLNode* node) override;
  virtual const char* GetNodeTagName() override { return "VolumeReconstruction"; }
  void ProcessMRMLEvents(vtkObject* caller, unsigned long eventID, void* callData) override;

protected:
  vtkMRMLVolumeReconstructionNode();
  virtual ~vtkMRMLVolumeReconstructionNode();
  vtkMRMLVolumeReconstructionNode(const vtkMRMLVolumeReconstructionNode&);
  void operator=(const vtkMRMLVolumeReconstructionNode&);

public:

  enum
  {
    VolumeReconstructionStarted = 56000,
    VolumeAddedToReconstruction,
    VolumeReconstructionFinished,
    InputVolumeModified,
  };

  enum InterpolationType
  {
    NEAREST_NEIGHBOR_INTERPOLATION = vtkIGSIOPasteSliceIntoVolume::NEAREST_NEIGHBOR_INTERPOLATION,
    LINEAR_INTERPOLATION = vtkIGSIOPasteSliceIntoVolume::LINEAR_INTERPOLATION
  };

  enum OptimizationType
  {
    NO_OPTIMIZATION = vtkIGSIOPasteSliceIntoVolume::NO_OPTIMIZATION,
    PARTIAL_OPTIMIZATION = vtkIGSIOPasteSliceIntoVolume::PARTIAL_OPTIMIZATION,
    FULL_OPTIMIZATION = vtkIGSIOPasteSliceIntoVolume::FULL_OPTIMIZATION,
    GPU_ACCELERATION_OPENCL = vtkIGSIOPasteSliceIntoVolume::GPU_ACCELERATION_OPENCL
  };

  enum CompoundingType
  {
    UNDEFINED_COMPOUNDING_MODE = vtkIGSIOPasteSliceIntoVolume::UNDEFINED_COMPOUNDING_MODE,
    LATEST_COMPOUNDING_MODE = vtkIGSIOPasteSliceIntoVolume::LATEST_COMPOUNDING_MODE,
    MAXIMUM_COMPOUNDING_MODE = vtkIGSIOPasteSliceIntoVolume::MAXIMUM_COMPOUNDING_MODE,
    MEAN_COMPOUNDING_MODE = vtkIGSIOPasteSliceIntoVolume::MEAN_COMPOUNDING_MODE,
    IMPORTANCE_MASK_COMPOUNDING_MODE = vtkIGSIOPasteSliceIntoVolume::IMPORTANCE_MASK_COMPOUNDING_MODE
  };

  const char* GetInputSequenceBrowserNodeReferenceRole() { return "inputSequenceBrowserNode"; };
  const char* GetInputSequenceBrowserNodeReferenceMRMLAttributeName() { return "inputSequenceBrowserNodeRef"; };

  const char* GetInputVolumeNodeReferenceRole() { return "inputVolumeNode"; };
  const char* GetInputVolumeNodeReferenceMRMLAttributeName() { return "inputVolumeNodeRef"; };

  const char* GetInputROINodeReferenceRole() { return "inputROINode"; };
  const char* GetInputROINodeReferenceMRMLAttributeName() { return "inputROINodeRef"; };

  const char* GetOutputVolumeNodeReferenceRole() { return "outputVolumeNode"; };
  const char* GetOutputVolumeNodeReferenceMRMLAttributeName() { return "outputVolumeNodeRef"; };

  vtkMRMLSequenceBrowserNode* GetInputSequenceBrowserNode();
  vtkMRMLVolumeNode* GetInputVolumeNode();
  vtkMRMLAnnotationROINode* GetInputROINode();
  vtkMRMLVolumeNode* GetOutputVolumeNode();

  virtual void SetAndObserveInputSequenceBrowserNode(vtkMRMLSequenceBrowserNode* sequenceBrowserNode);
  virtual void SetAndObserveInputVolumeNode(vtkMRMLVolumeNode* volumeNode);
  virtual void SetAndObserveInputROINode(vtkMRMLAnnotationROINode* roiNode);
  virtual void SetAndObserveOutputVolumeNode(vtkMRMLVolumeNode* volumeNode);

  vtkSetMacro(LiveVolumeReconstruction, bool);
  vtkGetMacro(LiveVolumeReconstruction, bool);

  vtkSetMacro(LiveUpdateIntervalSeconds, double);
  vtkGetMacro(LiveUpdateIntervalSeconds, double);

  vtkSetVector2Macro(ClipRectangleOrigin, int);
  vtkGetVector2Macro(ClipRectangleOrigin, int);

  vtkSetVector2Macro(ClipRectangleSize, int);
  vtkGetVector2Macro(ClipRectangleSize, int);

  vtkSetVector3Macro(OutputSpacing, double);
  vtkGetVector3Macro(OutputSpacing, double);

  vtkSetMacro(InterpolationMode, int);
  vtkGetMacro(InterpolationMode, int);

  vtkSetMacro(OptimizationMode, int);
  vtkGetMacro(OptimizationMode, int);

  vtkSetMacro(CompoundingMode, int);
  vtkGetMacro(CompoundingMode, int);

  vtkSetMacro(FillHoles, bool);
  vtkGetMacro(FillHoles, bool);

  vtkSetMacro(NumberOfThreads, int);
  vtkGetMacro(NumberOfThreads, int);

  void SetNumberOfVolumesAddedToReconstruction(int numberOfVolumesAddedToReconstruction);
  vtkGetMacro(NumberOfVolumesAddedToReconstruction, int);

  vtkSetMacro(LiveVolumeReconstructionInProgress, bool);
  vtkGetMacro(LiveVolumeReconstructionInProgress, bool);
  vtkBooleanMacro(LiveVolumeReconstructionInProgress, bool);

protected:
  bool LiveVolumeReconstruction;
  double LiveUpdateIntervalSeconds;
  int ClipRectangleOrigin[2];
  int ClipRectangleSize[2];
  double OutputSpacing[3];
  int InterpolationMode;
  int OptimizationMode;
  int CompoundingMode;
  bool FillHoles;
  int NumberOfThreads;
  int NumberOfVolumesAddedToReconstruction;
  bool LiveVolumeReconstructionInProgress;
};

#endif // __vtkMRMLVolumeReconstructionNode_h
