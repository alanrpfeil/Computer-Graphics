/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_SHAPE_H
# define SN_SHAPE_H

/** \file sn_shape.h 
 * shape base class
 */

# include <sig/gs_material.h>
# include <sig/gs_box.h>
# include <sig/sn_node.h>

class SnShape;
class GlContext; // forward declaration

//============================ SnShapeRenderer ================================

class SnShapeRenderer
{  public:
	virtual ~SnShapeRenderer() {} //!< Virtual Destructor
	virtual void init ( SnShape* shape )=0; //!< Renderer initialization (called once)
	static void instantiator_error ( const char* cname ); //!< Utility to print fatal error
};

# define SN_SHAPE_RENDERER_DECLARATIONS \
public: static SnShapeRenderer* (*renderer_instantiator)(); \
protected: virtual SnShapeRenderer* instantiate_renderer () const override; \

# define SN_SHAPE_RENDERER_DEFINITIONS(X) \
SnShapeRenderer* (*X::renderer_instantiator)() = 0; \
SnShapeRenderer* X::instantiate_renderer () const \
{	if (!renderer_instantiator) SnShapeRenderer::instantiator_error(class_name); \
	return renderer_instantiator ();  \
}

//================================ SnShape ====================================

enum gsRenderMode { gsRenderModeDefault,
					gsRenderModePhong,
					gsRenderModeGouraud,
					gsRenderModeFlat,
					gsRenderModeLines,
					gsRenderModePoints };

/*! \class SnShape sn_shape.h
	\brief general scene shape

	Base class for shape scene nodes. This class has to be derived for
	creating shape nodes.  */
class SnShape : public SnNode
{  public :
	enum ChangeType { Unchanged=0, RenderModeChanged=1, MaterialChanged=2, Changed=8 };
   protected :
	mutable gsbyte _changed; // 0:unchanged, otherwise flags: 1:render mode, 2:mtl, 4:resolution, 8:first time/full change
	mutable gscbool _auto_clear_data; // default is 0
	gscenum _render_mode;
	gscenum _overriden_render_mode; // -1 if not overriden
	gscbool _can_override_render_mode;
	gscbool _material_is_overriden;
	GsMaterial _material;
	GsMaterial _overriden_material;
	SnShapeRenderer* _renderer;

   protected :

	/*! Constructor requires the name of the derived class. */
	SnShape ( const char* class_name );

	/* Destructor only accessible through unref().*/
	virtual ~SnShape ();

   public :

	/*! Method allowing to replace the existing renderer. */
	void replace_renderer ( SnShapeRenderer* r ) { delete _renderer; _renderer=r; }

	/*! Returns the associated renderer. */
	SnShapeRenderer* renderer () const { return _renderer; }

	/*! By default the render mode is gsRenderModeDefault, that is specific to the
		shape type. For example a GsModel has as default smooth render mode, while
		GsLines has line mode as default. */
	gsRenderMode render_mode () const { return (gsRenderMode)_render_mode; }
	void render_mode ( gsRenderMode m );

	/*! Saves the current render mode and then replaces it by m. If
		can_override_render_mode() is false, then nothing is done. */
	void override_render_mode ( gsRenderMode m );

	/*! Restore the render mode that was used before overriding it. */
	void restore_render_mode ();

	/*! If false, the render action will not be able to override the render mode
		of this shape node. Default is true. */
	void can_override_render_mode ( bool b ) { _can_override_render_mode=(char)b; }
	bool can_override_render_mode () { return _can_override_render_mode==1; }

	/*! Saves the current material and then replaces it by m. */
	void override_material ( GsMaterial m );

	/*! Restore the material that was used before overriding it. */
	void restore_material ();

	/*! Returns true if the material is being overrriden, and false otherwise. */
	bool material_is_overriden () const { return _material_is_overriden==1; }

	/*! Sets the material to be used for the shape */
	void material ( const GsMaterial& m );
	const GsMaterial& material () const { return _material; }

	/*! Sets the diffuse color of the shape material. Note that when rendering
		shapes without lighting, only the diffuse color is used. */
	void color ( GsColor c );
	GsColor color () const { return _material.diffuse; }

	/*! Returns if the node parameters have changed, requiring
		the re-generation of the associated render lists. */
	gsbyte changed () const { return _changed; }

	/*! Sets the changed / unchanged state. If true, it will force the
		node to regenerate all its rendering data. */
	void changed ( bool b ) const { _changed = b? Changed:Unchanged; }

	/*! Sets the change flags at once. */
	void changed ( ChangeType c ) const { _changed=c; }

	/*! Equivalent to calling changed(Changed); */
	void touch () { _changed=Changed; }

	/*! If turned on all possible internal buffers are released after a render call,
		such that memory is saved but the geometry information is lost after 
		it is passed to the graphics hardware. If turned on the shape will
		render but functionality depending on the geometry will act like the
		shape is empty. Not all shapes implement this functionality. Default is off. */
	void auto_clear_data ( bool b ) { _auto_clear_data=b; }
	gscbool auto_clear_data () { return _auto_clear_data; }

	// Reminder: derived classes may also derive this SnNode method:
	// virtual void update_node () {}

	/*! Preparation method for rendering the shape. It will check visibility,
		initialize the renderer if needed, and return true if the shape indeed needs rendering */
	virtual bool prep_render ();

	/*! Method to be called after rendering. Here it sets internal changed state to Unchanged */
	virtual void post_render ();

	/*! Returns the bounding box of the shape. */
	virtual void get_bounding_box ( GsBox &box ) const=0;

   protected :

	/*! Calls a->shape_apply() for this node */
	virtual bool apply ( SaAction* a ) override;

	/*! Returns a renderer for this shape type. */
	virtual SnShapeRenderer* instantiate_renderer () const=0;
};

//================================ End of File ==========================================

# endif  // SN_SHAPE_H
