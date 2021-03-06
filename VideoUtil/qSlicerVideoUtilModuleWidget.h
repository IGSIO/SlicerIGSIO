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

#ifndef __qSlicerVideoUtilModuleWidget_h
#define __qSlicerVideoUtilModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVideoUtilModuleExport.h"

#include <QObject>
#include <QtGui>

class qSlicerVideoUtilModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_VideoUtil
class Q_SLICER_QTMODULES_VIDEOUTIL_EXPORT qSlicerVideoUtilModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVideoUtilModuleWidget(QWidget* parent = 0);
  virtual ~qSlicerVideoUtilModuleWidget();

public slots:

  void onCodecChanged(const QString& fourCC);
  void encodeVideo();

protected:
  QScopedPointer<qSlicerVideoUtilModuleWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void setMRMLScene(vtkMRMLScene*);

  static void updateProgress(vtkObject* caller, unsigned long event, void* clientData, void* callData);

private:
  Q_DECLARE_PRIVATE(qSlicerVideoUtilModuleWidget);
  Q_DISABLE_COPY(qSlicerVideoUtilModuleWidget);

};

#endif
