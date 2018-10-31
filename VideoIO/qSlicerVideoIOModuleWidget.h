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

#ifndef __qSlicerVideoIOModuleWidget_h
#define __qSlicerVideoIOModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerVideoIOModuleExport.h"

#include <QObject>
#include <QtGui>

class qSlicerVideoIOModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_VideoIO
class Q_SLICER_QTMODULES_VIDEOIO_EXPORT qSlicerVideoIOModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVideoIOModuleWidget(QWidget *parent=0);
  virtual ~qSlicerVideoIOModuleWidget();

public slots:

  void onCodecChanged(const QString& fourCC);
  void encodeVideo();

protected:
  QScopedPointer<qSlicerVideoIOModuleWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void setMRMLScene(vtkMRMLScene*);

private:
  Q_DECLARE_PRIVATE(qSlicerVideoIOModuleWidget);
  Q_DISABLE_COPY(qSlicerVideoIOModuleWidget);

};

#endif
