/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerMetafileIOOptionsWidget_h
#define __qSlicerMetafileIOOptionsWidget_h

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerIOOptionsWidget.h"

class qSlicerMetafileIOOptionsWidgetPrivate;

/// \ingroup Slicer_QtModules_MetafileImporter
class qSlicerMetafileIOOptionsWidget :
	public qSlicerIOOptionsWidget
{
	Q_OBJECT
public:
	qSlicerMetafileIOOptionsWidget(QWidget* parent = nullptr);
	~qSlicerMetafileIOOptionsWidget() override;

protected slots:
	void updateProperties();

private:
	Q_DECLARE_PRIVATE_D(qGetPtrHelper(qSlicerIOOptions::d_ptr), qSlicerMetafileIOOptionsWidget);
	Q_DISABLE_COPY(qSlicerMetafileIOOptionsWidget);
};

#endif
