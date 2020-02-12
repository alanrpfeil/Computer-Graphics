/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/gl_core.h>
# include <sigogl/gl_texture.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

# include <sig/gs_dirs.h>
# include <sig/gs_image.h>
# include <sig/sn_model.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
//# define GS_USE_TRACE3 // Model Details
//# define GS_USE_TRACE4 // Render type
# include <sig/gs_trace.h>

//======================================= SnModel ====================================

const char* SnModel::class_name = "SnModel";
SN_SHAPE_RENDERER_DEFINITIONS(SnModel);

/*ImprNote: to consider:
	- 3dtextured shaders should be split in phong/gouraud; review notes.txt
	- design way to be able to share geometry but not materials, maybe rendering
	  context could store: CurMaterial, or CurMaterialGroup, or an optional 
	  SnAppearance could be defined before a shape to provide info during rendering.
	- revise if SnModel could have its own GsCfg, or if better rely/extend SnTransform
	- consider option to order transparent faces from point of view for perfect transparency
	- load_obj() could recover additional parameters such as group and material names
	- revisit auto_clear_data functionality:
	  renderer needs only access to G information when calling lists
	  consider impact on SnPrimitive, primitives need V, N, F, Fn
*/

SnModel::SnModel ( const char* classname ) : SnShape ( classname )
{
	GS_TRACE1 ( "Protected Constructor" );
	_model = new GsModel;
	_model->ref();
}

SnModel::SnModel ( GsModel* m ) : SnShape ( class_name )
{
	GS_TRACE1 ( "Constructor" );
	_model = m? m : new GsModel;
	_model->ref();
}

SnModel::~SnModel ()
{
	GS_TRACE1 ( "Destructor" );
	_model->unref();
}

void SnModel::model ( GsModel* m )
{
	if ( _model==m ) return;
	_model->unref();
	_model = m? m : new GsModel;
	_model->ref();
	touch ();
}

void SnModel::get_bounding_box ( GsBox& b ) const
{
	_model->get_bounding_box(b);
}

/*Notes:
  - MultiDrawArrays() requires indices and is not faster than DrawArrays() multiple times
  - glPolygonMode remains in version 4.5: opengl.org/sdk/docs/man4/html/glPolygonMode.xhtml
*/

//================================ EOF =================================================
