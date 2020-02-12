/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SA_EPS_EXPORT_H
# define SA_EPS_EXPORT_H

/** \file sa_eps_export.h 
 * Encapsulated Postscript Export
 */

# include <sig/sa_action.h>

/*! Export the scene graph to a .eps file.
	Note that not all nodes are supported, mainly only 2d shapes are supported*/
class SaEpsExport : public SaAction
{  public :
	typedef void (*export_function)(SnShape*,GsOutput&,const SaEpsExport*);

   private :
	struct RegData { const char* class_name;
					 export_function efunc;
				   };
	static GsArray<RegData> _efuncs;
	GsOutput& _output;
	float _page_width;
	float _page_height;
	float _page_margin;
	float _bbox_margin;
	float _scale_factor;
	GsVec2 _translation;
	
   public :
	/*! Constructor */
	SaEpsExport ( GsOutput& o );

	/*! Virtual destructor. */
	virtual ~SaEpsExport ();

	float scale_factor () const { return _scale_factor; }

	/*! Set print page dimensions in cms. Default is w=21.59 and h=27.94 */
	void set_page ( float w, float h ) { _page_width=w; _page_height=h; }

	/*! Set the page margin in cms to use, default is 4cms */
	void set_page_margin ( float m ) { _page_margin=m; }

	/*! The eps bounding box might need to be increased to ensure that the
		style of drawn shapes will not generate drawings outside the bouding
		box limits. The default value is 0.1 cms. */
	void set_bbox_margin ( float m ) { _bbox_margin=m; }

	/*! Registration is kept in a static array, shared by all instances
		of SaEpsExport. Currently only GsLine export is available.
		Registration must be explicitly called for user-defined shapes. 
		In case a name already registered is registered again, the new
		one replaces the old one. */
	friend void register_export_function ( const char* class_name, export_function efunc );

	/*! Makes the node to start applying the action. If the action is not
		applied to the entire scene, false is returned. */
	bool apply ( SnNode* n );

   private :
	virtual void mult_matrix ( const GsMat& mat ) override;
	virtual void push_matrix () override;
	virtual void pop_matrix () override;
	virtual bool shape_apply ( SnShape* s ) override;
};

//================================ End of File =================================================

# endif  // SA_EPS_EXPORT_H

