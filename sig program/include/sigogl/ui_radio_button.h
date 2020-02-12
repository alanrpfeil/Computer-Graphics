/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file ui.h
 * OpenGL minimalistic user interface
 */

# ifndef UI_RADIO_BUTTON_H
# define UI_RADIO_BUTTON_H

# include <sigogl/ui_check_button.h>

/*! A radio button will enforce that among all radio buttons in a same
	panel, only one can be selected at a time.
	UiElement colors: bg:button, fg:selection mark, ln:frame, lb:label.
	The background color of a selected button will always be ButtonSelectionBg */
class UiRadioButton : public UiCheckButton
{  public :
	UiRadioButton ( const char* l, int ev, bool val=false, int x=0, int y=0, int mw=0, int mh=0 );
	void set () { value(true); make_value_unique(); }
	void make_value_unique ();
	virtual int handle ( const GsEvent& e, UiManager* uim ) override;
};

//================================ End of File =================================================

# endif // UI_RADIO_BUTTON_H
