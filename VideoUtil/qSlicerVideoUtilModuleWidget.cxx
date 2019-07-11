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
#include <QStandardItemModel>
#include <QTreeView>
#include <QTextEdit>

// SlicerQt includes
#include "qSlicerVideoUtilModuleWidget.h"
#include "ui_qSlicerVideoUtilModule.h"

// vtkAddon includes
#include <vtkStreamingVolumeCodecFactory.h>

// SequenceMRML includes
#include <vtkMRMLSequenceNode.h>

// SlicerIGSIOCommon includes
#include "vtkSlicerIGSIOCommon.h"

// qMRMLWidgets includes
#include <qMRMLNodeFactory.h>

enum
{
  PARAMETER_NAME_COLUMN = 0,
  PARAMETER_VALUE_COLUMN,
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VideoUtil
class qSlicerVideoUtilModuleWidgetPrivate: public Ui_qSlicerVideoUtilModule
{
  Q_DECLARE_PUBLIC(qSlicerVideoUtilModuleWidget);
protected:
  qSlicerVideoUtilModuleWidget* const q_ptr;
public:
  qSlicerVideoUtilModuleWidgetPrivate(qSlicerVideoUtilModuleWidget& object);
  ~qSlicerVideoUtilModuleWidgetPrivate();

};

//-----------------------------------------------------------------------------
// qSlicerVideoUtilModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVideoUtilModuleWidgetPrivate::qSlicerVideoUtilModuleWidgetPrivate(qSlicerVideoUtilModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerVideoUtilModuleWidgetPrivate::~qSlicerVideoUtilModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVideoUtilModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVideoUtilModuleWidget::qSlicerVideoUtilModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVideoUtilModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerVideoUtilModuleWidget::~qSlicerVideoUtilModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVideoUtilModuleWidget::setup()
{
  Q_D(qSlicerVideoUtilModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect(d->encodeButton, SIGNAL(clicked()), this, SLOT(encodeVideo()));
  connect(d->codecSelector, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onCodecChanged(QString)));

  std::vector<std::string> codecFourCCs = vtkStreamingVolumeCodecFactory::GetInstance()->GetStreamingCodecFourCCs();
  QStringList codecs;
  for (std::vector<std::string>::iterator codecIt = codecFourCCs.begin(); codecIt != codecFourCCs.end(); ++codecIt)
  {
    codecs << QString::fromStdString(*codecIt);
  }
  d->codecSelector->addItems(codecs);
}

//-----------------------------------------------------------------------------
void qSlicerVideoUtilModuleWidget::onCodecChanged(const QString& codecFourCC)
{
  Q_D(qSlicerVideoUtilModuleWidget);
  d->encodingParameterTable->setRowCount(0);

  vtkSmartPointer<vtkStreamingVolumeCodec> codec = vtkSmartPointer<vtkStreamingVolumeCodec> ::Take(
        vtkStreamingVolumeCodecFactory::GetInstance()->CreateCodecByFourCC(codecFourCC.toStdString()));
  if (!codec)
  {
    return;
  }

  std::vector<std::string> parameterNames = codec->GetAvailiableParameterNames();
  d->encodingParameterTable->setRowCount(parameterNames.size());

  int currentRow = 0;
  for (std::vector<std::string>::iterator parameterIt = parameterNames.begin(); parameterIt != parameterNames.end(); ++parameterIt)
  {
    std::string parameterName = parameterNames[currentRow];
    QString description = QString::fromStdString(codec->GetParameterDescription(*parameterIt));

    QLabel* nameLabel = new QLabel(parameterName.c_str(), d->encodingParameterTable);
    nameLabel->setToolTip(description);
    d->encodingParameterTable->setCellWidget(currentRow, PARAMETER_NAME_COLUMN, nameLabel);

    std::string value;
    codec->GetParameter(*parameterIt, value);
    QTextEdit* textEdit = new QTextEdit(d->encodingParameterTable);
    textEdit->setText(QString::fromStdString(value));
    textEdit->setToolTip(description);
    d->encodingParameterTable->setCellWidget(currentRow, PARAMETER_VALUE_COLUMN, textEdit);

    ++currentRow;
  }
}

//-----------------------------------------------------------------------------
void qSlicerVideoUtilModuleWidget::encodeVideo()
{
  Q_D(qSlicerVideoUtilModuleWidget);
  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(d->videoNodeSelector->currentNode());
  if (!sequenceNode)
  {
    return;
  }

  std::map<std::string, std::string> parameters;
  for (int i = 0; i < d->encodingParameterTable->rowCount(); ++i)
  {
    std::string parameterName = d->encodingParameterTable->verticalHeaderItem(i)->text().toStdString();

    QTextEdit* valueTextEdit = qobject_cast<QTextEdit*>(d->encodingParameterTable->cellWidget(i, PARAMETER_VALUE_COLUMN));
    std::string parameterValue = valueTextEdit->toPlainText().toStdString();
    if (parameterValue == "")
    {
      continue;
    }
    parameters[parameterName] = parameterValue;
  }

  std::string encoding = d->codecSelector->currentText().toStdString();
  vtkSlicerIGSIOCommon::ReEncodeVideoSequence(sequenceNode, 0, -1, encoding, parameters, true);

}

//-----------------------------------------------------------------------------
void qSlicerVideoUtilModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerVideoUtilModuleWidget);

  this->Superclass::setMRMLScene(scene);
  if (scene == NULL)
  {
    return;
  }
}
