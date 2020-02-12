/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file ui.h
 * OpenGL minimalistic user interface
 */

# ifndef UI_CHECK_BUTTON_H
# define UI_CHECK_BUTTON_H

# include <sigogl/ui_button.h>

/*! A check button can only be selected or unselected.
	UiElement colors: bg:button bg, fg:check mark region color, ln:check mark, lb:label.
	The bg/fg color of a selected button will always come from UiStyle::Current().Color.SelectionBg/Fg. */
class UiCheckButton : public UiButton
 { public:
	enum State { Undef=-1, Unset=0, Set=1 };
   protected :
	gschar _value;
	gsbyte  _drawsq;
	GsRect  _box;
   public :
	UiCheckButton ( const char* l, int ev, bool val=false, int x=0, int y=0, int mw=0, int mh=0 );
	void value ( gscbool b ) { _value=b; changed(NeedsRedraw); }
	void state ( State s ) { _value=(gschar)s; changed(NeedsRedraw); }
	void value ( int s ) { _value=(gschar)s; changed(NeedsRedraw); }
	void value ( bool b ) { _value=b? Set:Unset; changed(NeedsRedraw); }
	State state () const { return (State)_value; }
	bool value () const { return _value==1; } // returns value as a boolean
	void drawsq ( bool b ) { _drawsq=(gsbyte)b; }
	void set_style ( const UiStyle& s );
	// Overriden virtual methods:
	virtual void change_style ( const UiStyle& s );
	virtual void build () override;
	virtual void draw ( UiPanel* panel ) override;
	virtual int handle ( const GsEvent& e, UiManager* uim ) override;
};

//================================ End of File =================================================

# endif // UI_CHECK_BUTTON_H
