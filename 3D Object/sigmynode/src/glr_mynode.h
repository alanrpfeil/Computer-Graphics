# pragma once

# include <sigogl/glr_base.h>
# include <sigogl/gl_objects.h>

/*! \class GlrLines glr_lines.h
	Renderer for SnLines */
class GlrMyNode : public GlrBase
 { protected :
	GlObjects _glo; // indices for opengl vertex arrays and buffers
	gsuint _psize;
   public :
	GlrMyNode ();
	virtual ~GlrMyNode ();
	virtual void init ( SnShape* s ) override;
	virtual void render ( SnShape* s, GlContext* c ) override;
};

//================================ End of File =================================================

