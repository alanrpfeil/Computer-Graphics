/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file ui.h
 * OpenGL minimalistic user interface
 */

# ifndef UI_INPUT_H
# define UI_INPUT_H

# include <sig/gs_string.h>
# include <sigogl/ui_element.h>

/*! An input element allows input of a single line text.
	UiElement colors: bg:input area, fg:input text, lb:label. */
class UiInput : public UiElement
 { private :
	gscenum _evgen;  // event generation: 0:none, 1:tab+enter, 2:each key
	gscbool _inuse;  // 0:not in use, 1:mouse cursor over, 2:being edited
	gscenum _mode;   // 1:real, 2:integer, otherwise all characters accepted
	gsint16 _cursor; // cursor position is the index of next character
	gsint16 _curini; // current initial text display position
	gsint16 _event;  // event id
	GsRect  _ir;	 // input rect
	GsString _text;  // input text
    GsCharPt _restext; // text to restore when Esc is pressed
	void (*_usercb) ( UiInput* inp, const GsEvent& ev, char delc, void* udata );
	void* _userdata;

   protected :
	void setir ();

   public :
	UiInput ( const char* l, int ev, int x=0, int y=0, int mw=120, int mh=0, const char* t=0 );

	const GsString& cvalue () const { return _text; }
	GsString& value () { return _text; }
	float valuef () const { return _text.atof(); }
	int valuei () const { return _text.atoi(); }
	void value ( const char* s ) { _text=s; }
	void sets ( const char* s ) { value(s); _mode=0; }
	void setf ( const char* s ) { value(s); _mode=1; }
	void seti ( const char* s ) { value(s); _mode=2; }
	void all_events () { _evgen=2; }
	void normal_events () { _evgen=1; }
	void no_events () { _evgen=0; }
	const GsRect& input_rect () const { return _ir; }

	/*! Set a user callback that is called each time a key is pressed */
	void callback ( void(*cb)(UiInput*,const GsEvent&,char,void*), void* udata ) { _usercb=cb; _userdata=udata; }

	/*! Returns the associated callback data */
	void* userdata () const { return _userdata; }

	void set_style ( const UiStyle& s );
	UiInput* adjacent_input ( int inc );

	// Overriden virtual methods:
	virtual void change_style ( const UiStyle& s ) override;
	virtual void move ( float dx, float dy ) override;
	virtual void resize ( float w, float h ) override;
	virtual void build () override;
	virtual void draw ( UiPanel* panel ) override;
	virtual int handle ( const GsEvent& e, UiManager* uim ) override;
	virtual void close () override;
};

//================================ End of File =================================================

# endif // UI_INPUT_H
