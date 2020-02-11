/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GSM_UI_EVENTS_H
# define GSM_UI_EVENTS_H

enum GsmMainEvent
	 { evFileOpen, evFileSave,
	   evFileSaveAll,
	   evFileExport,
	   evFileExportSkeleton,
	   evFileClose,
	   evFileOpenNumbered,
	   evFileOpenDir,
	   evFileCloseAll,

	   evEditNormalize,
	   evEditSmoothNormals,
	   evEditFlatNormals,
	   evEditInvertFaces,
	   evEditInvertNormals,
	   evEditValidate,

	   evManipEditTransform,
	   evManipApplyToModel,
	   
	   evViewFirst,
	   evViewNext,
	   evViewPrior,
	   
	   evJoinModels,
	   evAddPrimitive,
	   evEditPrimitive,

	   evInfoOpenGL,

	   evTabGroup,
	   evCullingToggle,
	   evNameInput,
	   evModelChange,
	   evViewChange,
	   evStatisticsToggle,
	};

enum GsmMtlTabEvent 
{	evBrowser, 
	evAmbient, evDiffuse, evSpecular, evEmission,
	evAmbientValue, evDiffuseValue, 
	evSpecularValue, evEmissionValue, evShininessValue,
	evMtlName, evUpdate, evClearAll, evSetOne, evAddOne
};

# endif // GSM_UI_EVENTS_H
