/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file gs_label.h
 * UI label management class
 */

# ifndef UI_LABEL_H
# define UI_LABEL_H

# include <sig/gs_font.h>
# include <sig/gs_color.h>
# include <sig/gs_rect.h>

class SnPlanarObjects;

/*	UiLabel contains the label string and additional positioning and appearance information.
	The maintained label is dynamically allocated in GsCharPt. UiLabel does not have a virtual
	destructor and its use as a base class should be limited to only appropriate cases. */
class UiLabel
{  public :
	enum Alignment { Left, Center, Right };

   private :
	GsCharPt  _text; // dynamically allocated label text
	GsFontStyle _fs; // font parameters to be used
	gsint16  _x, _y; // position to start drawing the text, in local coordinates
	gsint16  _w, _h; // width and height of the text in pixels (note: x,y,w,h is not the rect containing the text)
	gschar   _accel; // text index position of the accelerator character, -1 if none
	gscenum  _align; // alignment of the element label

   public :
	UiLabel (); // default alignment is Left
   ~UiLabel ();
	const char* text () const { return _text; }
	void pos ( int x, int y ) { _x=x; _y=y; }
	void move ( int dx, int dy ) { _x+=dx; _y+=dy; }
	void addspace ( int dx ) { _x+=dx; _w+=dx; }
	void x ( int x ) { _x=x; }
	void y ( int y ) { _y=y; }
	gsint16 x () const { return _x; }
	gsint16 y () const { return _y; }
	gsint16 w () const { return _w; }
	gsint16 h () const { return _h; }
	gsint16 xp () const { return _x+_w; }
	float base () const;
	int search_fkey () const; // returns 0 if no F key, otherwise 1, 2, ... for F1, F2, ...
	bool hasaccel () const { return _accel>=0; }
	gschar accelind () const { return _accel; }
	gschar accelchar () const { return _accel<0? 0:GS_LOWER(_text[_accel]); }
	void accelrange ( float& a, float& b, const char* st=0 ) const;
	void left () { _align=Left; }
	void right () { _align=Right; }
	void center () { _align=Center; }
	void align ( Alignment a ) { _align=a; }
	Alignment alignment () const { return (Alignment)_align; }
	GsFontStyle& fs () { return _fs; }

	/*! Returns the rectangle containing the label in local coordinates.
		If there is no text defined the rect will have the size to contain a space character in the default font. */
	GsRect rect() const { return GsRect((float)_x,(float)_y-base(),(float)_w,(float)_h); }

	/*! Returns the rectangle containing the label in global coordinates by adding its local position to (x,y)*/
	GsRect rect ( int x, int y ) const { return GsRect(float(x+_x),float(y+_y)-base(),(float)_w,(float)_h); }
	GsRect rect ( float x, float y ) const { return GsRect(x+(float)_x,y+float(_y-base()),(float)_w,(float)_h); }

	/*! Update label size (w,h) from the size of the given string s. If the given string is null or empty,
		The label rect will have the size for a space character in the default font. */
	void set_size ( const char* s );

	/*! Allocates the needed internal space and copies to it the contents of l */
	void set ( const char* l, bool chkaccel=true );

	/*! Allocates s+1 bytes space for a string of s characters, or frees internal memory if s<=0 */
	void size ( int s );

	/*! Draw the label at coordinates (x,y)+(_x,_y) according to the font style of the label */
	void draw ( SnPlanarObjects* pobs, float x, float y, GsColor c, float* xmax=0, const char* s=0 ) const;

	/*! Copy the spacing-related members (x,y,w,h,spc) from l */
	void copy_spacing ( const UiLabel& l );

	/*! Output values */
	friend GsOutput& operator<< ( GsOutput& o, const UiLabel& l );
};

//================================ End of File =================================================

# endif // UI_LABEL_H
