/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_buffer.h>
# include <sigogl/gl_font.h>
# include <sigogl/gl_texture.h>

//# define GS_USE_TRACE1 // basic trace
# include <sig/gs_trace.h>

# include <string.h>

//=================================== GlFont ====================================

GlFont::GlFont ()
{
	_texture = 0;
	_decl = 0;
};

GlFont::~GlFont ()
{
};

# include "gl_font18.inc"

static void Decode85 ( const unsigned char* src, unsigned char* dst );

GlFont* GlFont::default_font () // static
{ 
	GS_TRACE1 ( "default_font" );
	static GlFont* df=0;
	if ( df ) return df;

	GS_TRACE1 ( "Creating GlFont..." );
	df = new GlFont;
	ADOPT_STATIC_DATA(df);

	return df;
}
  
void GlFont::init_default_font_texture () // static
{
	GS_TRACE1 ( "init_default_font_texture" );
	GlFont* df = default_font ();
	if ( df->texture() ) return;

	GS_TRACE1 ( "Decoding png..." );
	GsBuffer<gsbyte> png ( PNG_ENCODED_SIZE/5*4 );
	Decode85 ( (const unsigned char*)png_data_base85, png.pt() );

	GS_TRACE1 ( "Creating bytemap..." );
	GsBytemap bm;
	bm.load_from_memory ( png.pt(), png.size() );
	bm.vertical_mirror();

	GS_TRACE1 ( "Declaring texture..." );
	GlTexture* tex = new GlTexture;
	tex->data ( &bm );
	df->texture ( tex );

	GS_TRACE1 ( "Done." );
};

static unsigned int Decode85Byte(char c) { return c >= '\\' ? c-36 : c-35; }

static void Decode85 ( const unsigned char* src, unsigned char* dst )
{
	while (*src)
	{	unsigned int tmp = Decode85Byte(src[0]) + 85*(Decode85Byte(src[1]) + 85*(Decode85Byte(src[2]) + 85*(Decode85Byte(src[3]) + 85*Decode85Byte(src[4]))));
		dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF); // We can't assume little-endianness.
		src += 5;
		dst += 4;
	}
}
