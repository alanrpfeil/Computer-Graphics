/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SA_MODEL_EXPORT_H
# define SA_MODEL_EXPORT_H

/** \file sa_model_export.h 
 * Export all models in global coordinates
 */

# include <sig/sa_action.h>
# include <sig/gs_string.h>

/*! Export all models in the scene graph in global coordinates
	to several .gsm files. */
class SaModelExport : public SaAction
 { private :
	GsString _dir;
	GsString _prefix;
	int _num;
   public :
	/*! Constructor */
	SaModelExport ( const GsString& dir );

	/*! Virtual destructor. */
	virtual ~SaModelExport ();

	/*! Change the directory to save files */
	void directory ( const GsString& dir );

	/*! Change the prefix name of the files */
	void prefix ( const GsString& pref ) { _prefix=pref; }

	/*! Start applying the action. If the action is not
		applied to the entire scene, false is returned. */
	bool apply ( SnNode* n );

   private :
	virtual bool shape_apply ( SnShape* s ) override;
};

//================================ End of File =================================================

# endif  // SA_MODEL_EXPORT_H

