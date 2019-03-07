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

#ifndef __qSlicerSequenceIOReader_h
#define __qSlicerSequenceIOReader_h

// SlicerQt includes
#include "qSlicerFileReader.h"
class qSlicerSequenceIOReaderPrivate;

// Slicer includes
class vtkSlicerSequenceIOLogic;

//-----------------------------------------------------------------------------
class qSlicerSequenceIOReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerSequenceIOReader(vtkSlicerSequenceIOLogic* logic, QObject* parent = 0 );
  virtual ~qSlicerSequenceIOReader();

  virtual QString description() const;
  virtual IOFileType fileType() const;
  virtual QStringList extensions() const;

  virtual bool load( const IOProperties& properties );

  void setSequenceIOLogic(vtkSlicerSequenceIOLogic* newSequenceIOLogic);
  vtkSlicerSequenceIOLogic* SequenceIOLogic() const;

protected:
  QScopedPointer< qSlicerSequenceIOReaderPrivate > d_ptr;

private:
  Q_DECLARE_PRIVATE( qSlicerSequenceIOReader );
  Q_DISABLE_COPY( qSlicerSequenceIOReader );
};

#endif
