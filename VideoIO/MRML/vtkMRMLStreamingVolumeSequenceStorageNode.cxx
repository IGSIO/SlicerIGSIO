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

// VTK includes
#include <vtkStringArray.h>

//MRML includes
#include <vtkMRMLStreamingVolumeNode.h>
#include <vtkStreamingVolumeCodecFactory.h>

// Sequence MRML includes
#include <vtkMRMLSequenceNode.h>

// SlicerIGSIOCommon includes
#include "vtkSlicerIGSIOCommon.h"

// IGSIO VideoIO includes
#include <vtkGenericVideoReader.h>
#include <vtkGenericVideoWriter.h>

// IGSIOCommon includes
#include "vtkTrackedFrameList.h"

// VideoIO MRML includes
#include "vtkMRMLStreamingVolumeSequenceStorageNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLStreamingVolumeSequenceStorageNode);

//----------------------------------------------------------------------------
vtkMRMLStreamingVolumeSequenceStorageNode::vtkMRMLStreamingVolumeSequenceStorageNode()
  : CodecFourCC("")
{
}

//----------------------------------------------------------------------------
vtkMRMLStreamingVolumeSequenceStorageNode::~vtkMRMLStreamingVolumeSequenceStorageNode()
{
}

//----------------------------------------------------------------------------
bool vtkMRMLStreamingVolumeSequenceStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLSequenceNode");
}

//---------------------------------------------------------------------------
bool vtkMRMLStreamingVolumeSequenceStorageNode::ReadVideo(std::string fileName, vtkTrackedFrameList* trackedFrameList)
{
  vtkSmartPointer<vtkGenericVideoReader> reader = vtkSlicerIGSIOCommon::CreateVideoReader(fileName);
  if (!reader)
  {
    vtkErrorWithObjectMacro(trackedFrameList, "Cannot create reader for file: " << fileName);
    return false;
  }
  reader->SetFilename(fileName);
  reader->SetTrackedFrameList(trackedFrameList);
  return reader->ReadFile();
}

//---------------------------------------------------------------------------
bool vtkMRMLStreamingVolumeSequenceStorageNode::WriteVideo(std::string fileName, vtkTrackedFrameList* trackedFrameList)
{
  vtkSmartPointer<vtkGenericVideoWriter> writer = vtkSlicerIGSIOCommon::CreateVideoWriter(fileName);
  if (!writer)
  {
    vtkErrorWithObjectMacro(trackedFrameList, "Cannot create writer for file: " << fileName);
    return false;
  }
  writer->SetFilename(fileName);
  writer->SetTrackedFrameList(trackedFrameList);
  return writer->WriteFile();
}

//----------------------------------------------------------------------------
int vtkMRMLStreamingVolumeSequenceStorageNode::ReadDataInternal(vtkMRMLNode* refNode)
{
  vtkMRMLSequenceNode* sequenceNode = vtkMRMLSequenceNode::SafeDownCast(refNode);
  if (!sequenceNode)
  {
    vtkErrorMacro("Cannot convert reference node to vtkMRMLSequenceNode");
    return 0;
  }

  vtkSmartPointer<vtkTrackedFrameList> trackedFrameList = vtkSmartPointer<vtkTrackedFrameList>::New();
  this->ReadVideo(this->FileName, trackedFrameList);
  vtkSlicerIGSIOCommon::TrackedFrameListToVolumeSequence(trackedFrameList, sequenceNode);
  this->CodecFourCC = trackedFrameList->GetCodecFourCC();

  return 1;
}

//----------------------------------------------------------------------------
bool vtkMRMLStreamingVolumeSequenceStorageNode::CanWriteFromReferenceNode(vtkMRMLNode *refNode)
{

  return true;
}

//----------------------------------------------------------------------------
int vtkMRMLStreamingVolumeSequenceStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLSequenceNode* videoStreamSequenceNode = vtkMRMLSequenceNode::SafeDownCast(refNode);
  if (!videoStreamSequenceNode)
  {
    vtkErrorMacro("Cannot convert reference node to vtkMRMLSequenceNode");
    return 0;
  }

  vtkSlicerIGSIOCommon::ReEncodeVideoSequence(videoStreamSequenceNode, 0, -1, this->CodecFourCC,std::map<std::string, std::string>(), false, true);

  vtkSmartPointer<vtkTrackedFrameList> trackedFrameList = vtkSmartPointer <vtkTrackedFrameList>::New();
  vtkSlicerIGSIOCommon::VolumeSequenceToTrackedFrameList(videoStreamSequenceNode, trackedFrameList);
  this->WriteVideo(this->GetFileName(), trackedFrameList);

  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLStreamingVolumeSequenceStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Matroska Video (.mkv)");
}

//----------------------------------------------------------------------------
void vtkMRMLStreamingVolumeSequenceStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Matroska Video (.mkv)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLStreamingVolumeSequenceStorageNode::GetDefaultWriteFileExtension()
{
  return "mkv";
}

//----------------------------------------------------------------------------
void vtkMRMLStreamingVolumeSequenceStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLStdStringMacro(codecFourCC, CodecFourCC);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLStreamingVolumeSequenceStorageNode::WriteXML(ostream& of, int indent)
{
  Superclass::WriteXML(of, indent);
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLStdStringMacro(codecFourCC, CodecFourCC);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLStreamingVolumeSequenceStorageNode::Copy(vtkMRMLNode *node)
{
  Superclass::Copy(node);
  vtkMRMLCopyBeginMacro(node);
  vtkMRMLCopyStdStringMacro(CodecFourCC);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLStreamingVolumeSequenceStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintStdStringMacro(CodecFourCC);
  vtkMRMLPrintEndMacro();
}
