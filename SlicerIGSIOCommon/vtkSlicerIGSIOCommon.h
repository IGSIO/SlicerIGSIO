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

==============================================================================*/

#ifndef __vtkSlicerIGSIO_h
#define __vtkSlicerIGSIO_h

#if defined(WIN32) && !defined(vtkSlicerIGSIOCommon_STATIC)
#if defined(vtkSlicerIGSIOCommon_EXPORTS)
#define VTK_SLICERIGSIOCOMMON_EXPORT __declspec( dllexport ) 
#else
#define VTK_SLICERIGSIOCOMMON_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_SLICERIGSIOCOMMON_EXPORT
#endif

class vtkMRMLScene;
class vtkTrackedFrameList;
class vtkMRMLSequenceNode;
class vtkMRMLSequenceBrowserNode;
class vtkGenericVideoReader;
class vtkGenericVideoWriter;

#include <vtkSmartPointer.h>
#include <map>

/// \ingroup SlicerIGSIO_vtkSlicerIGSIO
/// Common utility functions for SlicerRT.
/// Note: The vtk prefix ensures python wrapping of the class that broke in VTK8.
class VTK_SLICERIGSIOCOMMON_EXPORT vtkSlicerIGSIOCommon
{
public:
  //----------------------------------------------------------------------------
  // Utility functions
  //----------------------------------------------------------------------------

  static bool TrackedFrameListToVolumeSequence(vtkTrackedFrameList* trackedFrameList, vtkMRMLSequenceNode* sequenceNode);

  static bool TrackedFrameListToSequenceBrowser(vtkTrackedFrameList* trackedFrameList, vtkMRMLSequenceBrowserNode* sequenceBrowserNode);

  static bool VolumeSequenceToTrackedFrameList(vtkMRMLSequenceNode* sequenceNode, vtkTrackedFrameList* trackedFrameList);
  
  static bool SequenceBrowserToTrackedFrameList(vtkMRMLSequenceBrowserNode* sequenceBrowserNode, vtkTrackedFrameList* trackedFrameList);

  static vtkSmartPointer<vtkGenericVideoReader> CreateVideoReader(std::string fileName);
  static vtkSmartPointer<vtkGenericVideoWriter> CreateVideoWriter(std::string fileName);

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

  // Python wrapped function for ReEncodeVideoSequence
  static bool ReEncodeVideoSequence(vtkMRMLSequenceNode* videoStreamSequenceNode,
    int startIndex = 0, int endIndex = -1, std::string codecFourCC = "") {
    return vtkSlicerIGSIOCommon::ReEncodeVideoSequence(videoStreamSequenceNode, startIndex, endIndex, codecFourCC, std::map<std::string, std::string>());
  }

  static bool ReEncodeVideoSequence(vtkMRMLSequenceNode* videoStreamSequenceNode,
    int startIndex, int endIndex,
    std::string codecFourCC,
    std::map<std::string, std::string> codecParameters);

};

#endif