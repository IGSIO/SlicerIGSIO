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

  this->UpdateCompressionPresets();
  if (this->CompressionParameter.empty())
  {
    vtkErrorMacro(<< "WriteData: Could not determine which encoding to use for node " << refNode->GetID()
                  << ". Select a codec or compression parameter and try again");
    return 0;
  }

  vtkSmartPointer<vtkStreamingVolumeCodec> codec = vtkSmartPointer<vtkStreamingVolumeCodec>::Take(
    vtkStreamingVolumeCodecFactory::GetInstance()->CreateCodecByFourCC(this->CodecFourCC));

  std::vector<std::string> parameterNames;
  if (codec)
  {
    codec->SetParametersFromPresetValue(this->CompressionParameter);
    parameterNames = codec->GetAvailiableParameterNames();
  }
  

  std::map<std::string, std::string> parameters;
  std::vector<std::string>::iterator parameterNameIt;
  for (parameterNameIt = parameterNames.begin(); parameterNameIt != parameterNames.end(); ++parameterNameIt)
  {
    if ((*parameterNameIt).empty())
    {
      continue;
    }
    std::string parameterValue;
    if (codec->GetParameter(*parameterNameIt, parameterValue))
    {
      parameters[*parameterNameIt] = parameterValue;
    }
  }

  vtkSlicerIGSIOCommon::ReEncodeVideoSequence(videoStreamSequenceNode, 0, -1, this->CodecFourCC, parameters, false, true);

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
void vtkMRMLStreamingVolumeSequenceStorageNode::UpdateCompressionPresets()
{
  this->CompressionPresets.clear();

  std::map<std::string, std::string> codecPresetFourCCs;
  std::vector<std::string> fourCCs = vtkStreamingVolumeCodecFactory::GetInstance()->GetStreamingCodecFourCCs();
  for (std::vector<std::string>::iterator fourCCIt = fourCCs.begin(); fourCCIt != fourCCs.end(); ++fourCCIt)
  {
    vtkSmartPointer<vtkStreamingVolumeCodec> codec = vtkSmartPointer<vtkStreamingVolumeCodec>::Take(vtkStreamingVolumeCodecFactory::GetInstance()->CreateCodecByFourCC(*fourCCIt));
    std::vector<vtkStreamingVolumeCodec::ParameterPreset> presets = codec->GetParameterPresets();
    for (std::vector<vtkStreamingVolumeCodec::ParameterPreset>::iterator presetIt = presets.begin(); presetIt != presets.end(); ++presetIt)
    {
      codecPresetFourCCs[presetIt->Value] = codec->GetFourCC();
      CompressionPreset preset;
      preset.DisplayName = presetIt->Name;
      preset.CompressionParameter = presetIt->Value;
      this->CompressionPresets.push_back(preset);
    }
  }

  // FourCC not specified
  if (this->CodecFourCC.empty())
  {
    // Compression parameter is not specified
    if (!this->CompressionParameter.empty())
    {
      // Compression parameter is specified, we can determine codec FourCC from that
      std::map<std::string, std::string>::iterator codecPresetIt =
        codecPresetFourCCs.find(this->CompressionParameter);
      if (codecPresetIt != codecPresetFourCCs.end())
      {
        this->CodecFourCC = codecPresetIt->second;
      }
    }
    else
    {
      vtkSmartPointer<vtkMRMLSequenceNode> sequenceNode = vtkMRMLSequenceNode::SafeDownCast(this->GetStorableNode());
      if (sequenceNode)
      {
        // Find the first specified codec in the sequence node
        for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); ++i)
        {
          vtkMRMLStreamingVolumeNode* streamingVolume = vtkMRMLStreamingVolumeNode::SafeDownCast(
            sequenceNode->GetNthDataNode(i));
          if (!streamingVolume)
          {
            continue;
          }

          std::string streamingCodec = streamingVolume->GetCodecFourCC();
          if (!streamingCodec.empty())
          {
            this->CodecFourCC = streamingCodec;
            break;
          }
        }
      }
    }
  }

  // Codec is specified but compression paramter is not.
  if (!this->CodecFourCC.empty() && this->CompressionParameter.empty())
  {
    vtkSmartPointer<vtkStreamingVolumeCodec> codec = vtkSmartPointer<vtkStreamingVolumeCodec>::Take(
      vtkStreamingVolumeCodecFactory::GetInstance()->CreateCodecByFourCC(this->CodecFourCC));

    // Find the default compression parameter for the matching codec
    this->CompressionParameter = codec->GetDefaultParameterPresetValue();
  }
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
