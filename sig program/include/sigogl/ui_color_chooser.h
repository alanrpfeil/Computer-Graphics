/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file ui.h
 * OpenGL minimalistic user interface
 */

# ifndef UI_COLOR_CHOOSER_H
# define UI_COLOR_CHOOSER_H

# include <sigogl/ui_element.h>

class UiColorChooser : public UiElement
{  public :
   private :
	gschar _gbarw, _sbarh, _spc, _state;
	gscbool _allev;	   // if events should be sent continously
	gsint16 _event;	   // event identifier
	GsRect _rc, _rg, _rs; // colors rect, gray scale bar, selection bar
	GsPnt2 _pvalue;
	float _w1, _w2, _w3;
	GsColor _cvalue, _center, _c2, _c3;
	void _centervalue ();
	void _getpoints ( GsPnt2& g, GsPnt2& y, GsPnt2& r, GsPnt2& o, GsPnt2& c, GsPnt2& b, GsPnt2& p );
	void _mouseev ( int mx, int my, UiManager* uim );
	void _haspoint ( GsPnt2& a, const GsPnt2& b, const GsPnt2& c, GsColor ca, GsColor cb, GsColor cc, const GsPnt2& m );
   public :
	UiColorChooser ( GsColor c, int ev, int x=0, int y=0, int mw=600, int mh=340 );
	const GsColor& value () const { return _cvalue; }
	void value ( const GsColor& c );
	void all_events ( bool b ) { _allev=b; } // if all events should be sent
	void set_style ( const UiStyle& s );
	// Overriden virtual methods:
	virtual void change_style ( const UiStyle& s );
	virtual void move ( float dx, float dy ) override;
	virtual void resize ( float w, float h ) override;
	virtual void build () override;
	virtual void draw ( UiPanel* panel ) override;
	virtual int handle ( const GsEvent& e, UiManager* uim ) override;
	virtual void close () override;
};

//================================ End of File =================================================

# endif // UI_COLOR_CHOOSER_H
