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

// VideoUtil module includes
#include "qSlicerVideoUtilModule.h"
#include "qSlicerVideoUtilModuleWidget.h"
#include "qSlicerVideoUtilReader.h"

// VideoUtil Logic includes
#include "vtkSlicerVideoUtilLogic.h"

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
Q_EXPORT_PLUGIN2(qSlicerVideoUtilModule, qSlicerVideoUtilModule);
#endif
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VideoUtil
class qSlicerVideoUtilModulePrivate
{
public:
  qSlicerVideoUtilModulePrivate();

};

//-----------------------------------------------------------------------------
// qSlicerVideoUtilModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerVideoUtilModulePrivate::qSlicerVideoUtilModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVideoUtilModule methods

//-----------------------------------------------------------------------------
qSlicerVideoUtilModule::qSlicerVideoUtilModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVideoUtilModulePrivate)
{
  Q_D(qSlicerVideoUtilModule);
}

//-----------------------------------------------------------------------------
qSlicerVideoUtilModule::~qSlicerVideoUtilModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVideoUtilModule::helpText()const
{
  return "This is a module for reading, writing, and re-encoding video sequences. If you have questions, or encounter an problem, submit an issue on the <a href=\"https://github.com/IGSIO/SlicerIGSIO\">GitHub page</a>.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVideoUtilModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Kyle Sunderland (PerkLab, Queen's University)");
  moduleContributors << QString("Andras Lasso (PerkLab, Queen's University)");
  return moduleContributors;
}


//-----------------------------------------------------------------------------
QString qSlicerVideoUtilModule::acknowledgementText()const
{
  return "This module was developed through support from CANARIE's Research Software Program, and Cancer Care Ontario.";
}

//-----------------------------------------------------------------------------
QIcon qSlicerVideoUtilModule::icon()const
{
  return QIcon(":/Icons/VideoUtil.png");
}


//-----------------------------------------------------------------------------
QStringList qSlicerVideoUtilModule::categories() const
{
  return QStringList() << "IO";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVideoUtilModule::dependencies() const
{
  return QStringList() << "Sequences" << "SequenceBrowser";
}

//-----------------------------------------------------------------------------
void qSlicerVideoUtilModule::setup()
{
  this->Superclass::setup();

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();

  // Create logger instance
  vtkSlicerIGSIOLogger::Instance();

  // Register the IO
  vtkSlicerVideoUtilLogic* logic = vtkSlicerVideoUtilLogic::SafeDownCast(this->logic());
  app->coreIOManager()->registerIO(new qSlicerVideoUtilReader(logic, this));

  // Register the codecs
  vtkStreamingVolumeCodecFactory* codecFactory = vtkStreamingVolumeCodecFactory::GetInstance();
  codecFactory->RegisterStreamingCodec(vtkSmartPointer<vtkVP9VolumeCodec>::New());
}

//-----------------------------------------------------------------------------
void qSlicerVideoUtilModule::setMRMLScene(vtkMRMLScene* scene)
{
  vtkMRMLScene* oldScene = this->mrmlScene();
  this->Superclass::setMRMLScene(scene);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerVideoUtilModule::createWidgetRepresentation()
{
  return new qSlicerVideoUtilModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVideoUtilModule::createLogic()
{
  return vtkSlicerVideoUtilLogic::New();
}
