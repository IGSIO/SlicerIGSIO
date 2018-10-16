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

// Qt includes
#include <QDebug>
#include <QStandardItemModel>
#include <QTreeView>

// SlicerQt includes
#include "qSlicerOpenIGTLinkIFModuleWidget.h"
#include "ui_qSlicerOpenIGTLinkIFModule.h"

// qMRMLWidgets includes
#include <qMRMLNodeFactory.h>

// OpenIGTLinkIF Logic includes
#include "vtkSlicerOpenIGTLinkIFLogic.h"

// OpenIGTLinkIF MRML includes
#include "vtkMRMLIGTLConnectorNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_OpenIGTLinkIF
class qSlicerOpenIGTLinkIFModuleWidgetPrivate: public Ui_qSlicerOpenIGTLinkIFModule
{
  Q_DECLARE_PUBLIC(qSlicerOpenIGTLinkIFModuleWidget);
protected:
  qSlicerOpenIGTLinkIFModuleWidget* const q_ptr;
public:
  qSlicerOpenIGTLinkIFModuleWidgetPrivate(qSlicerOpenIGTLinkIFModuleWidget& object);

  vtkSlicerOpenIGTLinkIFLogic * logic();

};

//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkIFModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkIFModuleWidgetPrivate::qSlicerOpenIGTLinkIFModuleWidgetPrivate(qSlicerOpenIGTLinkIFModuleWidget& object)
 : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
vtkSlicerOpenIGTLinkIFLogic * qSlicerOpenIGTLinkIFModuleWidgetPrivate::logic()
{
  Q_Q(qSlicerOpenIGTLinkIFModuleWidget);
  return vtkSlicerOpenIGTLinkIFLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerOpenIGTLinkIFModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkIFModuleWidget::qSlicerOpenIGTLinkIFModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerOpenIGTLinkIFModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerOpenIGTLinkIFModuleWidget::~qSlicerOpenIGTLinkIFModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkIFModuleWidget::setup()
{
  Q_D(qSlicerOpenIGTLinkIFModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  // --------------------------------------------------
  // Connectors section
  //  Connector List View
  connect(d->ConnectorListView, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          d->ConnectorPropertyWidget, SLOT(setMRMLIGTLConnectorNode(vtkMRMLNode*)));
  d->ConnectorPropertyWidget->setMRMLIGTLConnectorNode(static_cast<vtkMRMLNode*>(0));

  //  Add(+) / Remove(-) Connector Buttons
  connect(d->AddConnectorButton, SIGNAL(clicked()), this,
          SLOT(onAddConnectorButtonClicked()));
  connect(d->RemoveConnectorButton, SIGNAL(clicked()), this,
          SLOT(onRemoveConnectorButtonClicked()));

  // --------------------------------------------------
  //  I/O Configuration Section
  connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
          d->IGTIONodeSelectorWidget, SLOT(setMRMLScene(vtkMRMLScene*)));
  connect(d->IOTreeView, SIGNAL(ioTreeViewUpdated(int,vtkMRMLIGTLConnectorNode*,int, vtkMRMLNode*)),
          d->IGTIONodeSelectorWidget, SLOT(updateEnabledStatus(int,vtkMRMLIGTLConnectorNode*,int, vtkMRMLNode*)));

}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkIFModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerOpenIGTLinkIFModuleWidget);

  this->Superclass::setMRMLScene(scene);
  if (scene == NULL)
    {
    return;
    }
  d->ConnectorListView->setMRMLScene(scene);
  d->IOTreeView->setMRMLScene(scene);

}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkIFModuleWidget::onAddConnectorButtonClicked()
{
  Q_D(qSlicerOpenIGTLinkIFModuleWidget);

  if (this->mrmlScene())
    {
    vtkMRMLIGTLConnectorNode* node = 
      vtkMRMLIGTLConnectorNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass("vtkMRMLIGTLConnectorNode"));
    if (node)
      {
      this->mrmlScene()->AddNode(node);
      d->ConnectorListView->setSelectedNode(node->GetID());
      node->SetType(vtkMRMLIGTLConnectorNode::TypeClient);
      node->Delete();
      }
    }
  //qMRMLNodeFactory::createNode(this->mrmlScene(), "vtkMRMLIGTLConnectorNode");
}

//-----------------------------------------------------------------------------
void qSlicerOpenIGTLinkIFModuleWidget::onRemoveConnectorButtonClicked()
{
  Q_D(qSlicerOpenIGTLinkIFModuleWidget);
  vtkMRMLNode * node = d->ConnectorListView->currentNode();
  if (!node)
    {
    return;
    }
  vtkMRMLIGTLConnectorNode * connectorNode = vtkMRMLIGTLConnectorNode::SafeDownCast(node);
  Q_ASSERT(connectorNode);
  connectorNode->Stop();
  this->mrmlScene()->RemoveNode(connectorNode);
}


