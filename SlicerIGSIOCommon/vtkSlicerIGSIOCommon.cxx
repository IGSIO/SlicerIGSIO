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

// SlicerIGSIOCommon includes
#include <igsioTrackedFrame.h>
#include <igsioVideoFrame.h>
#include "vtkSlicerIGSIOCommon.h"
#include "vtkStreamingVolumeCodec.h"
#include <vtkTrackedFrameList.h>

// vtkAddon includes
#include <vtkStreamingVolumeCodecFactory.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSequenceBrowserNode.h>
#include <vtkMRMLSequenceNode.h>
#include <vtkMRMLStreamingVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkMatrix4x4.h>

// vtkVideoIO includes
#include <vtkMKVReader.h>
#include <vtkMKVWriter.h>

//----------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
bool vtkSlicerIGSIOCommon::TrackedFrameListToVolumeSequence(vtkTrackedFrameList* trackedFrameList, vtkMRMLSequenceNode* sequenceNode)
{
  if (!trackedFrameList || !sequenceNode)
  {
    vtkErrorWithObjectMacro(trackedFrameList, "Invalid arguments");
    return false;
  }

  std::string trackedFrameName = trackedFrameList->GetName();
  sequenceNode->SetIndexName("time");
  sequenceNode->SetIndexUnit("s");

  FrameSizeType frameSize = { 0,0,0 };
  if (trackedFrameList->GetUseCompression())
  {
    frameSize = trackedFrameList->GetCompressedFrameSize();
  }
  else
  {
    trackedFrameList->GetFrameSize(frameSize);
  }

  std::string codecFourCC = trackedFrameList->GetCodecFourCC();
  vtkSmartPointer<vtkStreamingVolumeCodec> codec = vtkSmartPointer<vtkStreamingVolumeCodec>::Take(
    vtkStreamingVolumeCodecFactory::GetInstance()->CreateCodecByFourCC(codecFourCC));
  if (!codec)
  {
    vtkErrorWithObjectMacro(sequenceNode, "Could not find codec: " << codecFourCC);
    return false;
  }

  vtkSmartPointer<vtkStreamingVolumeFrame> previousFrame = NULL;

  // How many digits are required to represent the frame numbers
  int frameNumberMaxLength = std::floor(std::log10(trackedFrameList->GetNumberOfTrackedFrames())) + 1;

  for (int i = 0; i < trackedFrameList->GetNumberOfTrackedFrames(); ++i)
  {
    // Convert frame to a string with the a maximum number of digits (frameNumberMaxLength)
    // ex. 0, 1, 2, 3 or 0000, 0001, 0002, 0003 etc.
    std::stringstream frameNumberSS;
    frameNumberSS << std::setw(frameNumberMaxLength) << std::setfill('0') << i;

    std::stringstream nameSS;
    nameSS << trackedFrameName << "_" << frameNumberSS.str();
    std::string name = nameSS.str();

    igsioTrackedFrame* trackedFrame = trackedFrameList->GetTrackedFrame(i);
    std::stringstream timestampSS;
    timestampSS << trackedFrame->GetTimestamp();

    vtkSmartPointer<vtkMRMLVolumeNode> volumeNode;
    if (!trackedFrameList->GetUseCompression())
    {
      volumeNode = vtkSmartPointer<vtkMRMLVectorVolumeNode>::New();
      volumeNode->SetAndObserveImageData(trackedFrame->GetImageData()->GetImage());
    }
    else
    {
      vtkSmartPointer<vtkMRMLStreamingVolumeNode> streamingVolumeNode = vtkSmartPointer<vtkMRMLStreamingVolumeNode>::New();
      vtkSmartPointer<vtkStreamingVolumeFrame> currentFrame = vtkSmartPointer<vtkStreamingVolumeFrame>::New();
      currentFrame->SetDimensions(frameSize[0], frameSize[1], frameSize[2]);
      currentFrame->SetFrameData(trackedFrame->GetImageData()->GetCompressedFrameData());
      currentFrame->SetFrameType(trackedFrame->GetImageData()->GetFrameType());
      currentFrame->SetCodecFourCC(trackedFrameList->GetCodecFourCC());

      // The previous frame is only relevant if the current frame is not a keyframe
      if (!trackedFrame->GetImageData()->IsKeyFrame())
      {
        currentFrame->SetPreviousFrame(previousFrame);
      }
      streamingVolumeNode->SetAndObserveFrame(currentFrame);
      volumeNode = streamingVolumeNode;
      previousFrame = currentFrame;
    }

    vtkSmartPointer<vtkMatrix4x4> ijkToRASTransformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    igsioTransformName ijkToRasTransformName;
    ijkToRasTransformName.SetTransformName("IJKToRAS");
    if (trackedFrame->GetCustomFrameTransform(ijkToRasTransformName, ijkToRASTransformMatrix) == IGSIO_SUCCESS)
    {
      if (volumeNode)
      {
        volumeNode->SetIJKToRASMatrix(ijkToRASTransformMatrix);
      }
    }

    volumeNode->SetName(name.c_str());
    sequenceNode->SetDataNodeAtValue(volumeNode, timestampSS.str());
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerIGSIOCommon::TrackedFrameListToSequenceBrowser(vtkTrackedFrameList* trackedFrameList, vtkMRMLSequenceBrowserNode* sequenceBrowserNode)
{
  if (!trackedFrameList || !sequenceBrowserNode)
  {
    vtkErrorWithObjectMacro(trackedFrameList, "Invalid argument!");
    return false;
  }

  vtkMRMLScene* scene = sequenceBrowserNode->GetScene();
  if (!scene)
  {
    vtkErrorWithObjectMacro(sequenceBrowserNode, "No scene found in sequence browser nodes!");
    return false;
  }

  std::string name = trackedFrameList->GetName();

  vtkSmartPointer<vtkMRMLSequenceNode> videoSequenceNode = vtkSmartPointer <vtkMRMLSequenceNode>::New();
  videoSequenceNode->SetName(scene->GetUniqueNameByString(name.c_str()));
  vtkSlicerIGSIOCommon::TrackedFrameListToVolumeSequence(trackedFrameList, videoSequenceNode);
  scene->AddNode(videoSequenceNode);

  if (videoSequenceNode->GetNumberOfDataNodes() < 1)
  {
    vtkErrorWithObjectMacro(trackedFrameList, "No frames in trackedframelist!");
    return false;
  }
  sequenceBrowserNode->AddSynchronizedSequenceNode(videoSequenceNode);

  FrameSizeType frameSize = { 0,0,0 };
  if (trackedFrameList->GetUseCompression())
  {
    frameSize = trackedFrameList->GetCompressedFrameSize();
  }
  else
  {
    trackedFrameList->GetFrameSize(frameSize);
  }

  int dimensions[3] = { 0,0,0 };
  dimensions[0] = frameSize[0];
  dimensions[1] = frameSize[1];
  dimensions[2] = frameSize[2];

  std::map<std::string, vtkSmartPointer<vtkMRMLSequenceNode>> transformSequenceNodes;

  int frameNumberMaxLength = std::floor(std::log10(trackedFrameList->GetNumberOfTrackedFrames())) + 1;
  for (int i = 0; i < trackedFrameList->GetNumberOfTrackedFrames(); ++i)
  {
    igsioTrackedFrame* trackedFrame = trackedFrameList->GetTrackedFrame(i);
    std::stringstream timestampSS;
    timestampSS << trackedFrame->GetTimestamp();

    std::vector<igsioTransformName> transformNames;
    trackedFrame->GetCustomFrameTransformNameList(transformNames);
    for (std::vector<igsioTransformName>::iterator transformNameIt = transformNames.begin(); transformNameIt != transformNames.end(); ++transformNameIt)
    {
      vtkSmartPointer<vtkMatrix4x4> transformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      trackedFrame->GetCustomFrameTransform(*transformNameIt, transformMatrix);

      std::string transformName;
      transformNameIt->GetTransformName(transformName);
      if (transformName != "IJKToRAS")
      {
        vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
        transformNode->SetMatrixTransformToParent(transformMatrix);

        std::string transformName = transformNameIt->GetTransformName();
        if (transformSequenceNodes.find(transformName) == transformSequenceNodes.end())
        {
          vtkSmartPointer<vtkMRMLSequenceNode> transformSequenceNode = vtkMRMLSequenceNode::SafeDownCast(
            scene->AddNewNodeByClass("vtkMRMLSequenceNode"));
          transformSequenceNode->SetName(transformName.c_str());
          transformSequenceNode->SetIndexName("time");
          transformSequenceNode->SetIndexUnit("s");
          sequenceBrowserNode->AddSynchronizedSequenceNode(transformSequenceNode);
          transformSequenceNodes[transformName] = transformSequenceNode;
        }
        transformSequenceNodes[transformName]->SetDataNodeAtValue(transformNode, timestampSS.str());
      }
    }

  }

  sequenceBrowserNode->SetPlaybackRateFps(trackedFrameList->GetFPS());

  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerIGSIOCommon::VolumeSequenceToTrackedFrameList(vtkMRMLSequenceNode* sequenceNode, vtkTrackedFrameList* trackedFrameList)
{
  if (!sequenceNode || !trackedFrameList)
  {
    vtkErrorWithObjectMacro(sequenceNode, "Invalid arguments");
    return false;
  }

  if (sequenceNode->GetNumberOfDataNodes() < 1)
  {
    vtkErrorWithObjectMacro(sequenceNode, "No data nodes in sequence");
    return false;
  }

  std::string codecFourCC;
  bool useTimestamp = sequenceNode->GetIndexName() == "time";

  int dimensions[3] = { 0,0,0 };

  double timestamp = 0.0;
  for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); ++i)
  {
    vtkMRMLStreamingVolumeNode* streamingVolumeNode = vtkMRMLStreamingVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(i));
    if (!streamingVolumeNode)
    {
      continue;
    }

    vtkStreamingVolumeFrame* frame = streamingVolumeNode->GetFrame();
    if (!frame)
    {
      continue;
    }

    frame->GetDimensions(dimensions);
    codecFourCC = streamingVolumeNode->GetCodecFourCC();

    std::stringstream timestampSS;
    timestampSS << sequenceNode->GetNthIndexValue(i);
    if (useTimestamp)
    {
      timestampSS >> timestamp;
    }
    else
    {
      timestamp += 0.1;
    }

    vtkSmartPointer<vtkMatrix4x4> ijkToRASTransform = vtkSmartPointer<vtkMatrix4x4>::New();
    streamingVolumeNode->GetIJKToRASMatrix(ijkToRASTransform);

    igsioTransformName ijkToRasName;
    ijkToRasName.SetTransformName("IJKToRAS");

    igsioTrackedFrame trackedFrame;
    igsioVideoFrame videoFrame;
    videoFrame.SetCompressedFrameData(frame->GetFrameData());
    videoFrame.SetFrameType(frame->GetFrameType());
    trackedFrame.SetImageData(videoFrame);
    trackedFrame.SetTimestamp(timestamp);
    trackedFrame.SetCustomFrameTransform(ijkToRasName, ijkToRASTransform);
    trackedFrameList->AddTrackedFrame(&trackedFrame);
  }
  trackedFrameList->SetCodecFourCC(codecFourCC);
  trackedFrameList->SetUseCompression(true);
  FrameSizeType frameSize = { 0,0,0 };
  for (int i=0; i<3; ++i)
  {
    frameSize[i] = (unsigned int)dimensions[i];
  }

  trackedFrameList->SetCompressedFrameSize(frameSize);
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerIGSIOCommon::SequenceBrowserToTrackedFrameList(vtkMRMLSequenceBrowserNode* sequenceBrowserNode, vtkTrackedFrameList* trackedFrameList)
{
  // TODO
  return false;
}

//---------------------------------------------------------------------------
vtkSmartPointer<vtkGenericVideoReader> vtkSlicerIGSIOCommon::CreateVideoReader(std::string fileName)
{
  vtkSmartPointer<vtkGenericVideoReader> reader = NULL;
  if (vtkMKVReader::CanReadFile(fileName))
  {
    reader = vtkSmartPointer<vtkMKVReader>::New();
  }
  return reader;
}

//---------------------------------------------------------------------------
vtkSmartPointer<vtkGenericVideoWriter> vtkSlicerIGSIOCommon::CreateVideoWriter(std::string fileName)
{
  vtkSmartPointer<vtkGenericVideoWriter> writer = NULL;
  if (vtkMKVWriter::CanWriteFile(fileName))
  {
    writer = vtkSmartPointer<vtkMKVWriter>::New();
  }
  return writer;
}

//----------------------------------------------------------------------------
bool vtkSlicerIGSIOCommon::ReEncodeVideoSequence(vtkMRMLSequenceNode* videoStreamSequenceNode, int startIndex, int endIndex, std::string codecFourCC, std::map<std::string, std::string> codecParameters)
{
  if (!videoStreamSequenceNode)
  {
    vtkErrorWithObjectMacro(videoStreamSequenceNode, "Cannot convert reference node to vtkMRMLSequenceNode");
    return false;
  }

  int dimensions[3] = { 0,0,0 };
  int numberOfFrames = videoStreamSequenceNode->GetNumberOfDataNodes();

  if (endIndex < 0)
  {
    endIndex = numberOfFrames - 1;
  }

  if (startIndex < 0 || startIndex >= numberOfFrames || startIndex > endIndex
    || endIndex >= numberOfFrames)
  {
    vtkErrorWithObjectMacro(videoStreamSequenceNode, "Invalid start and end indices!");
    return false;
  }

  std::vector<FrameBlock> frameBlocks;
  FrameBlock currentFrameBlock;
  currentFrameBlock.StartFrame = 0;
  currentFrameBlock.EndFrame = 0;
  currentFrameBlock.ReEncodingRequired = false;
  vtkSmartPointer<vtkStreamingVolumeFrame> previousFrame = NULL;
  for (int i = startIndex; i <= endIndex; ++i)
  {
    // TODO: for now, only support sequences of vtkMRMLStreamingVolumeNode
    // In the future, this could be changed to allow all types of volume nodes to be encoded
    vtkMRMLStreamingVolumeNode* streamingNode = vtkMRMLStreamingVolumeNode::SafeDownCast(videoStreamSequenceNode->GetNthDataNode(i));
    if (!streamingNode)
    {
      vtkErrorWithObjectMacro(videoStreamSequenceNode, "Invalid data node at index " << i);
      return false;
    }

    if (codecFourCC == "")
    {
      codecFourCC = streamingNode->GetCodecFourCC();
    }

    vtkStreamingVolumeFrame* currentFrame = streamingNode->GetFrame();
    if (i == 0 && !streamingNode->IsKeyFrame())
    {
      currentFrameBlock.ReEncodingRequired = true;
    }
    else if (!currentFrame)
    {
      currentFrameBlock.ReEncodingRequired = true;
    }
    else if (codecFourCC == "")
    {
      currentFrameBlock.ReEncodingRequired = true;
    }
    else if (codecFourCC != streamingNode->GetCodecFourCC())
    {
      currentFrameBlock.ReEncodingRequired = true;
    }
    else if (currentFrame && !currentFrame->IsKeyFrame() && previousFrame != currentFrame->GetPreviousFrame())
    {
      currentFrameBlock.ReEncodingRequired = true;
    }
    previousFrame = currentFrame;

    if (streamingNode->IsKeyFrame() && i != currentFrameBlock.StartFrame && currentFrame)
    {
      frameBlocks.push_back(currentFrameBlock);
      currentFrameBlock = FrameBlock();
      currentFrameBlock.StartFrame = i;
      currentFrameBlock.EndFrame = i;
      currentFrameBlock.ReEncodingRequired = false;
    }
    currentFrameBlock.EndFrame = i;

    /// TODO: If dimension changes, re-encode smaller images with padding
  }
  frameBlocks.push_back(currentFrameBlock);

  if (codecFourCC == "")
  {
    std::vector<std::string> codecFourCCs = vtkStreamingVolumeCodecFactory::GetInstance()->GetStreamingCodecFourCCs();
    if (codecFourCCs.empty())
    {
      vtkErrorWithObjectMacro(videoStreamSequenceNode, "Re-encode failed! No codecs registered!");
      return false;
    }

    codecFourCC = codecFourCCs.front();
    vtkDebugWithObjectMacro(videoStreamSequenceNode, "Streaming volume codec not specified! Using: " << codecFourCC);
  }

  std::vector<FrameBlock>::iterator frameBlockIt;
  for (frameBlockIt = frameBlocks.begin(); frameBlockIt != frameBlocks.end(); ++frameBlockIt)
  {
    if (frameBlockIt->ReEncodingRequired)
    {
      vtkNew<vtkMRMLStreamingVolumeNode> encodingProxyNode;
      for (int i = frameBlockIt->StartFrame; i <= frameBlockIt->EndFrame; ++i)
      {
        vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(videoStreamSequenceNode->GetNthDataNode(i));
        if (!volumeNode)
        {
          vtkErrorWithObjectMacro(videoStreamSequenceNode, "Invalid data node at index " << i);
          return false;
        }

        vtkMRMLStreamingVolumeNode* streamingNode = vtkMRMLStreamingVolumeNode::SafeDownCast(videoStreamSequenceNode->GetNthDataNode(i));
        if (streamingNode && streamingNode->GetFrame())
        {
          encodingProxyNode->SetAndObserveFrame(streamingNode->GetFrame());
          encodingProxyNode->DecodeFrame();
        }
        else
        {
          encodingProxyNode->SetAndObserveImageData(volumeNode->GetImageData());
        }
        encodingProxyNode->SetCodecFourCC(codecFourCC);

        vtkStreamingVolumeCodec* codec = encodingProxyNode->GetCodec();
        if (!codec)
        {
          return false;
        }

        codec->SetParameters(codecParameters);

        encodingProxyNode->EncodeImageData();
        streamingNode->SetAndObserveImageData(NULL);
        streamingNode->SetAndObserveFrame(encodingProxyNode->GetFrame());
      }
    }
  }
  return true;
}
