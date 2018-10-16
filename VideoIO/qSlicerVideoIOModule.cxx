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
and was supported through CANARIE’s Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

#include "qSlicerCoreApplication.h"

// VideoIO module includes
#include "qSlicerVideoIOModule.h"
#include "qSlicerVideoReader.h"

// VideoIO Logic includes
#include <vtkSlicerVideoIOLogic.h>

#include <qSlicerCoreApplication.h>
#include <qSlicerCoreIOManager.h>

#include <qSlicerNodeWriter.h>

//-----------------------------------------------------------------------------
#include <QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerVideoIOModule, qSlicerVideoIOModule);
#endif
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VideoIO
class qSlicerVideoIOModulePrivate
{
public:
  qSlicerVideoIOModulePrivate();

};

//-----------------------------------------------------------------------------
// qSlicerVideoIOModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerVideoIOModulePrivate::qSlicerVideoIOModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVideoIOModule methods

//-----------------------------------------------------------------------------
qSlicerVideoIOModule::qSlicerVideoIOModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVideoIOModulePrivate)
{
  Q_D(qSlicerVideoIOModule);
}

//-----------------------------------------------------------------------------
qSlicerVideoIOModule::~qSlicerVideoIOModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVideoIOModule::helpText()const
{
  return "";
}

//-----------------------------------------------------------------------------
QString qSlicerVideoIOModule::acknowledgementText()const
{
  return "This module was developed by Kyle Sunderland (Perk Lab, Queen's University), "
         "";
}

//-----------------------------------------------------------------------------
QIcon qSlicerVideoIOModule::icon()const
{
  return QIcon(":/Icons/VideoIO.png");
}


//-----------------------------------------------------------------------------
QStringList qSlicerVideoIOModule::categories() const
{
  return QStringList() << "IO";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVideoIOModule::dependencies() const
{
  return QStringList() << "Sequences" << "SequenceBrowser";
}

//-----------------------------------------------------------------------------
void qSlicerVideoIOModule::setup()
{
  this->Superclass::setup();

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();

  // Register the IO
 
  vtkSlicerVideoIOLogic* logic = vtkSlicerVideoIOLogic::SafeDownCast(this->logic());
  app->coreIOManager()->registerIO(new qSlicerVideoReader(logic, this));
  //app->coreIOManager()->registerIO(new qSlicerNodeWriter("VideoIO", QString("Video Container"), QStringList() << "vtkMRMLSequenceBrowserNode", true, this));
}

//-----------------------------------------------------------------------------
void qSlicerVideoIOModule::setMRMLScene(vtkMRMLScene* scene)
{
  vtkMRMLScene* oldScene = this->mrmlScene();
  this->Superclass::setMRMLScene(scene);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerVideoIOModule::createWidgetRepresentation()
{
  //return new qSlicerVideoIOModuleWidget;
  return NULL;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVideoIOModule::createLogic()
{
  return vtkSlicerVideoIOLogic::New();
}
