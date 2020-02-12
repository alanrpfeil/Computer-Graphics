/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/ui_manager.h>
# include <sigogl/ui_radio_button.h>

//================================== UiRadioButton =========================================

UiRadioButton::UiRadioButton ( const char* l, int ev, bool val, int x, int y, int mw, int mh )
			  :UiCheckButton ( l, ev, val, x, y, mw, mh )
{
	_type = UiElement::RadioButton;
}

void UiRadioButton::make_value_unique ()
{
	if ( !in_panel() ) return;
	UiPanel* p = (UiPanel*)_owner;

	// find our position:
	int r, i, s=p->elements();
	for ( i=0; i<s; i++ ) if ( p->get(i)==this ) break;
	if ( i==s ) return; // not found

	// now adjust values in the adjacent radio buttons:
	r = i;
	for ( i=r-1; i>=0; i-- ) if ( p->get(i)->type()!=RadioButton ) break; else ((UiRadioButton*)p->get(i))->value(!_value);
	for ( i=r+1; i<s;  i++ ) if ( p->get(i)->type()!=RadioButton ) break; else ((UiRadioButton*)p->get(i))->value(!_value);
}

int UiRadioButton::handle ( const GsEvent& e, UiManager* uim )
{
	gsbyte origval = _value;
	int h = UiCheckButton::handle(e,uim);
	if ( h && origval!=_value ) // value changed
	{	if ( _type==RadioButton ) make_value_unique();
	}
	return h;
}

//================================ End of File =================================================
