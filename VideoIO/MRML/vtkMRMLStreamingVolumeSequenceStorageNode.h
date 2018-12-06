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

#ifndef __vtkMRMLStreamingVolumeSequenceStorageNode_h
#define __vtkMRMLStreamingVolumeSequenceStorageNode_h

#include "vtkSlicerVideoIOModuleMRMLExport.h"

#include "vtkMRMLStorageNode.h"
#include <string>

class vtkIGSIOTrackedFrameList;
class vtkGenericVideoReader;
class vtkGenericVideoWriter;
class vtkMRMLSequenceNode;

/// \ingroup Slicer_QtModules_Sequences
class VTK_SLICER_VIDEOIO_MODULE_MRML_EXPORT vtkMRMLStreamingVolumeSequenceStorageNode : public vtkMRMLStorageNode
{
public:

  static vtkMRMLStreamingVolumeSequenceStorageNode *New();
  vtkTypeMacro(vtkMRMLStreamingVolumeSequenceStorageNode,vtkMRMLStorageNode);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "VideoStorage";};

  /// Return true if the node can be read in.
  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);

  /// Return true if the node can be written by using thie writer.
  virtual bool CanWriteFromReferenceNode(vtkMRMLNode* refNode);

  /// Return a default file extension for writting
  virtual const char* GetDefaultWriteFileExtension();

  static bool ReadVideo(std::string fileName, vtkIGSIOTrackedFrameList* trackedFrameList);
  static bool WriteVideo(std::string fileName, vtkIGSIOTrackedFrameList* trackedFrameList);

  // FourCC code representing the codec that should be used to encode the video
  vtkSetMacro(CodecFourCC, std::string);
  vtkGetMacro(CodecFourCC, std::string);

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes(const char** atts) VTK_OVERRIDE;
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) VTK_OVERRIDE;
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node) VTK_OVERRIDE;
  /// Print the contents of the node
  virtual void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

protected:
  vtkMRMLStreamingVolumeSequenceStorageNode();
  ~vtkMRMLStreamingVolumeSequenceStorageNode();
  vtkMRMLStreamingVolumeSequenceStorageNode(const vtkMRMLStreamingVolumeSequenceStorageNode&);
  void operator=(const vtkMRMLStreamingVolumeSequenceStorageNode&);

  virtual int WriteDataInternal(vtkMRMLNode *refNode);

  /// Does the actual reading. Returns 1 on success, 0 otherwise.
  /// Returns 0 by default (read not supported).
  /// This implementation delegates most everything to the superclass
  /// but it has an early exit if the file to be read is incompatible.
  virtual int ReadDataInternal(vtkMRMLNode* refNode);

  /// Initialize all the supported write file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Update the supported compression presets
  virtual void UpdateCompressionPresets();

  std::string CodecFourCC;
};

#endif
