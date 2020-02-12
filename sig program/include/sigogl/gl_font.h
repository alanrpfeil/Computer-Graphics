/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_FONT_H
# define GL_FONT_H

# include <sig/gs_font.h>

class GlResources;
class GlFontDecl;
class GlTexture;

//====================== GlFont =====================

/*! Stores data from a .fnt file */
class GlFont : public GsFont
 { private :   
	const GlTexture* _texture;

   private : // resource management information
	GlFontDecl* _decl;
	friend GlResources;

   public :
	GlFont ();
   ~GlFont ();
	bool texture_valid () const { return _texture!=0; }
	const GlTexture* texture() const { return _texture; }
	void texture ( const GlTexture* t ) { _texture=t; }
	static GlFont* default_font ();
	static void init_default_font_texture ();
};

//======================== End of File ============================

# endif // GL_FONT_H
