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

// SlicerIGSIO include
#include "vtkSlicerSequenceIOLogic.h"
#include "vtkSlicerIGSIOCommon.h"
#include "qSlicerSequenceIOModule.h"

// IGSIO includes
#include <vtkIGSIOSequenceIO.h>
#include <vtkIGSIOTrackedFrameList.h>

// SlicerQt includes
#include "qSlicerSequenceIOReader.h"

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
class qSlicerSequenceIOReaderPrivate
{
public:
  vtkSlicerSequenceIOLogic* SequenceIOLogic;
};

//-----------------------------------------------------------------------------
qSlicerSequenceIOReader::qSlicerSequenceIOReader(vtkSlicerSequenceIOLogic* newSequenceIOLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSequenceIOReaderPrivate)
{
  this->setSequenceIOLogic(newSequenceIOLogic);
}

//-----------------------------------------------------------------------------
qSlicerSequenceIOReader::~qSlicerSequenceIOReader()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSequenceIOReader::description() const
{
  return "IGSIO Sequence";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSequenceIOReader::fileType() const
{
  return "IGSIO Sequence";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSequenceIOReader::extensions() const
{
  QStringList supportedExtensions = QStringList();
#ifdef IGSIO_SEQUENCEIO_ENABLE_MKV
  supportedExtensions << "IGSIO sequence Matroska (*.mkv)";
  supportedExtensions << "IGSIO sequence WebM (*.webm)";
#endif
  supportedExtensions << "IGSIO sequence metafile (*.igs.mha)";
  supportedExtensions << "IGSIO sequence metafile (*.igs.mhd)";
  supportedExtensions << "IGSIO sequence nrrd (*.igs.nrrd)";
  return supportedExtensions;
}

//-----------------------------------------------------------------------------
void qSlicerSequenceIOReader::setSequenceIOLogic(vtkSlicerSequenceIOLogic* newSequenceIOLogic)
{
  Q_D(qSlicerSequenceIOReader);
  d->SequenceIOLogic = newSequenceIOLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerSequenceIOLogic* qSlicerSequenceIOReader::SequenceIOLogic() const
{
  Q_D(const qSlicerSequenceIOReader);
  return d->SequenceIOLogic;
}

//-----------------------------------------------------------------------------
bool qSlicerSequenceIOReader::load(const IOProperties& properties)
{
  Q_D(qSlicerSequenceIOReader);
  if (!properties.contains("fileName"))
  {
    qCritical() << Q_FUNC_INFO << " did not receive fileName property";
  }
  QString fileName = properties["fileName"].toString();

  std::string fileNameNoExtension = vtksys::SystemTools::GetFilenameWithoutExtension(fileName.toStdString());

  vtkSmartPointer<vtkIGSIOTrackedFrameList> trackedFrameList = vtkSmartPointer<vtkIGSIOTrackedFrameList>::New();
  trackedFrameList->SetCustomString("TrackName", fileNameNoExtension);
  if (!vtkIGSIOSequenceIO::Read(fileName.toStdString(), trackedFrameList))
  {
    qCritical() << Q_FUNC_INFO << " error reading video: " << fileName;
    return false;
  }

  vtkSmartPointer<vtkMRMLSequenceBrowserNode> sequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast(
        this->mrmlScene()->AddNewNodeByClass("vtkMRMLSequenceBrowserNode", fileNameNoExtension));
  if (!vtkSlicerIGSIOCommon::TrackedFrameListToSequenceBrowser(trackedFrameList, sequenceBrowserNode))
  {
    this->mrmlScene()->RemoveNode(sequenceBrowserNode);
    qCritical() << Q_FUNC_INFO << " could not convert tracked frame list to sequence browser node";
    return false;
  }

  vtkSlicerApplicationLogic* appLogic = this->SequenceIOLogic()->GetApplicationLogic();
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
