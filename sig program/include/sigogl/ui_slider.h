/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file ui.h
 * OpenGL minimalistic user interface
 */

# ifndef UI_SLIDER_H
# define UI_SLIDER_H

# include <sigogl/ui_element.h>

/*! A slider element with optional display of the current number.
	UiElement colors: bg:slider mark, fg:value number (if displayed), ln:line, lb:label. */
class UiSlider : public UiElement
{  private :
	char _fmt[6];	  // format string "%N.Df" to display the current value
	gscbool _allev;	  // if events during slider manip are to be generated
	gscbool _inuse;	  // true while mouse cursor is over the element
	gsint16 _nx, _nw; // xpos and width taken by the number format
	gsint16 _event;	  // event id
	float _t;		  // slider value in [0,1]
	float _inc;		  // increment for keyboard input
	float _min, _max; // range
	GsRect _sr;		  // slider rect

   protected :
	void setsr ();

   public :
	UiSlider ( const char* l, int ev, int x=0, int y=0, int mw=120, int mh=0 );
	float value () const; // returns value between min, max
	float tvalue () const { return _t; } // return values between 0,1
	void value ( float v ); // between min, max
	void tvalue ( float t ); // between 0,1
	void range ( float min, float max );
	float range_min () const { return _min; }
	float range_max () const { return _max; }
	void format ( int n, int d ) { _fmt[1]='0'+n; _fmt[3]='0'+d; } // n digits, d decimals; if n==0 number does not show
	void increment ( float inc ) { _inc=inc/(_max-_min); }
	void configure ( float min, float max, float inc, int n, int d ) { range(min,max); increment(inc); format(n,d); }
	void configure ( int min, int max, int inc, int n ) { range((float)min,(float)max); increment((float)inc); format(n,0); }
	void all_events ( bool b ) { _allev=b; }
	void set_style ( const UiStyle& s );

	// Overriden virtual methods:
	virtual void change_style ( const UiStyle& s ) override;
	virtual void move ( float dx, float dy ) override;
	virtual void resize ( float w, float h ) override;
	virtual void build () override;
	virtual void draw ( UiPanel* panel ) override;
	virtual int handle ( const GsEvent& e, UiManager* uim ) override;
	virtual void close () override;
};

//================================ End of File =========================================

# endif // UI_SLIDER_H
