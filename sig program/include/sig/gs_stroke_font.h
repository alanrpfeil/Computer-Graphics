/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_STROKE_FONT_H
# define GS_STROKE_FONT_H

/** \file gs_stroke_font.h 
 * vectorized simplex font data
 * (utility functionality - will only be linked if included in your program)
 */

# include <sig/gs.h>
# include <sig/gs_vec2.h>
# include <sig/sn_lines2.h>

/*! \class GsStrokeFont gs_stroke_font.h
	\brief vectorized simplex font data
	Size and spacing are encoded as follows:
		 | MaxS |
	   --+----+-+ 
		 | CC |   MaxY (>0) (height from base)
	base +-c--+   MaxH=MaxY-Miny
		 |	|   MinY (<0)
	   --+----+-+ 
		 |MaxW|  
	Using this this class will make the linker to place the 
	static data in your executable, an increase of about 5K */
class GsStrokeFont
{  public :
	/* Character description:
	   n: number of vertices to describe the character,
	   s: horizontal spacing of the character,
	   i: vertex data starting index. */
	struct ChDesc { gsbyte n, s; gsuint16 i; };

	/*! Character info for the simplex font. */
	static const ChDesc SimplexChar[95];

	/*! Vertex info for the simplex font, a (-1,-1) means a pen-up. */
	static const gschar SimplexData[2388];

	/*! Non-scaled simplex font info:
		MaxS: maximum horizontal spacing among all characters,
		MaxW: maximum character width,
		MinY: minimum character Y coordinate with respect to base,
		MaxY: maximum character Y coordinate with respect to base,
		MaxH: maximum character height. */
	static int MaxS, MaxW, MinY, MaxY, MaxH;

   public :

	static float scaling ( float charh ) { return charh/float(MaxH); }

	struct Output
	 { virtual void begin ()=0;
	   virtual void end ()=0;
	   virtual void add ( float x, float y )=0;
	 };

	static void draw ( const char* text, const GsPnt2& p, Output& out, float charh, float wperc=100, float linespc=0, float charspc=0, bool wcoords=false, float* xmax=0 );

   private:

	/*! GsStrokeFont should not be instantiated by the user, all its members are static. */
	GsStrokeFont () { }
};

/*! Add polylines to the internal arrays in order to draw the given text */
void push_text ( SnLines2* l, const char* text, const GsPnt2& p, GsStrokeFont::Output& out, float charh, float wperc=100, float linespc=0, float charspc=0, bool wcoords=false, float* xmax=0 );

/*! Add polylines to the internal arrays in order to draw the given text in window coordinates */
void push_text_wc ( SnLines2* l, const char* text, const GsPnt2& p, GsStrokeFont::Output& out, float charh, float wperc=100, float linespc=0, float charspc=0, float* xmax=0 );

//=============================== End of File =================================

# endif  // GS_STROKE_FONT_H
