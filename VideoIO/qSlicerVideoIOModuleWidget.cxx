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
#include "qSlicerVideoIOModuleWidget.h"
#include "ui_qSlicerVideoIOModule.h"

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
  PARAMETER_VALUE_COLUMN=0,
  PARAMETER_DESCRIPTION_COLUMN,
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VideoIO
class qSlicerVideoIOModuleWidgetPrivate: public Ui_qSlicerVideoIOModule
{
  Q_DECLARE_PUBLIC(qSlicerVideoIOModuleWidget);
protected:
  qSlicerVideoIOModuleWidget* const q_ptr;
public:
  qSlicerVideoIOModuleWidgetPrivate(qSlicerVideoIOModuleWidget& object);
  ~qSlicerVideoIOModuleWidgetPrivate();

};

//-----------------------------------------------------------------------------
// qSlicerVideoIOModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVideoIOModuleWidgetPrivate::qSlicerVideoIOModuleWidgetPrivate(qSlicerVideoIOModuleWidget& object)
 : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerVideoIOModuleWidgetPrivate::~qSlicerVideoIOModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVideoIOModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVideoIOModuleWidget::qSlicerVideoIOModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerVideoIOModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerVideoIOModuleWidget::~qSlicerVideoIOModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVideoIOModuleWidget::setup()
{
  Q_D(qSlicerVideoIOModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect(d->EncodeButton, SIGNAL(clicked()), this, SLOT(encodeVideo()));
  connect(d->CodecSelector, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(onCodecChanged(QString)));

  std::vector<std::string> codecFourCCs = vtkStreamingVolumeCodecFactory::GetInstance()->GetStreamingCodecFourCCs();
  QStringList codecs;
  for (std::vector<std::string>::iterator codecIt = codecFourCCs.begin(); codecIt != codecFourCCs.end(); ++codecIt)
  {
    codecs << QString::fromStdString(*codecIt);
  }
  d->CodecSelector->addItems(codecs);
}

//-----------------------------------------------------------------------------
void qSlicerVideoIOModuleWidget::onCodecChanged(const QString &codecFourCC)
{
  Q_D(qSlicerVideoIOModuleWidget);
  d->EncodingParameterTable->setRowCount(0);

  vtkSmartPointer<vtkStreamingVolumeCodec> codec = vtkSmartPointer<vtkStreamingVolumeCodec> ::Take(
    vtkStreamingVolumeCodecFactory::GetInstance()->CreateCodecByFourCC(codecFourCC.toStdString()));
  if (!codec)
  {
    return;
  }

  std::vector<std::string> parameterNames = codec->GetAvailiableParameterNames();
  QStringList parameters;
  for (std::vector<std::string>::iterator parameterIt = parameterNames.begin(); parameterIt != parameterNames.end(); ++parameterIt)
  {
    parameters << QString::fromStdString(*parameterIt);
  }

  d->EncodingParameterTable->setRowCount(parameterNames.size());
  d->EncodingParameterTable->setVerticalHeaderLabels(parameters);

  int currentRow = 0;
  for (std::vector<std::string>::iterator parameterIt = parameterNames.begin(); parameterIt != parameterNames.end(); ++parameterIt)
  {
    std::string value;
    codec->GetParameter(*parameterIt, value);
    QTextEdit* textEdit = new QTextEdit(d->EncodingParameterTable);
    textEdit->setText(QString::fromStdString(value));
    d->EncodingParameterTable->setCellWidget(currentRow, PARAMETER_VALUE_COLUMN, textEdit);

    QString description = QString::fromStdString(codec->GetParameterDescription(*parameterIt));
    QLabel* descriptionLabel = new QLabel(d->EncodingParameterTable);
    descriptionLabel->setText(description);
    d->EncodingParameterTable->setCellWidget(currentRow, PARAMETER_DESCRIPTION_COLUMN, descriptionLabel);
    ++currentRow;
  }
}

//-----------------------------------------------------------------------------
void qSlicerVideoIOModuleWidget::encodeVideo()
{
  Q_D(qSlicerVideoIOModuleWidget);
  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(d->VideoNodeSelector->currentNode());
  if (!sequenceNode)
  {
    return;
  }

  std::map<std::string, std::string> parameters;
  for (int i = 0; i < d->EncodingParameterTable->rowCount(); ++i)
  {
    std::string parameterName = d->EncodingParameterTable->verticalHeaderItem(i)->text().toStdString();

    QTextEdit* valueTextEdit = qobject_cast<QTextEdit*>(d->EncodingParameterTable->cellWidget(i, PARAMETER_VALUE_COLUMN));
    std::string parameterValue = valueTextEdit->toPlainText().toStdString();
    if (parameterValue == "")
    {
      continue;
    }
    parameters[parameterName] = parameterValue;
  }

  std::string encoding = d->CodecSelector->currentText().toStdString();
  vtkSlicerIGSIOCommon::ReEncodeVideoSequence(sequenceNode, 0, -1, encoding, parameters, true);

}

//-----------------------------------------------------------------------------
void qSlicerVideoIOModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerVideoIOModuleWidget);

  this->Superclass::setMRMLScene(scene);
  if (scene == NULL)
    {
    return;
    }
}
