/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/ui_style.h>

//================================ UiStyle =====================================

// Static members:
UiStyle& UiStyle::Current () { static UiStyle cur(UiStyle::Light); return cur; }
UiStyle& UiStyle::Dialog () { static UiStyle dlg(UiStyle::Gray); return dlg; }

// Important about static objects: we cannot guarantee order of creation,
// so in this file we initilialize all colors by value, and not by accessing
// the predefined colors in GsColor.

// Save/Restore functionality:
static UiStyle* Saved=0;
void UiStyle::Save () { if (!Saved) Saved=new UiStyle(Current()); }
void UiStyle::Restore () { if (Saved) { Current()=*Saved; delete Saved; Saved=0; } }

UiStyle::UiStyle ( StyleType t )
{
	set ( t );
}

UiStyle::UiStyle ( UiStyle& s ) :
		color ( s.color ),
		alignment ( s.alignment ),
		font ( s.font ),
		type ( s.type )
{
}

void UiStyle::set ( StyleType t )
{
	switch ( t )
	{	case Subtle:set_subtle_style (); break;
		case Light:	set_light_style (); break;
		case Gray:	set_gray_style (); break;
		case Dark:	set_dark_style (); break;
		default: type=Custom;
	}
}

void UiStyle::set_font_id ( int fid )
{
	font.label.fontid = fid;
	font.input.fontid = fid;
	font.panel_title.fontid = fid;
	font.slider.fontid = fid;
}

void UiStyle::set_font_h ( int h )
{
	font.label.charh = h;
	font.input.charh = h;
	font.panel_title.charh = h;
	font.slider.charh = h;
}

void UiStyle::set_subtle_style ()
{
	set_light_style();
	type = Subtle;

	color.panel_bg.set ( 250, 250, 250, 220 );
	color.panel_separator.set ( 150, 150, 150, 80 );
	color.panel_frame.set ( 0, 0, 0, 0 );
}

void UiStyle::set_light_style ()
 {
	color.background.set ( 255, 255, 255, 255 ); // white
	color.element_label.set ( 0, 0, 0, 255 );

	color.panel_bg.set ( 248, 248, 248, 220 );
	color.panel_separator.set ( 0, 0, 0, 100 );
	color.panel_frame.set ( 0, 0, 0, 100 );
	color.panel_title.set ( 10, 30, 120, 255 );
	color.close_button.set ( 255, 0, 0, 255 ); // red

	color.button_bg.set ( 180, 180, 180, 0 );
	color.button_frame.set ( 0, 0, 0, 255 );
	color.button_label.set ( 0, 0, 0, 255 );
	color.submenu_arrow.set ( 0, 0, 0, 255 );

	color.slider_line.set ( 80, 80, 80, 255 );
	color.slider_value.set ( 0, 0, 0, 255 );

	color.output_bg = color.panel_bg;
	color.output_fg.set ( 0, 0, 80, 255 );

	color.input_bg.set ( 210, 210, 210, 240 );
	color.input_fg.set ( 0, 0, 80, 255 );
	color.cursor.set ( 225, 40, 40, 255 );

	color.selection_fg.set ( 255, 255, 255, 255 );
	color.selection_bg.set ( 10, 50, 230, 200 );
	color.disabled_fg.set ( 200, 200, 200, 255 );

	alignment.element = UiLabel::Left;
	alignment.panel_title = UiLabel::Left;

	// The fontid is lest as -1 which indicates the defaut pre-defined font to be used
	font.label.set		( 10, 100, 0 );
	font.input.set		( 10, 100, 0 );
	font.panel_title.set( 10, 100, 0 );
	font.slider.set		( 10, 100, 0 );

	type = Light;
}

// GsColor values are: gray (127,127,127), dark gray (64,64,64), light gray (225,225,225)

void UiStyle::set_gray_style ()
{
	set_light_style ();
	type = Gray;

	color.panel_bg.set ( 230, 230, 230, 255 );
	color.panel_frame.set ( 0, 0, 0, 0 ); // no frame

	color.button_bg.set ( 200, 200, 200, 0 );

	alignment.element = UiLabel::Center;
	alignment.panel_title = UiLabel::Center;
}

void UiStyle::set_dark_style ()
{
	set_light_style ();
	type = Dark;

	GsColor bg ( 6, 16, 16, 255 );
	GsColor fg ( 190, 190, 190, 255 );

	color.background.set ( 0, 0, 0, 255 ); // black;

	color.element_label = fg;

	color.panel_bg = bg;
	color.panel_separator = fg;
	color.panel_frame = fg;
	color.panel_title.set ( 190, 190, 255, 255 );

	color.button_bg.set ( 180, 180, 180, 0 );
	color.button_frame = fg;

	color.button_label = fg;
	color.submenu_arrow = fg;

	color.slider_line.set ( 80, 80, 80, 255 );
	color.slider_value = fg;

	color.output_bg = bg;
	color.output_fg = fg;

	color.input_bg.set ( 80, 80, 80, 255 );
	color.input_fg = fg;

	color.selection_fg.set ( 255, 255, 255, 255 );
	color.selection_bg.set ( 10, 150, 255, 200 );
}

void UiStyle::operator = ( const UiStyle& s )
{
	color = s.color;
	font = s.font;
	alignment = s.alignment;
	type = s.type;
}

//================================ End of File ==================================
