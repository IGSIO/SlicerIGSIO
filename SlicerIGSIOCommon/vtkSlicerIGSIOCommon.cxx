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

// IGSIOCommon includes
#include <igsioTrackedFrame.h>
#include <igsioVideoFrame.h>
#include <vtkIGSIOTrackedFrameList.h>
#include <vtkIGSIOTransformRepository.h>

// SlicerIGSIOCommon includes
#include "vtkSlicerIGSIOCommon.h"
#include "vtkStreamingVolumeCodec.h"

// vtkAddon includes
#include <vtkStreamingVolumeCodecFactory.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSequenceBrowserNode.h>
#include <vtkMRMLSequenceNode.h>
#include <vtkMRMLStreamingVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLScalarVolumeNode.h>

#include <vtkMRMLSequenceStorageNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>

// vtkSequenceIO includes
#include <vtkIGSIOMkvSequenceIO.h>

#include <stack>

std::string FRAME_STATUS_TRACKNAME = "FrameStatus";
std::string TRACKNAME_FIELD_NAME = "TrackName";
enum FrameStatus
{
  Frame_OK,
  Frame_Invalid,
  Frame_Skip,
};

struct FrameBlock
{
  int StartFrame;
  int EndFrame;
  bool ReEncodingRequired;
  FrameBlock()
    : StartFrame(-1)
    , EndFrame(-1)
    , ReEncodingRequired(false)
  {
  }
};

//----------------------------------------------------------------------------
bool vtkSlicerIGSIOCommon::TrackedFrameListToVolumeSequence(vtkIGSIOTrackedFrameList* trackedFrameList, vtkMRMLSequenceNode* sequenceNode)
{
  if (!trackedFrameList || !sequenceNode)
  {
    vtkErrorWithObjectMacro(trackedFrameList, "Invalid arguments");
    return false;
  }

  std::string trackedFrameName = "Video";
  if (!trackedFrameList->GetCustomString(TRACKNAME_FIELD_NAME).empty())
  {
    trackedFrameName = trackedFrameList->GetCustomString(TRACKNAME_FIELD_NAME);
  }

  sequenceNode->SetIndexName("time");
  sequenceNode->SetIndexUnit("s");

  std::string encodingFourCC;
  trackedFrameList->GetEncodingFourCC(encodingFourCC);
  if (!encodingFourCC.empty())
  {
    vtkSmartPointer<vtkStreamingVolumeCodec> codec = vtkSmartPointer<vtkStreamingVolumeCodec>::Take(
      vtkStreamingVolumeCodecFactory::GetInstance()->CreateCodecByFourCC(encodingFourCC));
    if (!codec)
    {
      vtkErrorWithObjectMacro(sequenceNode, "Could not find codec: " << encodingFourCC);
      return false;
    }
  }

  vtkSmartPointer<vtkMRMLScene> mrmlScene = sequenceNode->GetScene();

  vtkSmartPointer<vtkStreamingVolumeFrame> previousFrame = NULL;

  // How many digits are required to represent the frame numbers
  int frameNumberMaxLength = std::floor(std::log10(trackedFrameList->GetNumberOfTrackedFrames())) + 1;

  for (unsigned int i = 0; i < trackedFrameList->GetNumberOfTrackedFrames(); ++i)
  {
    igsioTrackedFrame* trackedFrame = trackedFrameList->GetTrackedFrame(i);
    std::stringstream timestampSS;
    timestampSS << trackedFrame->GetTimestamp();

    vtkSmartPointer<vtkMRMLVolumeNode> volumeNode;
    if (!trackedFrame->GetImageData()->IsFrameEncoded())
    {
      unsigned int numberOfScalarComponents = 0;
      if (trackedFrame->GetNumberOfScalarComponents(numberOfScalarComponents) && numberOfScalarComponents > 1)
      {
        if (mrmlScene)
        {
          volumeNode = vtkSmartPointer<vtkMRMLVolumeNode>::Take(vtkMRMLVolumeNode::SafeDownCast(mrmlScene->CreateNodeByClass("vtkMRMLVectorVolumeNode")));
        }
        if (!volumeNode)
        {
          volumeNode = vtkSmartPointer<vtkMRMLVectorVolumeNode>::New();
        }

      }
      else
      {
        if (mrmlScene)
        {
          volumeNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::Take(vtkMRMLScalarVolumeNode::SafeDownCast(mrmlScene->CreateNodeByClass("vtkMRMLScalarVolumeNode")));
        }
        if (!volumeNode)
        {
          volumeNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
        }
      }
      volumeNode->SetAndObserveImageData(trackedFrame->GetImageData()->GetImage());
    }
    else
    {
      vtkSmartPointer<vtkMRMLStreamingVolumeNode> streamingVolumeNode = nullptr;
      if (sequenceNode->GetScene())
      {
        streamingVolumeNode = vtkSmartPointer<vtkMRMLStreamingVolumeNode>::Take(vtkMRMLStreamingVolumeNode::SafeDownCast(sequenceNode->GetScene()->CreateNodeByClass("vtkMRMLStreamingVolumeNode")));
      }
      if (!streamingVolumeNode)
      {
        streamingVolumeNode = vtkSmartPointer<vtkMRMLStreamingVolumeNode>::New();
      }

      vtkSmartPointer<vtkStreamingVolumeFrame> currentFrame = trackedFrame->GetImageData()->GetEncodedFrame();

      // The previous frame is only relevant if the current frame is not a keyframe
      if (!trackedFrame->GetImageData()->GetEncodedFrame()->IsKeyFrame())
      {
        currentFrame->SetPreviousFrame(previousFrame);
      }
      streamingVolumeNode->SetAndObserveFrame(currentFrame);
      volumeNode = streamingVolumeNode;
      previousFrame = currentFrame;
    }

    vtkSmartPointer<vtkMatrix4x4> ijkToRASTransformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    igsioTransformName imageToPhysicalTransformName;
    imageToPhysicalTransformName.SetTransformName(trackedFrameName + "ToPhysical");
    if (!trackedFrame->GetFrameField(imageToPhysicalTransformName.GetTransformName() + "Transform").empty())
    {
      if (trackedFrame->GetFrameTransform(imageToPhysicalTransformName, ijkToRASTransformMatrix) == IGSIO_SUCCESS)
      {
        if (volumeNode)
        {
          volumeNode->SetIJKToRASMatrix(ijkToRASTransformMatrix);
        }
      }
    }

    // Convert frame to a string with the maximum number of digits (frameNumberMaxLength)
    // ex. 0, 1, 2, 3 or 0000, 0001, 0002, 0003 etc.
    std::stringstream frameNumberSS;
    frameNumberSS << std::setw(frameNumberMaxLength) << std::setfill('0') << i;

    // Generating a unique name is important because that will be used to generate the filename by default
    std::ostringstream nameStr;
    nameStr << "Image_" << frameNumberSS.str() << std::ends;
    std::string volumeName = nameStr.str();
    volumeNode->SetName(volumeName.c_str());

    std::string frameStatus = trackedFrame->GetFrameField(FRAME_STATUS_TRACKNAME);
    if (!frameStatus.empty() || vtkVariant(frameStatus).ToInt() != Frame_Skip)
    {
      sequenceNode->SetDataNodeAtValue(volumeNode, timestampSS.str());
    }
  }

  sequenceNode->SetAttribute("Sequences.Source", "Image");

  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerIGSIOCommon::TrackedFrameListToSequenceBrowser(vtkIGSIOTrackedFrameList* trackedFrameList, vtkMRMLSequenceBrowserNode* sequenceBrowserNode)
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

  std::string trackedFrameName = "Video";
  if (!trackedFrameList->GetCustomString(TRACKNAME_FIELD_NAME).empty())
  {
    trackedFrameName = trackedFrameList->GetCustomString(TRACKNAME_FIELD_NAME);
  }

  std::string imagesSequenceName = vtkMRMLSequenceStorageNode::GetSequenceNodeName(trackedFrameName, "Image");
  vtkSmartPointer<vtkMRMLSequenceNode> videoSequenceNode = vtkMRMLSequenceNode::SafeDownCast(
    scene->AddNewNodeByClass("vtkMRMLSequenceNode", imagesSequenceName.c_str()));
  vtkSlicerIGSIOCommon::TrackedFrameListToVolumeSequence(trackedFrameList, videoSequenceNode);

  if (videoSequenceNode->GetNumberOfDataNodes() < 1)
  {
    // No images in tracked frame list
    scene->RemoveNode(videoSequenceNode);
  }
  else
  {
    sequenceBrowserNode->AddSynchronizedSequenceNode(videoSequenceNode);
  }

  std::map<std::string, vtkSmartPointer<vtkMRMLSequenceNode>> transformSequenceNodes;

  int frameNumberMaxLength = std::floor(std::log10(trackedFrameList->GetNumberOfTrackedFrames())) + 1;
  for (unsigned int i = 0; i < trackedFrameList->GetNumberOfTrackedFrames(); ++i)
  {
    igsioTrackedFrame* trackedFrame = trackedFrameList->GetTrackedFrame(i);
    std::stringstream timestampSS;
    timestampSS << trackedFrame->GetTimestamp();

    std::vector<igsioTransformName> transformNames;
    trackedFrame->GetFrameTransformNameList(transformNames);
    for (std::vector<igsioTransformName>::iterator transformNameIt = transformNames.begin(); transformNameIt != transformNames.end(); ++transformNameIt)
    {
      vtkSmartPointer<vtkMatrix4x4> transformMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
      trackedFrame->GetFrameTransform(*transformNameIt, transformMatrix);

      std::string transformName;
      transformNameIt->GetTransformName(transformName);
      if (transformName != trackedFrameName + "ToPhysical")
      {
        vtkSmartPointer<vtkMRMLLinearTransformNode> transformNode = nullptr;
        if (scene)
        {
          transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::Take(vtkMRMLLinearTransformNode::SafeDownCast(scene->CreateNodeByClass("vtkMRMLLinearTransformNode")));
        }
        if (!transformNode)
        {
          transformNode = vtkSmartPointer<vtkMRMLLinearTransformNode>::New();
        }

        transformNode->SetMatrixTransformToParent(transformMatrix);

        // Convert frame to a string with the a maximum number of digits (frameNumberMaxLength)
        // ex. 0, 1, 2, 3 or 0000, 0001, 0002, 0003 etc.
        std::stringstream frameNumberSS;
        frameNumberSS << std::setw(frameNumberMaxLength) << std::setfill('0') << i;

        // Generating a unique name is important because that will be used to generate the filename by default
        std::ostringstream nameStr;
        nameStr << transformName << "Transform_" << frameNumberSS.str() << std::ends;
        std::string transformNodeName = nameStr.str();
        transformNode->SetName(transformNodeName.c_str());

        if (transformSequenceNodes.find(transformName) == transformSequenceNodes.end())
        {
          std::string transformSequenceName = vtkMRMLSequenceStorageNode::GetSequenceNodeName(trackedFrameName, transformName);
          vtkSmartPointer<vtkMRMLSequenceNode> transformSequenceNode = vtkMRMLSequenceNode::SafeDownCast(
            scene->AddNewNodeByClass("vtkMRMLSequenceNode", transformSequenceName.c_str()));
          transformSequenceNode->SetIndexName("time");
          transformSequenceNode->SetIndexUnit("s");
          // Save transform name to Sequences.Source attribute so that modules can
          // find a transform by matching the original the transform name.
          transformSequenceNode->SetAttribute("Sequences.Source", transformName.c_str());
          sequenceBrowserNode->AddSynchronizedSequenceNode(transformSequenceNode);
          transformSequenceNodes[transformName] = transformSequenceNode;
        }
        transformSequenceNodes[transformName]->SetDataNodeAtValue(transformNode, timestampSS.str());
      }
    }

  }
  sequenceBrowserNode->Modified();
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerIGSIOCommon::VolumeSequenceToTrackedFrameList(vtkMRMLSequenceNode* sequenceNode, vtkIGSIOTrackedFrameList* trackedFrameList)
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
  std::string trackName = sequenceNode->GetName();
  if (sequenceNode->GetNumberOfDataNodes() > 0)
  {
    trackName = sequenceNode->GetNthDataNode(0)->GetName();
  }
  if (trackedFrameList->SetCustomString(TRACKNAME_FIELD_NAME, trackName) == IGSIO_FAIL)
  {
    vtkErrorWithObjectMacro(sequenceNode, "Could not set track name!");
    return false;
  }


  int dimensions[3] = { 0, 0, 0 };
  vtkSmartPointer<vtkStreamingVolumeFrame> lastFrame = NULL;
  double timestamp = 0;
  double lastTimestamp = 0.0;
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

    igsioTransformName imageToPhysicalName;
    imageToPhysicalName.SetTransformName(trackName + "ToPhysical");

    std::stack<vtkSmartPointer<vtkStreamingVolumeFrame> > frameStack;
    frameStack.push(frame);
    if (!frame->IsKeyFrame())
    {
      vtkStreamingVolumeFrame* currentFrame = frame->GetPreviousFrame();
      while (currentFrame && currentFrame != lastFrame)
      {
        frameStack.push(currentFrame);
        currentFrame = currentFrame->GetPreviousFrame();
      }
    }
    lastFrame = frame;

    int initialStackSize = frameStack.size();
    while (frameStack.size() > 0)
    {
      vtkStreamingVolumeFrame* currentFrame = frameStack.top();
      igsioTrackedFrame trackedFrame;
      igsioVideoFrame videoFrame;
      videoFrame.SetEncodedFrame(currentFrame);
      trackedFrame.SetImageData(videoFrame);
      double currentTimestamp = lastTimestamp + (timestamp - lastTimestamp) * ((double)(initialStackSize - frameStack.size() + 1.0) / initialStackSize);
      trackedFrame.SetTimestamp(currentTimestamp);
      trackedFrame.SetFrameTransform(imageToPhysicalName, ijkToRASTransform);
      trackedFrame.SetFrameField(FRAME_STATUS_TRACKNAME, vtkVariant(frameStack.size() == 1 ? Frame_OK : Frame_Skip).ToString());
      trackedFrameList->AddTrackedFrame(&trackedFrame);
      frameStack.pop();
    }
    lastTimestamp = timestamp;
  }
  trackedFrameList->GetEncodingFourCC(codecFourCC);

  FrameSizeType frameSize = { 0, 0, 0 };
  for (int i = 0; i < 3; ++i)
  {
    frameSize[i] = (unsigned int)dimensions[i];
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerIGSIOCommon::SequenceBrowserToTrackedFrameList(vtkMRMLSequenceBrowserNode* inputSequenceBrowserNode,
  vtkIGSIOTrackedFrameList* outputTrackedFrameList)
{
  if (!inputSequenceBrowserNode)
  {
    LOG_ERROR("Invalid input sequence browser!");
  }

  if (!outputTrackedFrameList)
  {
    LOG_ERROR("Invalid output volume node!");
  }


  vtkMRMLSequenceNode* masterSequenceNode = inputSequenceBrowserNode->GetMasterSequenceNode();
  if (!masterSequenceNode || masterSequenceNode->GetNumberOfDataNodes() <= 0 || !masterSequenceNode->GetNthDataNode(0)->IsA("vtkMRMLVolumeNode"))
  {
    LOG_ERROR("Invalid master sequence node!");
  }
  int numberOfDataNodes = masterSequenceNode->GetNumberOfDataNodes();

  std::vector<vtkMRMLSequenceNode*> sequenceNodes;
  inputSequenceBrowserNode->GetSynchronizedSequenceNodes(sequenceNodes, true);
  for (int i = 0; i < masterSequenceNode->GetNumberOfDataNodes(); ++i)
  {
    igsioTrackedFrame emptyFrame;
    outputTrackedFrameList->AddTrackedFrame(&emptyFrame, vtkIGSIOTrackedFrameList::ADD_INVALID_FRAME);
  }

  int selectedItemNumber = inputSequenceBrowserNode->GetSelectedItemNumber();
  for (int i = 0; i < numberOfDataNodes; ++i)
  {
    igsioTrackedFrame* trackedFrame = outputTrackedFrameList->GetTrackedFrame(i);
    inputSequenceBrowserNode->SetSelectedItemNumber(i);

    for (vtkMRMLSequenceNode* sequenceNode : sequenceNodes)
    {
      vtkMRMLNode* proxyNode = inputSequenceBrowserNode->GetProxyNode(sequenceNode);
      vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(proxyNode);
      vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(proxyNode);

      if (volumeNode)
      {
        trackedFrame->GetImageData()->SetImageOrientation(US_IMG_ORIENT_MF); // TODO: save orientation and type
        //trackedFrame->GetImageData()->SetImageType(US_IMG_RGB_COLOR);
        trackedFrame->SetTimestamp(i);
        trackedFrame->GetImageData()->DeepCopyFrom(volumeNode->GetImageData());

        vtkNew<vtkTransform> imageToWorldTransform;
        imageToWorldTransform->Identity();
        imageToWorldTransform->PreMultiply();
        vtkNew<vtkMatrix4x4> ijkToRASMatrix;
        volumeNode->GetIJKToRASMatrix(ijkToRASMatrix);
        imageToWorldTransform->Concatenate(ijkToRASMatrix);

        vtkNew<vtkMatrix4x4> parentToWorldMatrix;
        vtkMRMLTransformNode* transformNode = volumeNode->GetParentTransformNode();
        if (transformNode)
        {
          transformNode->GetMatrixTransformToWorld(parentToWorldMatrix);
          imageToWorldTransform->Concatenate(parentToWorldMatrix);
        }

        vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
        imageToWorldTransform->GetMatrix(matrix);
        igsioTransformName transformName("ImageToWorld");
        trackedFrame->SetFrameTransform(transformName, matrix);
        trackedFrame->SetFrameTransformStatus(transformName, ToolStatus::TOOL_OK); //TODO: Attribute to status
      }

      if (transformNode)
      {
        vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
        transformNode->GetMatrixTransformToParent(matrix);

        igsioTransformName transformName(transformNode->GetName());
        trackedFrame->SetFrameTransform(transformName, matrix);
        trackedFrame->SetFrameTransformStatus(transformName, ToolStatus::TOOL_OK); //TODO: Attribute to status
      }
    }
  }
  inputSequenceBrowserNode->SetSelectedItemNumber(selectedItemNumber);

  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerIGSIOCommon::EncodeVideoSequence(vtkMRMLSequenceNode* inputSequenceNode, vtkMRMLSequenceNode* outputSequenceNode, int startIndex, int endIndex, std::string codecFourCC, std::map<std::string, std::string> codecParameters, bool forceReEncoding, bool minimalReEncoding, vtkCallbackCommand* progressCallback)
{
  if (!inputSequenceNode)
  {
    vtkErrorWithObjectMacro(inputSequenceNode, "Input is invalid vtkMRMLSequenceNode");
    return false;
  }

  if (!outputSequenceNode)
  {
    vtkErrorWithObjectMacro(outputSequenceNode, "Output is invalid vtkMRMLSequenceNode");
    return false;
  }

  int numberOfFrames = inputSequenceNode->GetNumberOfDataNodes();
  if (endIndex < 0)
  {
    endIndex = numberOfFrames - 1;
  }

  if (startIndex < 0 || startIndex >= numberOfFrames || startIndex > endIndex
    || endIndex >= numberOfFrames)
  {
    vtkErrorWithObjectMacro(inputSequenceNode, "Invalid start and end indices!");
    return false;
  }

  std::vector<FrameBlock> frameBlocks;

  if (!forceReEncoding)
  {
    FrameBlock currentFrameBlock;
    currentFrameBlock.StartFrame = 0;
    currentFrameBlock.EndFrame = 0;
    currentFrameBlock.ReEncodingRequired = false;
    vtkSmartPointer<vtkStreamingVolumeFrame> previousFrame = NULL;
    for (int i = startIndex; i <= endIndex; ++i)
    {
      // TODO: for now, only support sequences of vtkMRMLStreamingVolumeNode
      // In the future, this could be changed to allow all types of volume nodes to be encoded
      vtkMRMLVolumeNode* inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(inputSequenceNode->GetNthDataNode(i));
      if (!inputVolumeNode)
      {
        vtkErrorWithObjectMacro(inputSequenceNode, "Invalid data node at index " << i);
        return false;
      }

      vtkStreamingVolumeFrame* currentFrame = nullptr;
      vtkMRMLStreamingVolumeNode* inputStreamingVolumeNode = vtkMRMLStreamingVolumeNode::SafeDownCast(inputSequenceNode->GetNthDataNode(i));
      if (inputStreamingVolumeNode)
      {
        if (codecFourCC == "")
        {
          codecFourCC = inputStreamingVolumeNode->GetCodecFourCC();
        }
        currentFrame = inputStreamingVolumeNode->GetFrame();

        if (currentFrameBlock.ReEncodingRequired == false)
        {
          if (i == 0 && !inputStreamingVolumeNode->IsKeyFrame())
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
          else if (codecFourCC != inputStreamingVolumeNode->GetCodecFourCC())
          {
            currentFrameBlock.ReEncodingRequired = true;
          }
          else if (!minimalReEncoding && currentFrame && !currentFrame->IsKeyFrame() && previousFrame != currentFrame->GetPreviousFrame())
          {
            currentFrameBlock.ReEncodingRequired = true;
          }
        }
      }

      if (currentFrame && // Current frame exists
        previousFrame && // Current frame is not the initial frame
        currentFrame->IsKeyFrame() && // Current frame is a keyframe
        !previousFrame->IsKeyFrame()) // Previous frame was not also a keyframe
      {
        frameBlocks.push_back(currentFrameBlock);
        currentFrameBlock = FrameBlock();
        currentFrameBlock.StartFrame = i;
        currentFrameBlock.EndFrame = i;
        currentFrameBlock.ReEncodingRequired = false;
      }
      currentFrameBlock.EndFrame = i;
      previousFrame = currentFrame;

      /// TODO: If dimension changes, re-encode smaller images with padding
    }
    frameBlocks.push_back(currentFrameBlock);
  }
  else
  {
    FrameBlock totalFrameBlock;
    totalFrameBlock.StartFrame = startIndex;
    totalFrameBlock.EndFrame = endIndex;
    totalFrameBlock.ReEncodingRequired = true;
    frameBlocks.push_back(totalFrameBlock);
  }

  if (codecFourCC == "")
  {
    std::vector<std::string> codecFourCCs = vtkStreamingVolumeCodecFactory::GetInstance()->GetStreamingCodecFourCCs();
    if (codecFourCCs.empty())
    {
      vtkErrorWithObjectMacro(nullptr, "Re-encode failed! No codecs registered!");
      return false;
    }

    codecFourCC = codecFourCCs.front();
    vtkDebugWithObjectMacro(nullptr, "Streaming volume codec not specified! Using: " << codecFourCC);
  }

  double progress = 0.0;
  int framesProcessed = 0;

  std::vector<FrameBlock>::iterator frameBlockIt;
  for (frameBlockIt = frameBlocks.begin(); frameBlockIt != frameBlocks.end(); ++frameBlockIt)
  {
    if (frameBlockIt->ReEncodingRequired)
    {
      vtkNew<vtkMRMLStreamingVolumeNode> decodingProxyNode;
      vtkSmartPointer<vtkStreamingVolumeCodec> codec = vtkSmartPointer<vtkStreamingVolumeCodec>::Take(
        vtkStreamingVolumeCodecFactory::GetInstance()->CreateCodecByFourCC(codecFourCC));
      if (!codec)
      {
        vtkErrorWithObjectMacro(nullptr, "Could not find codec: " << codecFourCC);
        return false;
      }
      codec->SetParameters(codecParameters);

      for (int i = frameBlockIt->StartFrame; i <= frameBlockIt->EndFrame; ++i)
      {
        vtkMRMLVolumeNode* inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(inputSequenceNode->GetNthDataNode(i));
        if (!inputVolumeNode)
        {
          vtkErrorWithObjectMacro(inputSequenceNode, "Invalid data node at index " << i);
          return false;
        }

        vtkSmartPointer<vtkImageData> imageData = NULL;
        vtkMRMLStreamingVolumeNode* inputStreamingVolumeNode = vtkMRMLStreamingVolumeNode::SafeDownCast(inputVolumeNode);
        if (inputStreamingVolumeNode && inputStreamingVolumeNode->GetFrame())
        {
          decodingProxyNode->SetAndObserveFrame(inputStreamingVolumeNode->GetFrame());
          decodingProxyNode->DecodeFrame();
          imageData = decodingProxyNode->GetImageData();
        }
        else
        {
          imageData = inputVolumeNode->GetImageData();
        }

        vtkSmartPointer<vtkStreamingVolumeFrame> outputFrame = vtkSmartPointer<vtkStreamingVolumeFrame>::New();
        if (!codec->EncodeImageData(imageData, outputFrame))
        {
          vtkErrorWithObjectMacro(nullptr, "Error encoding frame!");
          return false;
        }

        vtkNew<vtkMatrix4x4> ijkToRASMatrix;
        inputVolumeNode->GetIJKToRASMatrix(ijkToRASMatrix);

        vtkNew<vtkMRMLStreamingVolumeNode> outputStreamingVolumeNode;
        outputStreamingVolumeNode->SetRASToIJKMatrix(ijkToRASMatrix);
        outputStreamingVolumeNode->SetAndObserveFrame(outputFrame);
        outputSequenceNode->SetDataNodeAtValue(outputStreamingVolumeNode, inputSequenceNode->GetNthIndexValue(i));

        ++framesProcessed;
        progress = (1.0 * framesProcessed) / numberOfFrames;
        if (progressCallback)
        {
          progressCallback->Execute(nullptr, vtkCommand::ProgressEvent, (void*)&progress);
        }
      }
    }
  }
  return true;
}
