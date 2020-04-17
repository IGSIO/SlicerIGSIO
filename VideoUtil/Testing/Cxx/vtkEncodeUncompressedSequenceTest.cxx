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

// std includes
#include <iostream>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtksys/CommandLineArguments.hxx>

// Sequences includes
#include <vtkMRMLSequenceBrowserNode.h>
#include <vtkMRMLSequenceNode.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLStreamingVolumeNode.h>

// vtkAddon includes
#include <vtkStreamingVolumeCodecFactory.h>

// SlicerIGSIOCommon includes
#include <vtkSlicerIGSIOCommon.h>

// SequenceIO includes
#include <vtkSlicerSequenceIOLogic.h>

//---------------------------------------------------------------------------
void SetTestingImageDataForValue(vtkImageData* image, unsigned char value)
{
  int dimensions[3] = { 0, 0, 0 };
  image->GetDimensions(dimensions);

  unsigned char* imageDataScalars = (unsigned char*)image->GetScalarPointer();
  for (int y = 0; y < dimensions[1]; ++y)
  {
    for (int x = 0; x < dimensions[0]; ++x)
    {
      unsigned char red = 255 * (x / (double)dimensions[0]);
      unsigned char green = 255 * (y / (double)dimensions[1]);
      imageDataScalars[0] = red;
      imageDataScalars[1] = green;
      imageDataScalars[2] = value;
      imageDataScalars += 3;
    }
  }
}

//----------------------------------------------------------------------------
int vtkEncodeUncompressedSequenceTest(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  int width = 10;
  int height = 10;
  int numFrames = 25;

  vtkSmartPointer<vtkStreamingVolumeCodecFactory> factory = vtkStreamingVolumeCodecFactory::GetInstance(); 

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSequenceNode> sequenceNode;
  scene->AddNode(sequenceNode);

  std::vector<vtkSmartPointer<vtkImageData>> images;

  for (int i = 0; i < numFrames; ++i)
  {
    vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
    imageData->SetDimensions(width, height, 1);
    imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    unsigned char value = 255 * (i / (double)numFrames);
    SetTestingImageDataForValue(imageData, value);
    images.push_back(imageData);

    vtkSmartPointer<vtkMRMLStreamingVolumeNode> streamingVolumeNode = vtkSmartPointer<vtkMRMLStreamingVolumeNode>::New();
    streamingVolumeNode->SetAndObserveImageData(imageData);
    if (streamingVolumeNode->GetFrame())
    {
      return EXIT_FAILURE;
    }

    std::stringstream indexValue;
    indexValue << i;
    sequenceNode->SetDataNodeAtValue(streamingVolumeNode, indexValue.str());
  }

  std::string codecFourCC = "RV24";
  if (!vtkSlicerIGSIOCommon::ReEncodeVideoSequence(sequenceNode.GetPointer(), 0, -1, codecFourCC))
  {
    return EXIT_FAILURE;
  }

  for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); ++i)
  {
    vtkMRMLStreamingVolumeNode* streamingVolumeNode = vtkMRMLStreamingVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(i));
    if (!streamingVolumeNode->GetFrame())
    {
      return EXIT_FAILURE;
    }
  }

  for (int i = 0; i < sequenceNode->GetNumberOfDataNodes(); ++i)
  {
    vtkMRMLStreamingVolumeNode* inputStreamingVolumeNode = vtkMRMLStreamingVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(i));

    vtkSmartPointer<vtkMRMLStreamingVolumeNode> outputStreamingVolumeNode = vtkSmartPointer<vtkMRMLStreamingVolumeNode>::New();
    outputStreamingVolumeNode->SetAndObserveFrame(inputStreamingVolumeNode->GetFrame());

    vtkImageData* inputImage = images[i];
    vtkImageData* outputImage = outputStreamingVolumeNode->GetImageData();
    if (!inputImage || !outputImage)
    {
      return EXIT_FAILURE;
    }

    unsigned char* inputImagePointer = (unsigned char*)inputImage->GetScalarPointer();
    unsigned char* outputImagePointer = (unsigned char*)outputImage->GetScalarPointer();
    for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
      {
        for (int c = 0; c < 3; c++)
        {
          if (*inputImagePointer != *outputImagePointer)
          {
            return EXIT_FAILURE;
          }
          ++inputImagePointer;
          ++outputImagePointer;
        }
      }
    }
  }

  return EXIT_SUCCESS;
}
