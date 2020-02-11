/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_array.h>
# include <sig/gs_image.h>
# include <sigogl/ui_dialogs.h>

# include "main.h"

# define STB_TRUETYPE_IMPLEMENTATION
# include "stb_truetype.h"

extern bool binary_to_compressed_c ( const char* filename, const char* symbol, bool use_base85_encoding, bool use_compression );

// Note: stb_rect_pack.h is not being used

bool makefont ( const GsArray<gsbyte>& ttfdata, int w, int h, int padding, int fontsize, int osamp, GsBytemap& bmap )
{
	if ( ttfdata.size()<0 ) return false;

	int firstchar=32;
	int numchars=95;
	int stride=0; // stride in bytes: distance from one row to the next, normally 1 with bilinear filtering
	bmap.init(w,h); // 1-channel bitmap that is w x h

	// Get font metrics:
	int ok;
	stbtt_fontinfo fontinfo;
	ok = stbtt_InitFont ( &fontinfo, ttfdata.pt(), 0 );
	if ( !ok ) { ui_message("Error in ttf data!"); return false; }
	int origascent, origdescent, origlinegap;
	stbtt_GetFontVMetrics ( &fontinfo, &origascent, &origdescent, &origlinegap );
	float height = (float)fontsize;
	float fontscale = stbtt_ScaleForPixelHeight ( &fontinfo, height );
	float ascent = float(origascent)*fontscale; // >0
	float descent = float(origdescent)*fontscale; // <0
	//gsout<<ascent<< ", "<<descent<<", "<< origlinegap<<gsnl;

	// Pack ttfdata in bytemap:
	stbtt_pack_context pc;
	ok = stbtt_PackBegin ( &pc, bmap.data(), w, h, stride, padding, NULL);
	if ( !ok ) { ui_message("Error initializing atlas!"); return false; }
	stbtt_PackSetOversampling ( &pc, osamp, osamp );
	static stbtt_packedchar chardata[256];
	ok = stbtt_PackFontRange ( &pc, ttfdata.pt(), 0, float(fontsize), firstchar, numchars, chardata+32 );
	if ( !ok ) { ui_message("Bytemap not large enought!"); return false; }
	stbtt_PackEnd(&pc);

	// Save bytemap texture as png:
	bmap.save("newfont.png");

	// Save bytemap texture as cpp declaration:
	ok = binary_to_compressed_c ( "newfont.png", "newfontpng", true, false ); // use_base85_encoding, use_compression
	if ( !ok ) { ui_message("Could not compress font!"); return false; }

	// Example of using function that shows how offset2 is to be used when drawing the font:
	float xpos=0, ypos=0;
	stbtt_aligned_quad quad;
	stbtt_GetPackedQuad ( chardata, w,h, 65, &xpos,&ypos, &quad, 1 );

	// Adjust values to GsFont format:
	if ( descent<0) descent*=-1;
	float top = ascent+3;
	float bottom = descent+1;

	// Adjustements if desired:
	if ( CurExFont==DroidSans_256x190_18_P1OS2 )
	{	ascent -= 2;
		top -= 2;
		chardata['_'].x0 += 1;
	}
	else if ( CurExFont==DroidSans_128x188_12_P1OS2 )
	{	ascent -= 2;
		top -= 3;
	}

	// Save character info in file:
	GsOutput o;
	ok = o.open("newfont.fnt");
	if ( !ok ) { ui_message("Could not create .fnt file!"); return false; }
	o<<"width "<<w<<gsnl;
	o<<"height "<<h<<gsnl;
	o<<"top "<<top<<gsnl;
	o<<"ascent "<<ascent<<gsnl;
	o<<"descent "<<descent<<gsnl;
	o<<"bottom "<<bottom<<gsnl;
	o<<"chars "<<numchars<<gsnl;
	int lastchar = firstchar+numchars-1;
	for ( int i=firstchar; i<=lastchar; i++ )
	{	o<<"ch "<<(char)(i==32?'-':char(i))<<gspc<<i;
		o<<" pos "<<chardata[i].x0 <<gspc<<chardata[i].y0<<gspc<<chardata[i].x1<<gspc<<chardata[i].y1;
		o<<" xoffset "<<chardata[i].xoff<<gspc<<chardata[i].xoff2;
		o<<" yoffset "<<chardata[i].yoff<<gspc<<chardata[i].yoff2;
		o<<" adv "<<chardata[i].xadvance;
		o<<gsnl;
	}
	o.close();

	// Save character info as cpp declaration:
	const char c = ',';
	const char* sc = ", ";
	ok = o.open("newfontfnt.cpp");
	if ( !ok ) { ui_message("Could not create .cpp file!"); return false; }
	o<<"# define ADOPT_STATIC_DATA(f) ";
	o<<"f->adopt_static_data ( fnt_chardata, "<<numchars<<sc<<w<<sc<<h<<sc<<top<<sc<<ascent<<sc<<descent<<sc<<bottom<<" );\n\n";
	o<<"static GsFont::Character fnt_chardata["<<numchars<<"] = {\n";
	for ( int i=firstchar; i<=lastchar; i++ )
	{	o<<"{"<< chardata[i].x0 <<c<<chardata[i].y0<<c<<chardata[i].x1<<c<<chardata[i].y1;
		o<<c<<gs_round(100.0f*chardata[i].xoff)<<c<<gs_round(100.0f*chardata[i].xoff2);
		o<<c<<gs_round(100.0f*chardata[i].yoff)<<c<<gs_round(100.0f*chardata[i].yoff2);
		o<<c<<chardata[i].xadvance<<'f';
		if (i<lastchar) o<<"},\n"; else o<<"}";
	}
	o<<" };\n";
	o.close();

   return true;
 }


/*
texture space: (x,y)---------
				 |
				 ------------

window space: (curpos)--+-------+
			yoff |      | GLYPH |
				 +-xoff-+--xadv-+
*/

/* useful links:
http://acodigo.blogspot.com/2016/04/dibujar-texto-en-opengl.html
https://github.com/raphm/makeglfont/blob/master/distance_map.cpp
*/

