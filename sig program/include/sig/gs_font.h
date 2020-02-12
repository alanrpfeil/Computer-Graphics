/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_FONT_H
# define GS_FONT_H

/** \file gs_font.h 
 * font classes and vectorized simplex font data
 */

# include <sig/gs.h>
# include <sig/gs_vec2.h>
# include <sig/gs_color.h>

class SnPlanarObjects;

//====================== GsFontStyle =====================

/*! \class GsFontStyle gs_font.h
	\brief Basic parameters specifying font type and dimensions.
	GsFontStyle specifies font dimensions in pixels. Application-specific mechanisms
	should then adjust values according to a specific display PPI. */
class GsFontStyle
{  public :
	gsbyte charh;   //!< vertical dimension of character A (without counting any spaces below or above)
	gsbyte wperc;   //!< char width as percentage of the default width in specified height
	gschar charspc; //!< additional char spacing in pixels, can be <0
	gschar fontid;  //!< font id, -1 if undefined, in which case the default font should be used
   public :
	GsFontStyle ( gsbyte h=10, gsbyte wp=100, gschar cs=0, gschar fid=-1 )
	{ set(h,wp,cs,fid); }

	GsFontStyle ( const GsFontStyle& fs )
	{ set(fs.charh,fs.wperc,fs.charspc,fs.fontid); }

	void set ( gsbyte h, gsbyte wp, gschar cs, gschar fid )
	{ charh=h; wperc=wp; charspc=cs; fontid=fid; }

	void set ( gsbyte h, gsbyte wp, gschar cs )
	{ charh=h; wperc=wp; charspc=cs; }

	bool operator == ( const GsFontStyle& s )
	{ return charh==s.charh && wperc==s.wperc && charspc==s.charspc && fontid==s.fontid; }
};

//====================== GsFont =====================

/*! Stores font metrics data */
class GsFont
{  public :
	struct Character
	 { gsint16 x0, y0, x1, y1; 
	   gsint16 xoffset1, xoffset2, yoffset1, yoffset2; // these are 100 times their true values
	   float xadvance;
	 };

   private :   
	float _top, _ascent, _descent, _bottom;
	Character* _chars;
	gsuint16 _nchars;
	gsuint16 _imgw, _imgh;
	gscbool _allocateddata;

   public :
	GsFont ();
	virtual ~GsFont ();
	int imgw () const { return (int)_imgw; }
	int imgh () const { return (int)_imgh; }
	float ascent () const { return _ascent; }
	float descent () const { return _descent; }
	float top () const { return _top; }
	float bottom () const { return _bottom; }
	float height () const { return _top+_bottom; }
	int characters () const { return _nchars; }
	const Character& charindex ( int i ) const { return _chars[i]; }
	const Character& character ( char c ) const { return _chars[c-32]; }
	float xadvance ( char c ) const { return _chars[c-32].xadvance; }

	float hscaling ( float charh ) const { return charh/_ascent; }
	float hscaling ( const GsFontStyle& fs ) const { return hscaling(float(fs.charh)); }
	float wscaling ( const GsFontStyle& fs ) const { return hscaling(fs)*float(fs.wperc)/100.0f; }
	GsVec2 scaling ( const GsFontStyle& fs ) const { GsVec2 s(float(fs.wperc)/100,hscaling(fs)); s.x*=s.y; return s; }

   public :
	bool loaded () const { return characters()>0; }
	bool load ( const char* filename );
	void adopt_static_data ( Character* cdata, int nchars, int imgw, int imgh, float t, float a, float d, float b );

   public :
	/*! Returns text width; if n<0 all characters are considered, otherwise only n characters are counted */
	float text_width ( const GsFontStyle& fs, const char* text, int n=-1 ) const;

	GsVec2 text_size ( const GsFontStyle& fs, const char* text, float extralinespc=0 ) const;

	/*! Output the text to the pobs scene node, which has to be already started as a Masked group. Returns last x coordinate.
		If 's' is the value of parameter cs, scene coordinates are assumed, otherwise window coordinated.
		Optional parameter e3c is to provide extra 3 colors to be assigned to the vertices of each character's quad. */
	float output ( const GsFontStyle& fs, const char* text, float x, float y, GsColor c, SnPlanarObjects* pobs,
                   float extralinespc=0, const float* xmax=0, char cs='s', int accel=-1, GsColor* e3c=0 ) const;
};

//=============================== End of File =================================

# endif  // GS_FONT_H
