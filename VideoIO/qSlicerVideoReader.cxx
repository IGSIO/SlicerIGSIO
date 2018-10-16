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

// SlicerIGSIO include
#include <vtkSlicerVideoIOLogic.h>

#include <qSlicerVideoIOModule.h>

// IGSIO includes
#include <vtkTrackedFrameList.h>

// SlicerQt includes
#include "qSlicerVideoReader.h"

#include "vtkSlicerIGSIOCommon.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLSelectionNode.h>

// Sequence MRML includes
#include "vtkMRMLSequenceNode.h"

// Sequence browser MRML includes
#include "vtkMRMLSequenceBrowserNode.h"

//
#include "vtkMRMLStreamingVolumeNode.h"

#include "vtkMRMLStreamingVolumeSequenceStorageNode.h"

// VTK includes
#include <vtkSmartPointer.h>


//-----------------------------------------------------------------------------
class qSlicerVideoReaderPrivate
{
public:
  vtkSlicerVideoIOLogic* VideoIOLogic;
};

//-----------------------------------------------------------------------------
qSlicerVideoReader::qSlicerVideoReader(vtkSlicerVideoIOLogic* newVideoIOLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVideoReaderPrivate)
{
  this->setVideoIOLogic(newVideoIOLogic);
}

//-----------------------------------------------------------------------------
qSlicerVideoReader::~qSlicerVideoReader()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVideoReader::description() const
{
  return "Video Container";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerVideoReader::fileType() const
{
  return "Video Container";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVideoReader::extensions() const
{
  return QStringList() << "Matroska Video (*.mkv)" << "WebM (*.webm)";
}

//-----------------------------------------------------------------------------
void qSlicerVideoReader::setVideoIOLogic(vtkSlicerVideoIOLogic* newVideoIOLogic)
{
  Q_D(qSlicerVideoReader);
  d->VideoIOLogic = newVideoIOLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerVideoIOLogic* qSlicerVideoReader::VideoIOLogic() const
{
  Q_D(const qSlicerVideoReader);
  return d->VideoIOLogic;
}

//-----------------------------------------------------------------------------
bool qSlicerVideoReader::load(const IOProperties& properties)
{
  Q_D(qSlicerVideoReader);
  if (!properties.contains("fileName"))
  {
    qCritical() << Q_FUNC_INFO << " did not receive fileName property";
  }
  QString fileName = properties["fileName"].toString();

  vtkSmartPointer<vtkTrackedFrameList> trackedFrameList = vtkSmartPointer<vtkTrackedFrameList>::New();
  if (!vtkMRMLStreamingVolumeSequenceStorageNode::ReadVideo(fileName.toStdString(), trackedFrameList))
  {
    qCritical() << Q_FUNC_INFO << " error reading video: " << fileName;
    return false;
  }

  std::string sequenceBrowserName = vtksys::SystemTools::GetFilenameWithoutExtension(fileName.toStdString());
  vtkSmartPointer<vtkMRMLSequenceBrowserNode> sequenceBrowserNode = vtkSmartPointer<vtkMRMLSequenceBrowserNode>::New();
  sequenceBrowserNode->SetName(this->mrmlScene()->GetUniqueNameByString(sequenceBrowserName.c_str()));
  this->mrmlScene()->AddNode(sequenceBrowserNode);

  if (!vtkSlicerIGSIOCommon::TrackedFrameListToSequenceBrowser(trackedFrameList, sequenceBrowserNode))
  {
    this->mrmlScene()->RemoveNode(sequenceBrowserNode);
    qCritical() << Q_FUNC_INFO << " could not convert tracked frame list to sequence browser node";
    return false;
  }

  std::vector<vtkMRMLSequenceNode*> sequenceNodes;
  sequenceBrowserNode->GetSynchronizedSequenceNodes(sequenceNodes, true);
  for (std::vector<vtkMRMLSequenceNode*>::iterator sequenceNodeIt = sequenceNodes.begin(); sequenceNodeIt != sequenceNodes.end(); ++sequenceNodeIt)
  {
    vtkMRMLSequenceNode* sequenceNode = *sequenceNodeIt;
    if (!sequenceNode || sequenceNode->GetNumberOfDataNodes() < 1)
    {
      continue;
    }

    vtkMRMLStreamingVolumeNode* streamingVolumeNode = vtkMRMLStreamingVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(0));
    if (!streamingVolumeNode)
    {
      continue;
    }

    vtkSmartPointer<vtkMRMLStreamingVolumeSequenceStorageNode> storageNode = vtkSmartPointer<vtkMRMLStreamingVolumeSequenceStorageNode>::New();
    this->mrmlScene()->AddNode(storageNode.GetPointer());
    sequenceNode->SetAndObserveStorageNodeID(storageNode->GetID());
  }

  vtkSlicerApplicationLogic* appLogic = this->VideoIOLogic()->GetApplicationLogic();
  vtkMRMLSelectionNode* selectionNode = appLogic ? appLogic->GetSelectionNode() : 0;
  if (appLogic && selectionNode)
  {
    vtkMRMLVolumeNode* imageProxyVolumeNode = vtkMRMLVolumeNode::SafeDownCast(sequenceBrowserNode->GetProxyNode(sequenceBrowserNode->GetMasterSequenceNode()));
    if (imageProxyVolumeNode)
    {
      selectionNode->SetReferenceActiveVolumeID(imageProxyVolumeNode->GetID());
      appLogic->PropagateVolumeSelection();
      appLogic->FitSliceToAll();
    }
  }

  return true;
}
