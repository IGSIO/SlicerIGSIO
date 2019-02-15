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

#include "qSlicerCoreApplication.h"

// VideoIO module includes
#include "qSlicerVideoIOModule.h"
#include "qSlicerVideoIOModuleWidget.h"
#include "qSlicerVideoReader.h"

// VideoIO Logic includes
#include "vtkSlicerVideoIOLogic.h"

// Slicer includes
#include <qSlicerCoreApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerNodeWriter.h>

// IGSIO codec includes
#include <vtkVP9VolumeCodec.h>

// vtkSlicerIGSIOCommon includes
#include <vtkSlicerIGSIOLogger.h>

// vtkAddon include
#include <vtkStreamingVolumeCodecFactory.h>

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
  return "This is a module for reading, writing, and re-encoding video sequences. If you have questions, or encounter an problem, submit an issue on the <a href=\"https://github.com/IGSIO/SlicerIGSIO\">GitHub page</a>.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVideoIOModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Kyle Sunderland (PerkLab, Queen's University)");
  moduleContributors << QString("Andras Lasso (PerkLab, Queen's University)");
  return moduleContributors;
}


//-----------------------------------------------------------------------------
QString qSlicerVideoIOModule::acknowledgementText()const
{
  return "This module was developed through support from CANARIE's Research Software Program, and Cancer Care Ontario.";
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

  // Create logger instance
  vtkSlicerIGSIOLogger::Instance();

  // Register the IO
  vtkSlicerVideoIOLogic* logic = vtkSlicerVideoIOLogic::SafeDownCast(this->logic());
  app->coreIOManager()->registerIO(new qSlicerVideoReader(logic, this));

  // Register the codecs
  vtkStreamingVolumeCodecFactory* codecFactory = vtkStreamingVolumeCodecFactory::GetInstance();
  codecFactory->RegisterStreamingCodec(vtkSmartPointer<vtkVP9VolumeCodec>::New());
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
  return new qSlicerVideoIOModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVideoIOModule::createLogic()
{
  return vtkSlicerVideoIOLogic::New();
}
