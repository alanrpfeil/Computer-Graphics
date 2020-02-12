/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file ui_style.h
 * Color scheme management for graphim's minimalistic user interface
 */
# ifndef UI_STYLE_H
# define UI_STYLE_H

# include <sig/gs_color.h>
# include <sig/gs_font.h>
# include <sigogl/ui_label.h>

/*! UiStyle maintains settings and colors to be used by UI elements.
	To have an element to not be drawn the alpha value of the corresponding 
	color is to be set to zero. */
class UiStyle
{ public :
	/*! Current returns the current style being used by elements when they are created. Initialized as style Light. */
	static UiStyle& Current();

	/*! Dialog returns the style that will become current when dialog boxes are created. Initialized as style Gray. */
	static UiStyle& Dialog();

	/*! Save current style so that it can be later restored. It has no effect if a style is already currently saved. */
	static void Save();

	/*! Restore the currently saved style. It has no effect if no style has been saved. */
	static void Restore();

   public :

	/*! Style types. Default is Light with Gray dialogs. */
	enum StyleType { Subtle, Light, Gray, Dark, Custom };

	/*! Each element will use the current style colors defined below and map them to 
		their internal colors (bg, fg, ln, lb), which can then be customized if desired. */
	struct ColorScheme { GsColor background, element_label,
						panel_bg, panel_separator, panel_frame, panel_title, close_button,
						button_bg, button_frame, button_label, submenu_arrow,
						slider_line, slider_value,
						input_bg, input_fg, cursor,
						output_bg, output_fg,
						selection_bg, selection_fg, disabled_fg;
						};

	struct AlignmentScheme { UiLabel::Alignment element, panel_title; };
	struct FontScheme { GsFontStyle label, input, panel_title, slider; };

	ColorScheme color;
	AlignmentScheme alignment;
	FontScheme font;
	StyleType type;

   public :

	/*! Initializes as given style t. If t is Custom no style values are set. */
	UiStyle ( StyleType t );

	/*! Copy constructor */
	UiStyle ( UiStyle& s );

	/*! Sets the style to become given style t. If t is Custom only the style type is set. */
	void set ( StyleType t );

	/*! Restore values to the original ones as defined by the style type of this instance. */
	void restore_style () { set(type); }

	/*! Set given font id for all font styles in the font scheme */
	void set_font_id ( int fid );

	/*! Set given font height for all font styles in the font scheme */
	void set_font_h ( int h );

	/*! Same as set(UiStyle::Subtle) */
	void set_subtle_style ();

	/*! Same as set(UiStyle::Light) */
	void set_light_style ();

	/*! Same as set(UiStyle::Gray) */
	void set_gray_style ();

	/*! Same as set(UiStyle::Dark) */
	void set_dark_style ();

	/*! Assignment operator */
	void operator = ( const UiStyle& s );
};

//================================ End of File =================================================

# endif // UI_STYLE_H
