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

// SequenceIO module includes
#include "qSlicerSequenceIOModule.h"
#include "qSlicerSequenceIOReader.h"

// SequenceIO Logic includes
#include "vtkSlicerSequenceIOLogic.h"

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
  Q_EXPORT_PLUGIN2(qSlicerSequenceIOModule, qSlicerSequenceIOModule);
#endif
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SequenceIO
class qSlicerSequenceIOModulePrivate
{
public:
  qSlicerSequenceIOModulePrivate();

};

//-----------------------------------------------------------------------------
// qSlicerSequenceIOModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSequenceIOModulePrivate::qSlicerSequenceIOModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSequenceIOModule methods

//-----------------------------------------------------------------------------
qSlicerSequenceIOModule::qSlicerSequenceIOModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSequenceIOModulePrivate)
{
  Q_D(qSlicerSequenceIOModule);
}

//-----------------------------------------------------------------------------
qSlicerSequenceIOModule::~qSlicerSequenceIOModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSequenceIOModule::helpText()const
{
  return "This is a module for reading, writing, and re-encoding video sequences. If you have questions, or encounter an problem, submit an issue on the <a href=\"https://github.com/IGSIO/SlicerIGSIO\">GitHub page</a>.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSequenceIOModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Kyle Sunderland (PerkLab, Queen's University)");
  moduleContributors << QString("Andras Lasso (PerkLab, Queen's University)");
  return moduleContributors;
}


//-----------------------------------------------------------------------------
QString qSlicerSequenceIOModule::acknowledgementText()const
{
  return "This module was developed through support from CANARIE's Research Software Program, and Cancer Care Ontario.";
}

//-----------------------------------------------------------------------------
QIcon qSlicerSequenceIOModule::icon()const
{
  return QIcon(":/Icons/SequenceIO.png");
}


//-----------------------------------------------------------------------------
QStringList qSlicerSequenceIOModule::categories() const
{
  return QStringList() << "IO";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSequenceIOModule::dependencies() const
{
  return QStringList() << "Sequences" << "SequenceBrowser";
}

//-----------------------------------------------------------------------------
void qSlicerSequenceIOModule::setup()
{
  this->Superclass::setup();

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();

  // Create logger instance
  vtkSlicerIGSIOLogger::Instance();

  // Register the IO
  vtkSlicerSequenceIOLogic* logic = vtkSlicerSequenceIOLogic::SafeDownCast(this->logic());
  app->coreIOManager()->registerIO(new qSlicerSequenceIOReader(logic, this));
}

//-----------------------------------------------------------------------------
void qSlicerSequenceIOModule::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerSequenceIOModule::createWidgetRepresentation()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSequenceIOModule::createLogic()
{
  return vtkSlicerSequenceIOLogic::New();
}
