/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_image.h>
# include <sig/gs_buffer.h>

//# define GS_USE_TRACE1  // constructor / desctructor
//# define GS_USE_TRACE2  // load 
# include <sig/gs_trace.h>

# define STB_IMAGE_IMPLEMENTATION
# define STBI_NO_PSD
# define STBI_NO_PIC
# define STBI_NO_GIF
# define STBI_NO_PNM
# define STBI_NO_HDR
# include "stb_image.h"

# define STB_IMAGE_WRITE_IMPLEMENTATION
//# define STB_IMAGE_WRITE_DEFINE_BMP // custom define: bmp loader not used
# define STB_IMAGE_WRITE_DEFINE_TGA // custom define
# define STB_IMAGE_WRITE_DEFINE_PNG // custom define
# include "stb_image_write.h"

//=========================== GsImageBase ============================

GsImageBase::GsImageBase ()
 {
   GS_TRACE1 ("Default Constructor");
   _w = _h = 0;
   _img = 0;
 }

GsImageBase::~GsImageBase ()
 {
   GS_TRACE1 ("Destructor");
   if (_img) stbi_image_free ( _img );
 }

GsImageBase::GsImageBase ( int w, int h, unsigned sizeofx )
 {
   GS_TRACE1 ("Constructor");
   _w = _h = 0;
   _img = 0;
   if ( w>0 && h>0 )
	{ _w = w; 
	  _h = h;
	  _img = (gsbyte*) stbi__malloc(w*h*sizeofx);
	}
 }

void GsImageBase::init ( int w, int h, unsigned sizeofx )
 {
   if ( w==_w && h==_h ) return;

   if ( _img ) stbi_image_free ( (void*)_img );

   if ( w<=0 || h<=0 )
	{ _w = _h = 0; 
	  _img = 0;
	}
   else
	{ _w = w; 
	  _h = h;
	  _img = stbi__malloc(w*h*sizeofx);
	}
 }

void GsImageBase::vertical_mirror ( unsigned sizeofx )
 {
   int i, ie, mid;
   mid = _h/2;
 
   GsBuffer<gsbyte> linebuf(_w*sizeofx);
   gsbyte* buf = &linebuf[0];
   int lsize = _w*sizeofx;

   # define LINE(l) ((gsbyte*)_img)+(l*_w*sizeofx)
   for ( i=0,ie=_h-1; i<mid; i++,ie-- )
	{ memcpy ( buf,	  LINE(i),  lsize );
	  memcpy ( LINE(i),  LINE(ie), lsize );
	  memcpy ( LINE(ie), buf,	  lsize );
	}
   # undef LINE
 }

bool GsImageBase::save ( const char* filename,  unsigned sizeofx )
{
	if ( !filename ) return false;
	if ( !filename[0] ) return false;
	switch ( filename[strlen(filename)-1] )
	{	case 'a': return stbi_write_tga ( filename, _w, _h, sizeofx, _img )==1;
		case 'p': break; // save bmp below, not working: stbi_write_bmp ( filename, _w, _h, sizeofx, _img )
		default: return stbi_write_png ( filename, _w, _h, sizeofx, _img, 0 )==1;
	}

	// My original direct bmp implementation (might be faster than stbi_write_bmp):
	# define PutInt(i)   fwrite(&i,4,1,f) // 4 bytes, 1 item
	# define PutShort(s) fwrite(&s,2,1,f) // 2 bytes, 1 item

	if ( sizeofx!=4 ) gsout.fatal("GsImage can only save bmp in rgba format");

	FILE* f = fopen ( filename, "wb" );
	if ( !f ) return false;

	int i = 0;
	int offset = 14+40;
	//int dw = 4-(_w%4); if ( dw==4 ) dw=0;
	int dw = (_w%4);

	int filesize = 14 + 40 + (_w*_h*3) +(_h*dw); // 14bytes: header, 40: info

	// 14 bytes of header:
	fprintf ( f, "BM" ); // 2 bytes : signature
	PutInt ( filesize ); // file size
	PutInt ( i );		// reserved (zeros)
	PutInt ( offset );   // offset to the data

	// 40 bytes of info header:
	int infosize = 40;
	short planes = 1;
	short bits = 24;
	int compression = 0; // no compression
	int compsize = 0;	// no compression
	int hres = 600;
	int vres = 600;
	int colors = 0;
	int impcolors = 0;   // important colors: all
	PutInt ( infosize ); // size of info header
	PutInt ( _w );	   // width
	PutInt ( _h );	   // height
	PutShort ( planes );
	PutShort ( bits );
	PutInt ( compression );
	PutInt ( compsize );
	PutInt ( hres );
	PutInt ( vres );
	PutInt ( colors );
	PutInt ( impcolors );

	int w, h;
	GsColor* pixel;
	for ( h=_h-1; h>=0; h-- )
	{	pixel = ((GsColor*)_img)+(h*_w); // line(h)

		for ( w=0; w<_w; w++ )
		{	fputc ( pixel->b, f ); // B
			fputc ( pixel->g, f ); // G
			fputc ( pixel->r, f ); // R
			pixel++;
		}
		for ( w=0; w<dw; w++ ) fputc ( 0, f );
	}

	fclose ( f );
	return true;
}

bool GsImageBase::load ( const char* filename, unsigned sizeofx )
{
	GS_TRACE2 ( "Loading "<<filename<<":" );

	init(0,0,sizeofx);

	int w, h, d; // d = number of 8-bit components per pixel
	_img = stbi_load ( filename, &w, &h, &d, sizeofx ); // force sizeofx components per pixel

	GS_TRACE2 ( "depth:" << d << ", size:" << w << "x" << h << ", bitsperpixel:"<<d );

	if ( !_img ) 
	{	GS_TRACE2("Error!"); return false; }
	else
	{	GS_TRACE2("Loaded."); _w=w; _h=h; return true; }
}

bool GsImageBase::load_from_memory ( const gsbyte* buffer, unsigned buflen, unsigned sizeofx )
 {
   GS_TRACE2 ( "Loading from Memory: "<<buflen<<" bytes" );

   init(0,0,sizeofx);

   int w, h, d; // d = number of 8-bit components per pixel
   _img = stbi_load_from_memory ( buffer, buflen, &w, &h, &d, sizeofx );

   GS_TRACE2 ( "depth:" << d << ", size:" << w << "x" << h << ", bitsperpixel:"<<d );

   if ( !_img ) 
	{ GS_TRACE2("Error!"); return false; }
   else
	{ GS_TRACE2("Loaded."); _w=w; _h=h; return true; }
 }
