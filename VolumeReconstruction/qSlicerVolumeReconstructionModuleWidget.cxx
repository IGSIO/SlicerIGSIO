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
#include <vtkMRMLSequenceNode.h>

// Slicer MRML includes
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLAnnotationDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>

// VolumeReconstruction logic includes
#include "vtkSlicerVolumeReconstructionLogic.h"

// VolumeReconstruction MRML includes
#include <vtkMRMLVolumeReconstructionNode.h>

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
  QTimer* LiveUpdateIntervalTimer;

  vtkWeakPointer<vtkMRMLVolumeReconstructionNode> VolumeReconstructionNode;

};

//-----------------------------------------------------------------------------
// qSlicerVolumeReconstructionModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeReconstructionModuleWidgetPrivate::qSlicerVolumeReconstructionModuleWidgetPrivate(qSlicerVolumeReconstructionModuleWidget& object)
  : q_ptr(&object)
  , ReconstructionProgressDialog(nullptr)
  , VolumeReconstructionNode(nullptr)
  , LiveUpdateIntervalTimer(new QTimer(&object))
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

  connect(d->VolumeReconstructionSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onVolumeReconstructionNodeChanged(vtkMRMLNode*)));

  connect(d->LiveUpdateIntervalTimer, SIGNAL(timeout()), this, SLOT(onLiveUpdateIntervalTimeout()));

  connect(d->LiveReconstructionRadioButton, SIGNAL(clicked()), this, SLOT(updateMRMLFromWidget()));
  connect(d->RecordedReconstructionRadioButton, SIGNAL(clicked()), this, SLOT(updateMRMLFromWidget()));

  connect(d->InputSequenceBrowserSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateMRMLFromWidget()));
  connect(d->InputVolumeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateMRMLFromWidget()));
  connect(d->OutputVolumeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateMRMLFromWidget()));

  connect(d->InputROISelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(updateMRMLFromWidget()));
  connect(d->ROIVisibilityButton, SIGNAL(clicked()), this, SLOT(onToggleROIVisible()));

  // Volume reconstruction parameters
  connect(d->LiveUpdateIntervalSpinBox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->XSpacingSpinbox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->YSpacingSpinbox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->ZSpacingSpinbox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->InterpolationModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMRMLFromWidget()));
  connect(d->OptimizationModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMRMLFromWidget()));
  connect(d->CompoundingModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMRMLFromWidget()));
  connect(d->FillHolesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(updateMRMLFromWidget()));
  connect(d->NumberOfThreadsSpinBox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->SkipIntervalSpinbox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->XClipRectangleOriginSpinBox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->YClipRectangleOriginSpinBox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->XClipRectangleSizeSpinBox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->YClipRectangleSizeSpinBox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->MinFanAngleSpinbox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));
  connect(d->MaxFanAngleSpinbox, SIGNAL(valueChanged(QString)), this, SLOT(updateMRMLFromWidget()));

  connect(d->ResetButton, SIGNAL(clicked()), this, SLOT(onReset()));
  connect(d->ApplyButton, SIGNAL(clicked()), this, SLOT(onApply()));
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::enter()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);

  // Make sure a command line module node is available when the module widget
  // is activated. If no CLI node is available then create a new one.
  if (d->VolumeReconstructionSelector->currentNode() == nullptr)
  {
    vtkMRMLVolumeReconstructionNode* node = vtkMRMLVolumeReconstructionNode::SafeDownCast(d->VolumeReconstructionSelector->addNode());
    d->VolumeReconstructionSelector->setCurrentNode(node);
  }
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
    0, 100, this);
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

  if (!d->VolumeReconstructionNode)
  {
    return;
  }

  vtkMRMLSequenceBrowserNode* inputSequenceBrowser = d->VolumeReconstructionNode->GetInputSequenceBrowserNode();
  if (!inputSequenceBrowser)
  {
    return;
  }

  vtkMRMLSequenceNode* masterSequence = inputSequenceBrowser->GetMasterSequenceNode();
  if (!masterSequence)
  {
    return;
  }

  int numberOfFrames = masterSequence->GetNumberOfDataNodes();
  int progress = std::floor((100.0 * d->VolumeReconstructionNode->GetNumberOfVolumesAddedToReconstruction()) / numberOfFrames);
  d->ReconstructionProgressDialog->setValue(progress);
  qApp->processEvents();
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::onVolumeReconstructionNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }

  vtkMRMLVolumeReconstructionNode* volumeReconstructionNode = vtkMRMLVolumeReconstructionNode::SafeDownCast(node);
  qvtkReconnect(d->VolumeReconstructionNode, volumeReconstructionNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->VolumeReconstructionNode = volumeReconstructionNode;
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);

  if (!d->VolumeReconstructionNode)
  {
    d->ParameterWidget->setEnabled(false);
    d->ApplyButton->setEnabled(false);
    return;
  }
  d->ParameterWidget->setEnabled(true);
  d->ApplyButton->setEnabled(true);

  bool liveReconstruction = d->VolumeReconstructionNode->GetLiveVolumeReconstruction();
  bool wasBlocking = d->LiveReconstructionRadioButton->blockSignals(true);
  d->LiveReconstructionRadioButton->setChecked(liveReconstruction);
  d->LiveReconstructionRadioButton->blockSignals(wasBlocking);

  wasBlocking = d->LiveUpdateIntervalSpinBox->blockSignals(true);
  d->LiveUpdateIntervalSpinBox->setValue(d->VolumeReconstructionNode->GetLiveUpdateIntervalSeconds());
  d->LiveUpdateIntervalSpinBox->blockSignals(wasBlocking);

  wasBlocking = d->RecordedReconstructionRadioButton->blockSignals(true);
  d->RecordedReconstructionRadioButton->setChecked(!liveReconstruction);
  d->RecordedReconstructionRadioButton->blockSignals(wasBlocking);

  if (liveReconstruction)
  {
    d->InputSequenceBrowserSelector->setEnabled(false);
    d->ResetButton->setEnabled(true);
    d->ApplyButton->setCheckable(true);
    d->ApplyButton->setText("Start live volume reconstruction");
  }
  else
  {
    d->InputSequenceBrowserSelector->setEnabled(true);
    d->ResetButton->setEnabled(false);
    d->ApplyButton->setCheckable(false);
    d->ApplyButton->setText("Apply");
  }

  int liveUpdateIntervalMilliSeconds = d->VolumeReconstructionNode->GetLiveUpdateIntervalSeconds() * 1000;
  if (d->LiveUpdateIntervalTimer->interval() != liveUpdateIntervalMilliSeconds)
  {
    d->LiveUpdateIntervalTimer->setInterval(liveUpdateIntervalMilliSeconds);
  }

  if (liveReconstruction && d->VolumeReconstructionNode->GetLiveVolumeReconstructionInProgress())
  {
    d->ResetButton->setEnabled(false);
    d->ParameterWidget->setEnabled(false);
  }

  vtkMRMLVolumeNode* volumeNode = d->VolumeReconstructionNode->GetInputVolumeNode();
  QVariant currentVolumeData = volumeNode ? volumeNode->GetName() : "";

  vtkMRMLSequenceBrowserNode* inputSequenceBrowser = d->VolumeReconstructionNode->GetInputSequenceBrowserNode();
  wasBlocking = d->InputSequenceBrowserSelector->blockSignals(true);
  d->InputSequenceBrowserSelector->setCurrentNode(inputSequenceBrowser);
  d->InputSequenceBrowserSelector->blockSignals(wasBlocking);

  vtkMRMLVolumeNode* inputVolumeNode = d->VolumeReconstructionNode->GetInputVolumeNode();
  wasBlocking = d->InputVolumeSelector->blockSignals(true);
  d->InputVolumeSelector->setCurrentNode(inputVolumeNode);
  d->InputVolumeSelector->blockSignals(false);

  vtkMRMLVolumeNode* outputVolumeNode = d->VolumeReconstructionNode->GetOutputVolumeNode();
  wasBlocking = d->OutputVolumeSelector->blockSignals(true);
  d->OutputVolumeSelector->setCurrentNode(outputVolumeNode);
  d->OutputVolumeSelector->blockSignals(wasBlocking);

  vtkMRMLAnnotationROINode* inputROINode = d->VolumeReconstructionNode->GetInputROINode();
  wasBlocking = d->InputROISelector->blockSignals(true);
  d->InputROISelector->setCurrentNode(inputROINode);
  d->InputROISelector->blockSignals(wasBlocking);

  d->ROIVisibilityButton->setEnabled(inputROINode);
  if (inputROINode)
  {
    d->ROIVisibilityButton->setChecked(inputROINode->GetDisplayVisibility());
  }
  else
  {
    d->ROIVisibilityButton->setChecked(false);
  }

  if (liveReconstruction)
  {
    d->ApplyButton->setEnabled(inputVolumeNode && inputROINode);
  }
  else
  {
    d->ApplyButton->setEnabled(inputSequenceBrowser && inputVolumeNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::updateMRMLFromWidget()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);
  if (!d->VolumeReconstructionNode)
  {
    return;
  }

  MRMLNodeModifyBlocker(d->VolumeReconstructionNode);

  bool liveVolumeReconstruction = d->LiveReconstructionRadioButton->isChecked();
  if (liveVolumeReconstruction != d->VolumeReconstructionNode->GetLiveVolumeReconstruction())
  {
    d->VolumeReconstructionNode->SetNumberOfVolumesAddedToReconstruction(0);
  }
  d->VolumeReconstructionNode->SetLiveVolumeReconstruction(liveVolumeReconstruction);
  d->VolumeReconstructionNode->SetLiveUpdateIntervalSeconds(d->LiveUpdateIntervalSpinBox->value());

  vtkMRMLSequenceBrowserNode* inputSequenceBrowser = vtkMRMLSequenceBrowserNode::SafeDownCast(d->InputSequenceBrowserSelector->currentNode());
  d->VolumeReconstructionNode->SetAndObserveInputSequenceBrowserNode(inputSequenceBrowser);

  vtkMRMLVolumeNode* inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->InputVolumeSelector->currentNode());
  d->VolumeReconstructionNode->SetAndObserveInputVolumeNode(inputVolumeNode);

  vtkMRMLScalarVolumeNode* outputVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->OutputVolumeSelector->currentNode());
  d->VolumeReconstructionNode->SetAndObserveOutputVolumeNode(outputVolumeNode);

  vtkMRMLAnnotationROINode* reconstructionROINode = vtkMRMLAnnotationROINode::SafeDownCast(d->InputROISelector->currentNode());
  d->VolumeReconstructionNode->SetAndObserveInputROINode(reconstructionROINode);

  double outputSpacing[3] = { 0,0,0 };
  outputSpacing[0] = d->XSpacingSpinbox->value();
  outputSpacing[1] = d->YSpacingSpinbox->value();
  outputSpacing[2] = d->ZSpacingSpinbox->value();
  d->VolumeReconstructionNode->SetOutputSpacing(outputSpacing);

  int interpolationMode = d->InterpolationModeComboBox->currentData().toInt();
  d->VolumeReconstructionNode->SetInterpolationMode(interpolationMode);

  int optimizationMode = d->OptimizationModeComboBox->currentData().toInt();
  d->VolumeReconstructionNode->SetOptimizationMode(optimizationMode);

  int compoundingMode = d->CompoundingModeComboBox->currentData().toInt();
  d->VolumeReconstructionNode->SetCompoundingMode(compoundingMode);

  bool fillHoles = d->FillHolesCheckBox->isChecked();
  d->VolumeReconstructionNode->SetFillHoles(fillHoles);

  int numberOfThreads = d->NumberOfThreadsSpinBox->value();
  d->VolumeReconstructionNode->SetNumberOfThreads(numberOfThreads);

  int clipRectangleOrigin[2];
  clipRectangleOrigin[0] = d->XClipRectangleOriginSpinBox->value();
  clipRectangleOrigin[1] = d->YClipRectangleOriginSpinBox->value();
  d->VolumeReconstructionNode->SetClipRectangleOrigin(clipRectangleOrigin);

  int clipRectangleSize[2];
  clipRectangleSize[0] = d->XClipRectangleSizeSpinBox->value();
  clipRectangleSize[1] = d->YClipRectangleSizeSpinBox->value();
  d->VolumeReconstructionNode->SetClipRectangleSize(clipRectangleSize);
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::onToggleROIVisible()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);
  vtkMRMLAnnotationROINode* reconstructionROINode = vtkMRMLAnnotationROINode::SafeDownCast(d->InputROISelector->currentNode());
  if (!reconstructionROINode)
  {
    return;
  }

  reconstructionROINode->SetDisplayVisibility(d->ROIVisibilityButton->isChecked());
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::onReset()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);

  if (!this->mrmlScene())
  {
    return;
  }

  if (!d->VolumeReconstructionNode)
  {
    return;
  }

  d->VolumeReconstructionNode->SetNumberOfVolumesAddedToReconstruction(0);
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::onApply()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);

  if (!this->mrmlScene())
  {
    return;
  }

  if (!d->VolumeReconstructionNode)
  {
    return;
  }

  if (d->VolumeReconstructionNode->GetLiveVolumeReconstruction())
  {
    if (!d->VolumeReconstructionNode->GetLiveVolumeReconstructionInProgress())
    {
      if (d->VolumeReconstructionNode->GetNumberOfVolumesAddedToReconstruction() == 0)
      {
        d->logic()->StartLiveVolumeReconstruction(d->VolumeReconstructionNode);
      }
      else
      {
        d->logic()->ResumeLiveVolumeReconstruction(d->VolumeReconstructionNode);
      }
      d->LiveUpdateIntervalTimer->start();
    }
    else
    {
      d->LiveUpdateIntervalTimer->stop();
      d->logic()->StopLiveVolumeReconstruction(d->VolumeReconstructionNode);
    }
  }
  else
  {
    qvtkConnect(d->VolumeReconstructionNode, vtkMRMLVolumeReconstructionNode::VolumeReconstructionStarted, this, SLOT(startProgressDialog()));
    qvtkConnect(d->VolumeReconstructionNode, vtkMRMLVolumeReconstructionNode::VolumeAddedToReconstruction, this, SLOT(updateReconstructionProgress()));
    qvtkConnect(d->VolumeReconstructionNode, vtkMRMLVolumeReconstructionNode::VolumeReconstructionFinished, this, SLOT(stopProgressDialog()));
    d->logic()->ReconstructVolumeFromSequence(d->VolumeReconstructionNode);
    qvtkDisconnect(d->VolumeReconstructionNode, vtkMRMLVolumeReconstructionNode::VolumeReconstructionStarted, this, SLOT(startProgressDialog()));
    qvtkDisconnect(d->VolumeReconstructionNode, vtkMRMLVolumeReconstructionNode::VolumeAddedToReconstruction, this, SLOT(updateReconstructionProgress()));
    qvtkDisconnect(d->VolumeReconstructionNode, vtkMRMLVolumeReconstructionNode::VolumeReconstructionFinished, this, SLOT(stopProgressDialog()));
  }
}

//-----------------------------------------------------------------------------
void qSlicerVolumeReconstructionModuleWidget::onLiveUpdateIntervalTimeout()
{
  Q_D(qSlicerVolumeReconstructionModuleWidget);
  if (!d->VolumeReconstructionNode)
  {
    return;
  }
  d->logic()->GetReconstructedVolume(d->VolumeReconstructionNode);
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
