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

#ifndef __qSlicerSequenceIOModule_h
#define __qSlicerSequenceIOModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"
#include "qSlicerSequenceIOModuleExport.h"
#include "qSlicerApplication.h"

class qSlicerSequenceIOModulePrivate;
class vtkObject;

/// \ingroup Slicer_QtModules_SequenceIO
class Q_SLICER_QTMODULES_SEQUENCEIO_EXPORT qSlicerSequenceIOModule :
  public qSlicerLoadableModule
{
  Q_OBJECT;
  QVTK_OBJECT;
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerSequenceIOModule(QObject* parent = 0);
  virtual ~qSlicerSequenceIOModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Help to use the module
  virtual QString helpText() const override;

  /// Return acknowledgements
  virtual QString acknowledgementText() const override;

  /// Return a custom icon for the module
  virtual QIcon icon() const override;

  /// Module category
  virtual QStringList categories() const override;

  /// Dependencies on other Slicer modules
  virtual QStringList dependencies() const override;

  /// Return the authors of the module
  virtual QStringList contributors() const override;

  /// Make this module hidden
  bool isHidden() const override { return true; };

protected:

  /// Initialize the module. Register the volumes reader/writer
  virtual void setup() override;

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic() override;

public slots:
  virtual void setMRMLScene(vtkMRMLScene*) override;

protected:
  QScopedPointer<qSlicerSequenceIOModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSequenceIOModule);
  Q_DISABLE_COPY(qSlicerSequenceIOModule);

};

#endif
