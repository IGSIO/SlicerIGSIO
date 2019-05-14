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

#ifndef __qSlicerVolumeReconstructionModuleWidget_h
#define __qSlicerVolumeReconstructionModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVolumeReconstructionModuleExport.h"

#include <QObject>
#include <QtGui>

class qSlicerVolumeReconstructionModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_VolumeReconstruction
class Q_SLICER_QTMODULES_VOLUMERECONSTRUCTION_EXPORT qSlicerVolumeReconstructionModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumeReconstructionModuleWidget(QWidget* parent = 0);
  virtual ~qSlicerVolumeReconstructionModuleWidget();

public slots:
  void onToggleROIVisible();
  void onApply();

protected:
  QScopedPointer<qSlicerVolumeReconstructionModuleWidgetPrivate> d_ptr;

  virtual void setup();

protected slots:
  virtual void updateWidgetFromMRML();
  virtual void setMRMLScene(vtkMRMLScene*);

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeReconstructionModuleWidget);
  Q_DISABLE_COPY(qSlicerVolumeReconstructionModuleWidget);

};

#endif
