/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// CTK includes
#include <ctkFlowLayout.h>
#include <ctkUtils.h>

// VTK includes
#include <vtkNew.h>

/// Volumes includes
#include "qSlicerIOOptions_p.h"
#include "qSlicerMetafileIOOptionsWidget.h"
#include "ui_qSlicerMetafileIOOptionsWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Metafiles
class qSlicerMetafileIOOptionsWidgetPrivate
  : public qSlicerIOOptionsPrivate
  , public Ui_qSlicerMetafileIOOptionsWidget
{
public:
};

//-----------------------------------------------------------------------------
qSlicerMetafileIOOptionsWidget::qSlicerMetafileIOOptionsWidget(QWidget* parentWidget)
  : qSlicerIOOptionsWidget(new qSlicerMetafileIOOptionsWidgetPrivate, parentWidget)
{
  Q_D(qSlicerMetafileIOOptionsWidget);
  d->setupUi(this);

  ctkFlowLayout::replaceLayout(this);

  connect(d->OutputBrowserNodeIDEdit, SIGNAL(textChanged(QString)),
          this, SLOT(updateProperties()));
  connect(d->SaveSequenceChangesCheckbox, SIGNAL(stateChanged(int)),
          this, SLOT(updateProperties()));
  updateProperties();  // ensure that the default gui values are set as properties
}

//-----------------------------------------------------------------------------
qSlicerMetafileIOOptionsWidget::~qSlicerMetafileIOOptionsWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerMetafileIOOptionsWidget::updateProperties()
{
  Q_D(qSlicerMetafileIOOptionsWidget);
  if (!d->OutputBrowserNodeIDEdit->text().isEmpty())
    {
    d->Properties["outputBrowserNodeID"] = d->OutputBrowserNodeIDEdit->text();
    }
  else
    {
    d->Properties.remove("outputBrowserNodeID");
    }
  d->Properties["saveSequenceChanges"] = d->SaveSequenceChangesCheckbox->isChecked();
}
