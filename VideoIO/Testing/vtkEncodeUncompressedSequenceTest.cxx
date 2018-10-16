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

// std includes
#include <iostream>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtksys/CommandLineArguments.hxx>

//
#include <vtkMRMLSequenceBrowserNode.h>
#include <vtkMRMLSequenceNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStreamingVolumeNode.h>
#include <vtkStreamingVolumeCodecFactory.h>
#include <vtkSlicerIGSIOCommon.h>

//---------------------------------------------------------------------------
void SetTestingImageDataForValue(vtkImageData* image, unsigned char value)
{
  int dimensions[3] = { 0,0,0 };
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

//---------------------------------------------------------------------------
unsigned char GetTestingImageDataValue(vtkImageData* image)
{
  unsigned char value = 0;

  int dimensions[3] = { 0,0,0 };
  image->GetDimensions(dimensions);

  unsigned char* imageDataScalars = (unsigned char*)image->GetScalarPointer();
  value = imageDataScalars[2];
  return value;
}

//---------------------------------------------------------------------------
class vtkTempVolumeCodec : public vtkStreamingVolumeCodec
{
public:
  static vtkTempVolumeCodec *New();
  virtual vtkStreamingVolumeCodec* CreateCodecInstance();
  vtkTypeMacro(vtkTempVolumeCodec, vtkStreamingVolumeCodec);

  enum VideoFrameType
  {
    FrameKey,
    FrameInterpolated,
  };

  virtual std::string GetFourCC() { return "TEMP"; };

protected:
  vtkTempVolumeCodec() {};
  ~vtkTempVolumeCodec() {};

  virtual bool DecodeFrameInternal(vtkStreamingVolumeFrame* inputFrame, vtkImageData* outputImageData, bool saveDecodedImage = true)
  {
    if (!inputFrame->GetFrameData())
    {
      return false;
    }

    vtkUnsignedCharArray* frameData = inputFrame->GetFrameData();
    if (frameData->GetSize() != 1)
    {
      return false;
    }

    int dimensions[3] = { 0,0,0 };
    inputFrame->GetFrameDimensions(dimensions);

    unsigned char value = frameData->GetValue(0);
    outputImageData->SetDimensions(dimensions[0], dimensions[1], dimensions[0]);
    outputImageData->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    SetTestingImageDataForValue(outputImageData, value);

    return true;
  };

  virtual bool EncodeImageDataInternal(vtkImageData* inputImageData, vtkStreamingVolumeFrame* outputFrame, bool forceKeyFrame)
  {
    unsigned char* value = (unsigned char*)inputImageData->GetScalarPointer();
    unsigned char red = value[0];
    unsigned char green = value[1];
    unsigned char blue = value[2];

    vtkSmartPointer<vtkUnsignedCharArray> frameData = vtkSmartPointer<vtkUnsignedCharArray>::New();
    frameData->Allocate(1);
    frameData->SetValue(0, blue);

    outputFrame->SetPreviousFrame(NULL);
    outputFrame->SetFrameDimensions(inputImageData->GetDimensions());
    outputFrame->SetFrameData(frameData);
    outputFrame->SetCodecFourCC(this->GetFourCC());

    return true;
  };

private:
  vtkTempVolumeCodec(const vtkTempVolumeCodec&);
  void operator=(const vtkTempVolumeCodec&);
};

vtkCodecNewMacro(vtkTempVolumeCodec);

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  int width = 10;
  int height = 10;
  int numFrames = 25;

  vtkSmartPointer<vtkStreamingVolumeCodecFactory> factory = vtkStreamingVolumeCodecFactory::GetInstance();
  factory->RegisterStreamingCodec(vtkSmartPointer<vtkTempVolumeCodec>::New());
  
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

  std::string codecFourCC = "TEMP";
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
    vtkMRMLStreamingVolumeNode* streamingVolumeNode = vtkMRMLStreamingVolumeNode::SafeDownCast(sequenceNode->GetNthDataNode(i));
    vtkImageData* image = streamingVolumeNode->GetImageData();
    if (GetTestingImageDataValue(image) != GetTestingImageDataValue(images[i]))
    {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
