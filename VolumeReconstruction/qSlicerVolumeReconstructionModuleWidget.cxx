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

// Qt includes
#include <QDebug>
#include <QProgressDialog>
#include <QStandardItemModel>
#include <QTreeView>
#include <QTextEdit>

#include <qSlicerApplication.h>
#include <qSlicerModuleManager.h>

// SlicerQt includes
#include "qSlicerVolumeReconstructionModuleWidget.h"
#include "ui_qSlicerVolumeReconstructionModule.h"
#include "qSlicerVolumeReconstructionModule.h"

// SlicerIGSIOCommon includes
#include "vtkSlicerIGSIOCommon.h"

// Sequence includes
#include <vtkMRMLSequenceBrowserNode.h>

// Slicer MRML includes
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLAnnotationDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>

// Volume reconstruction logic includes
#include "vtkSlicerVolumeReconstructionLogic.h"

// IGSIO volume reconstruction includes
#include <vtkIGSIOPasteSliceIntoVolume.h>

// IGSIO common includes
#include <vtkIGSIOTrackedFrameList.h>

// qMRMLWidgets includes
#include <qMRMLNodeFactory.h>
#include <vtkMetaImageWriter.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeReconstruction
class qSlicerVolumeReconstructionModuleWidgetPrivate : public Ui_qSlicerVolumeReconstructionModule
{
  Q_DECLARE_PUBLIC(qSlicerVolumeReconstructionModuleWidget);
protected:
  qSlicerVolumeReconstructionModuleWidget* const q_ptr;
public:
  qSlicerVolumeReconstructionModuleWidgetPrivate(qSlicerVolumeReconstructionModuleWidget& object);
  ~qSlicerVolumeReconstructionModuleWidgetPrivate();

  vtkSlicerVolumeReconstructionLogic* logic() const;
  QProgressDialog* ReconstructionProgressDialog;
};

//-----------------------------------------------------------------------------
// qSlicerVolumeReconstructionModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeReconstructionModuleWidgetPrivate::qSlicerVolumeReconstructionModuleWidgetPrivate(qSlicerVolumeReconstructionModuleWidget& object)
  : q_ptr(&object)
  , ReconstructionProgressDialog(nullptr)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeReconstructionModuleWidgetPrivate::~qSlicerVolumeReconstructionModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerVolumeReconstructionLogic* qSlicerVolumeReconstructionModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerVolumeReconstructionModuleWidget);
  return vtkSlicerVolumeReconstructionLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerVolumeReconstructionModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVolumeReconstructionModuleWidget::qSlicerVolumeReconstructionModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumeReconstructionModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeReconstructionModuleWidget::~qSlicerVolumeReconstructionModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::setup()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // TODO: Currently fan angles is not currently connected to logic.
  d->CollapsibleButtonFanAngles->setEnabled(false);
  d->CollapsibleButtonFanAngles->setVisible(false);

  d->InterpolationModeComboBox->addItem("Nearest neighbor", vtkIGSIOPasteSliceIntoVolume::InterpolationType::NEAREST_NEIGHBOR_INTERPOLATION);
  d->InterpolationModeComboBox->addItem("Linear", vtkIGSIOPasteSliceIntoVolume::InterpolationType::LINEAR_INTERPOLATION);

  if (vtkIGSIOPasteSliceIntoVolume::IsGpuAccelerationSupported()) {
    d->OptimizationModeComboBox->addItem("GPU acceleration (OpenCL)", vtkIGSIOPasteSliceIntoVolume::OptimizationType::GPU_ACCELERATION_OPENCL);
  }
  d->OptimizationModeComboBox->addItem("Full optimization", vtkIGSIOPasteSliceIntoVolume::OptimizationType::FULL_OPTIMIZATION);
  d->OptimizationModeComboBox->addItem("Partial optimization", vtkIGSIOPasteSliceIntoVolume::OptimizationType::PARTIAL_OPTIMIZATION);
  d->OptimizationModeComboBox->addItem("No optimization", vtkIGSIOPasteSliceIntoVolume::OptimizationType::NO_OPTIMIZATION);  

  d->CompoundingModeComboBox->addItem("Latest", vtkIGSIOPasteSliceIntoVolume::CompoundingType::LATEST_COMPOUNDING_MODE);
  d->CompoundingModeComboBox->addItem("Maximum", vtkIGSIOPasteSliceIntoVolume::CompoundingType::MAXIMUM_COMPOUNDING_MODE);
  d->CompoundingModeComboBox->addItem("Mean", vtkIGSIOPasteSliceIntoVolume::CompoundingType::MEAN_COMPOUNDING_MODE);
  d->CompoundingModeComboBox->addItem("Importance mask", vtkIGSIOPasteSliceIntoVolume::CompoundingType::IMPORTANCE_MASK_COMPOUNDING_MODE);

  connect(d->InputSequenceBrowserSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateWidgetFromMRML()));
  connect(d->OutputVolumeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateWidgetFromMRML()));
  connect(d->ReconstructionROISelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateWidgetFromMRML()));
  connect(d->ROIVisibilityButton, SIGNAL(clicked()), this, SLOT(onToggleROIVisible()));
  connect(d->ApplyButton, SIGNAL(clicked()), this, SLOT(onApply()));

  qvtkConnect(d->logic(), vtkSlicerVolumeReconstructionLogic::VolumeReconstructionStarted, this, SLOT(startProgressDialog()));
  qvtkConnect(d->logic(), vtkSlicerVolumeReconstructionLogic::VolumeAddedToReconstruction, this, SLOT(updateReconstructionProgress()));
  qvtkConnect(d->logic(), vtkSlicerVolumeReconstructionLogic::VolumeReconstructionFinished, this, SLOT(stopProgressDialog()));
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::startProgressDialog()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);
  if (d->ReconstructionProgressDialog)
  {
    delete d->ReconstructionProgressDialog;
    d->ReconstructionProgressDialog = nullptr;
  }

  d->ReconstructionProgressDialog = new QProgressDialog("Volume reconstruction", "",
    0, d->logic()->GetNumberOfVolumeNodesForReconstructionInInput(), this);
  d->ReconstructionProgressDialog->setWindowTitle(QString("Reconstructing volume..."));
  d->ReconstructionProgressDialog->setCancelButton(nullptr);
  d->ReconstructionProgressDialog->setWindowFlags(d->ReconstructionProgressDialog->windowFlags()
    & ~Qt::WindowCloseButtonHint & ~Qt::WindowContextHelpButtonHint);
  d->ReconstructionProgressDialog->setFixedSize(d->ReconstructionProgressDialog->sizeHint());
  d->ReconstructionProgressDialog->setWindowModality(Qt::WindowModal);
  this->updateReconstructionProgress();
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::stopProgressDialog()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);
  if (d->ReconstructionProgressDialog)
  {
    delete d->ReconstructionProgressDialog;
    d->ReconstructionProgressDialog = nullptr;
  }
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::updateReconstructionProgress()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);
  if (!d->ReconstructionProgressDialog)
  {
    return;
  }
  d->ReconstructionProgressDialog->setValue(d->logic()->GetVolumeNodesAddedToReconstruction());
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);

  QVariant currentVolumeData = d->InputVolumeNodeSelector->currentData();
  d->InputVolumeNodeSelector->clear();

  vtkMRMLSequenceBrowserNode* inputSequenceBrowser = vtkMRMLSequenceBrowserNode::SafeDownCast(d->InputSequenceBrowserSelector->currentNode());
  std::vector<vtkMRMLNode*> proxyNodes;
  if (inputSequenceBrowser)
  {
    inputSequenceBrowser->GetAllProxyNodes(proxyNodes);
  }
  for (vtkMRMLNode* proxyNode : proxyNodes)
  {
    vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(proxyNode);
    if (!volumeNode)
    {
      continue;
    }
    d->InputVolumeNodeSelector->addItem(volumeNode->GetName(), volumeNode->GetID());
  }

  d->InputVolumeNodeSelector->setCurrentIndex(d->InputVolumeNodeSelector->findData(currentVolumeData));
  if (d->InputVolumeNodeSelector->currentIndex() == -1 && d->InputVolumeNodeSelector->count() > 0)
  {
    d->InputVolumeNodeSelector->setCurrentIndex(0);
  }

  vtkMRMLScalarVolumeNode* outputVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->OutputVolumeSelector->currentNode());
  d->ApplyButton->setEnabled(inputSequenceBrowser && outputVolumeNode && d->InputVolumeNodeSelector->currentIndex() > -1);

  vtkMRMLAnnotationROINode* reconstructionROINode = vtkMRMLAnnotationROINode::SafeDownCast(d->ReconstructionROISelector->currentNode());
  d->ROIVisibilityButton->setEnabled(reconstructionROINode);
  if (reconstructionROINode)
  {
    d->ROIVisibilityButton->setChecked(reconstructionROINode->GetDisplayVisibility());
  }
  else
  {
    d->ROIVisibilityButton->setChecked(false);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::onToggleROIVisible()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);
  vtkMRMLAnnotationROINode* reconstructionROINode = vtkMRMLAnnotationROINode::SafeDownCast(d->ReconstructionROISelector->currentNode());
  if (!reconstructionROINode)
  {
    return;
  }

  reconstructionROINode->SetDisplayVisibility(d->ROIVisibilityButton->isChecked());
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::onApply()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);

  if (!this->mrmlScene())
  {
    return;
  }

  std::string inputVolumeID = d->InputVolumeNodeSelector->currentData().toString().toStdString();

  vtkMRMLSequenceBrowserNode* inputSequenceBrowser = vtkMRMLSequenceBrowserNode::SafeDownCast(d->InputSequenceBrowserSelector->currentNode());
  vtkMRMLVolumeNode* inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->mrmlScene()->GetNodeByID(inputVolumeID));
  vtkMRMLScalarVolumeNode* outputVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->OutputVolumeSelector->currentNode());
  vtkMRMLAnnotationROINode* reconstructionROINode = vtkMRMLAnnotationROINode::SafeDownCast(d->ReconstructionROISelector->currentNode());

  double outputSpacing[3] = { 0,0,0 };
  outputSpacing[0] = d->XSpacingSpinbox->value();
  outputSpacing[1] = d->YSpacingSpinbox->value();
  outputSpacing[2] = d->ZSpacingSpinbox->value();

  int interpolationMode = d->InterpolationModeComboBox->currentData().toInt();
  int optimizationMode = d->OptimizationModeComboBox->currentData().toInt();
  int compoundingMode = d->CompoundingModeComboBox->currentData().toInt();
  bool fillHoles = d->FillHolesCheckBox->isChecked();
  int numberOfThreads = d->NumberOfThreadsSpinBox->value();

  int clipRectangleOrigin[2];
  clipRectangleOrigin[0] = d->XClipRectangleOriginSpinBox->value();
  clipRectangleOrigin[1] = d->YClipRectangleOriginSpinBox->value();
  int clipRectangleSize[2];
  clipRectangleSize[0] = d->XClipRectangleSizeSpinBox->value();
  clipRectangleSize[1] = d->YClipRectangleSizeSpinBox->value();

  d->logic()->ReconstructVolume(
    inputSequenceBrowser, inputVolumeNode, outputVolumeNode, reconstructionROINode,
    clipRectangleOrigin, clipRectangleSize,
    outputSpacing,
    interpolationMode, optimizationMode, compoundingMode, fillHoles,
    numberOfThreads);
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);

  this->Superclass::setMRMLScene(scene);
  this->updateWidgetFromMRML();
  if (scene == NULL)
  {
    return;
  }
}
