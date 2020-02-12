/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution.
  =======================================================================*/

# include <sig/gs_font.h>
# include <sig/gs_output.h>
# include <sig/gs_string.h>
# include <sig/sn_planar_objects.h>

//# define GS_USE_TRACE1 // basic trace
//# define GS_USE_TRACE2 // more trace
# include <sig/gs_trace.h>

//=================================== GsFont ====================================

GsFont::GsFont()
{
	_chars = 0;
	_nchars = 0;
	_allocateddata = 0;
	_imgw = _imgh = 0;
	_top = _ascent = _descent = _bottom = 0;
};

GsFont::~GsFont()
{
	if (_allocateddata) delete[] _chars;
}

bool GsFont::load ( const char* filename )
{
	GS_TRACE1("Loading [" << filename << "]...");

	GsInput in(fopen(filename, "rt"));
	if (!in.valid()) return false;

	// Load header:
	int nchars = 0;
	while (!in.end())
	{
		in.get();
		if (in.ltoken() == "top") { _top = in.getf(); }
		else if (in.ltoken() == "ascent") { _ascent = in.getf(); }
		else if (in.ltoken() == "descent") { _descent = in.getf(); }
		else if (in.ltoken() == "bottom") { _bottom = in.getf(); }
		else if (in.ltoken() == "width") { _imgw = in.geti(); }
		else if (in.ltoken() == "height") { _imgh = in.geti(); }
		else if (in.ltoken() == "chars") { nchars = in.geti(); break; }
	}
	if ( nchars==0 ) { gsout.putf("Keyword count not defined in [%s]!", filename); return false; }

	// Allocate space for character data:
	if (_allocateddata) delete[] _chars;
	_chars = new Character[nchars];
	_nchars = nchars;
	_allocateddata = 1;

	// Load info per character:
	int i = 0, c;
	while (!in.end())
	{
		in.get();
		if ( in.ltoken()[0]=='c' ) // ch
		{	in.get(); c = in.geti(); GS_CLIP(c, 32, nchars + 31); i = c - 32;
		}
		else if ( in.ltoken()[0]=='p' ) // pos
		{	_chars[i].x0 = in.geti();
			_chars[i].y0 = in.geti();
			_chars[i].x1 = in.geti();
			_chars[i].y1 = in.geti();
		}
		else if ( in.ltoken()[0]=='y' ) // yoffset
		{	_chars[i].yoffset1 = gs_round(100.0f*in.getf());
			_chars[i].yoffset2 = gs_round(100.0f*in.getf());
		}
		else if ( in.ltoken()[0]=='x' ) // xoffset
		{	_chars[i].xoffset1 = gs_round(100.0f*in.getf());
			_chars[i].xoffset2 = gs_round(100.0f*in.getf());
		}
		else if ( in.ltoken()[0]=='a' ) // xadvance
		{	_chars[i].xadvance = in.getf();
		}
	}

	GS_TRACE1("Loaded.");
	return true;
}

void GsFont::adopt_static_data ( Character* cdata, int nchars, int imgw, int imgh, float t, float a, float d, float b )
{
	if (_allocateddata) delete[] _chars;
	_allocateddata = 0;
	_chars = cdata;
	_nchars = nchars;
	_imgw = imgw;
	_imgh = imgh;
	_top = t;
	_ascent = a;
	_descent = d;
	_bottom = b;
}

float GsFont::text_width ( const GsFontStyle& fs, const char* text, int n ) const
{
	float w = 0;
	if ( !text ) return w;
	if ( n>=0 ) n--; // n becomes max char
	float s = wscaling(fs);
	const float charspc = float(fs.charspc); // horizontal spacing
	for ( int c=0; text[c]; c++ )
	{	if ( c==n ) break;
		w += s*xadvance(text[c]) + charspc; // horizontal spacing
	}
	return w;
}

GsVec2 GsFont::text_size ( const GsFontStyle& fs, const char* text, float extralinespc ) const
{
	GS_TRACE1("GsFont::text_size [" << text << ']');
	GsVec2 size;
	if (!text) return size;

	GsVec2 s = scaling(fs);

	const float charspc = float(fs.charspc); // horizontal spacing
	float linew = 0;
	int numlines = 1;
	for ( const char *c=text; *c; c++ )
	{	if (*c == '\n')
		{	numlines++;
			if ( linew>size.x ) size.x=linew;
			linew = 0;
			continue;
		}
		linew += float(xadvance(*c))*s.x + charspc;
	}

	if (linew > size.x) size.x = linew;
	size.y = (float(height())*s.y + extralinespc) * numlines;

	GS_TRACE1("Size: "<<size);
	return size;
}

static inline void out ( SnPlanarObjects* pobs, float x, float y, const GsFont::Character& c, const GsVec2& so, 
						 GsColor color, const float iw, const float ih, const GsFont::Character* uc=0 )
{
	if ( !uc ) uc = &c;
	const float xoffset1 = so.x*int(uc->xoffset1);
	const float xoffset2 = so.x*int(uc->xoffset2);
	const float yoffset1 = so.y*int(c.yoffset1);
	const float yoffset2 = so.y*int(c.yoffset2);
	GsRect cr ( x+xoffset1, y-yoffset2, xoffset2-xoffset1, yoffset2-yoffset1 );

	pobs->set_zero_index();
	pobs->push_rect(cr, color); // points will be in: bl, br, tr, tl 

	const float s0 = float(c.x0) / iw;		  // (s0,t0):bl, (s1,t1):tr in window coordinates
	const float t0 = 1.0f - float(c.y1) / ih; // image coordinates->texture coordinates
	const float s1 = float(c.x1) / iw;
	const float t1 = 1.0f - float(c.y0) / ih;
	pobs->setT ( 0, GsPnt2(s0,t0), GsPnt2(s1,t0), GsPnt2(s1,t1), GsPnt2(s0,t1) ); // order: bl, br, tr, tl
}

float GsFont::output ( const GsFontStyle& fs, const char* text, float x, float y, GsColor color,
					   SnPlanarObjects* pobs, float extralinespc, const float* xmax, char cs, int accel, GsColor* e3c ) const
{
	GsVec2 s = scaling(fs);
	if (cs!='s') s.y*=-1;
	GsVec2 so = s / 100.0f;

	GS_TRACE1("Output font scaling: " << s << "...");

    float origx=x;
	const float charspc = float(fs.charspc); // horizontal spacing
	const float iw = (float)imgw();
	const float ih = (float)imgh();

	for ( const char* ch=text; *ch; ch++ )
	{
        const char safec = *ch>=32? *ch:32;
		const Character& c = character(safec);
        const float nx = x + s.x*c.xadvance + charspc; // compute next x position

        if ( xmax && nx>*xmax ) // past limit, skip to new line or return
		{	while ( *ch && *ch!='\n' ) ch++;
			if ( *ch==0 ) break; // done
		}
		if ( *ch=='\n' ) { x=origx; y-=s.y*(height()+extralinespc); continue; } // skip line
		if ( *ch==' ' ) { x=nx; continue; } // no need to draw

		out ( pobs, x, y, c, so, color, iw, ih );
		if (e3c) { pobs->C.top(2)=e3c[0]; pobs->C.top(1)=e3c[1]; pobs->C.top()=e3c[2]; }

		if ( accel--==0 ) // draw underline character
		{	out ( pobs, x, y, character('_'), so, color, iw, ih, &c );
		}

		x=nx; // goto next x position
	}
	return x;
}

//======================================= EOF ========================================
