/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file ui.h
 * OpenGL minimalistic user interface
 */

# ifndef UI_BUTTON_H
# define UI_BUTTON_H

# include <sigogl/ui_element.h>

/*! A standalone button or menu button element.
	UiElement colors: bg:button, lb:label.
	The bg/fg color of a selected button will always come from UiStyle::Current().Color.SelectionBg/Fg. */
class UiButton : public UiElement
 { protected :
	gscbool _selected;
	gsint16 _event;
	UiPanel* _submenu;
	UiManager* _uimcmd;
	gsuint16 _accel1, _accel2;
	void _init ( int ev, UiPanel* p );
   public :
	UiButton ( const char* l, int ev, int x=0, int y=0, int mw=0, int mh=0, UiPanel* p=0 );
	UiButton ( const char* l, UiPanel* p ); // p is optional submenu
	virtual ~UiButton ();
	int get_event () const { return _event; }
	void submenu ( UiPanel* p );
	UiPanel* submenu () { return _submenu; }
	void accelerator ( int key ) { _accel1=key; }
	void accelerator ( int key1, int key2 ) { _accel1=key1; _accel2=key2; }
	void set_style ( const UiStyle& s );
	// Overriden virtual methods:
	virtual void change_style ( const UiStyle& s ) override;
	virtual void move ( float dx, float dy ) override;
	virtual void resize ( float w, float h ) override;
	virtual void add_key_commands ( UiManager* uim ) override;
	virtual void build () override;
	virtual void draw ( UiPanel* panel ) override;
	virtual int handle ( const GsEvent& e, UiManager* uim ) override;
	virtual void close () override;
};

//================================ End of File =================================================

# endif // UI_BUTTON_H
