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

#ifndef __qSlicerVideoReader_h
#define __qSlicerVideoReader_h

// SlicerQt includes
#include "qSlicerFileReader.h"
class qSlicerVideoReaderPrivate;

// Slicer includes
class vtkSlicerVideoIOLogic;

//-----------------------------------------------------------------------------
class qSlicerVideoReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerVideoReader(vtkSlicerVideoIOLogic* logic, QObject* parent = 0 );
  virtual ~qSlicerVideoReader();

  virtual QString description() const;
  virtual IOFileType fileType() const;
  virtual QStringList extensions() const;

  virtual bool load( const IOProperties& properties );
  
  void setVideoIOLogic(vtkSlicerVideoIOLogic* newVideoIOLogic);
  vtkSlicerVideoIOLogic* VideoIOLogic() const;

protected:
  QScopedPointer< qSlicerVideoReaderPrivate > d_ptr;

private:
  Q_DECLARE_PRIVATE( qSlicerVideoReader );
  Q_DISABLE_COPY( qSlicerVideoReader );
};

#endif
