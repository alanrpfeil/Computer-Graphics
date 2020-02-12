/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file ui.h
 * OpenGL minimalistic user interface
 */

# ifndef UI_OUTPUT_H
# define UI_OUTPUT_H

# include <sig/gs_string.h>
# include <sigogl/ui_element.h>

/*! UiOutput provides text output.
	UiElement colors: bg:output region, fg:text color, lb:label, ln:not used. */
class UiOutput : public UiElement
{  private :
	GsString _text;
	gsint16 _event;
	gscbool _showlabel;
	gscbool _rectclip;
	gscbool _wwrap;
	void _init ( const char* l );
   public :
	UiOutput ( const char* l, int x=0, int y=0, int mw=0, int mh=0 );
	UiOutput ( const char* l, const char* t, int x=0, int y=0, int mw=0, int mh=0 );
	const GsString& ctext() const { return _text; }
	GsString& text();
	void rect_clip ( bool b ) { _rectclip=(gscbool)b; }
	void word_wrap ( bool b ) { _wwrap=(gscbool)b; }
	void set_style ( const UiStyle& s );
	// Overriden virtual methods:
	virtual void change_style ( const UiStyle& s ) override;
	virtual void resize ( float w, float h ) override;
	virtual void build () override;
	virtual void draw ( UiPanel* panel ) override;
	virtual int handle ( const GsEvent& e, UiManager* uim ) override;
};

//================================ End of File =================================================

# endif // UI_OUTPUT_H
