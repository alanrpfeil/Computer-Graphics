/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file ui_element.h
 * OpenGL minimalistic user interface
 */

# ifndef UI_ELEMENT_H
# define UI_ELEMENT_H

# include <sig/gs_shareable.h>
# include <sig/gs_color.h>
# include <sig/gs_event.h>
# include <sig/gs_font.h>
# include <sig/gs_rect.h>
# include <sig/sn_group.h>
# include <sigogl/ui_label.h>
# include <sigogl/ui_style.h>

class UiManager;
class UiPanel;
class UiButton;
class SnPlanarObjects;

/* UiElement derives from SnGroup and is the base class for all ui elements.
   It includes functionality to display its label. It offers no user interaction. */
class UiElement : public GsShareable
{  public :
	/*! The Type enumerator lists the basic element types in order to
		facilitate the operation of the user interface elements. */
	enum Type { Label, Panel, Submenu, Button, CheckButton, RadioButton, Slider, Output, Input, Generic };

	/*! State of the element: when Hidden the element does not appear, when Disabled it will be
		displayed but not function, and when Active it will be in normal operation mode */
	enum State { Hidden, Active, Disabled };

	/*! Changed state of the element: when Hidden the element does not appear, when Disabled it will be
		displayed but not function, and when Active it will be in normal operation mode */
	enum ChangedState { NoChange, NeedsRedraw, NeedsRebuild };

	/*! The Color structure here contains a few colors per element in order to allow customization
		with respect to the current color style. The colors are named to be used for the background, 
		foreground, lines and label	of the elements, but elements may use these colors in different ways.
		Elements will automatically at construction time get values from the current color scheme
		and assign the respective values to these colors, which can later be changed by the user. */
	struct Color { GsColor bg, fg, ln, lb; };

   protected :
	UiElement* _owner;	// a panel or a button, null if none
	GsRect	_rect;		// area containing the element in global window coordinates
	Color	_color;		// color scheme of the element instance
	UiLabel _label;		// label with dynamically allocated text
	gscenum _type;		// specialized type of this element
	gscenum _state;		// activation state
	gscbool _separated; // if display separator line between this and the prior element
	gscenum	_changed;	 // 0:no change, 1:needs redraw, 2:needs build
	gsbyte	_xspc,_yspc; // spacing between rect borders and the element's contents
	gsbyte	_pspc,_nspc; // prior and next spacing outside the element's rect (in its panel)
	gsuint16	_minw,_minh; // minimum size of the element, user-specified and updated during build()
	gsbyte	_lxs;		 // group alignment spacing, from border to label, used in build and resize, specified by derived classes

   protected : // constructor below requires a type and should only be used by derived elements

	/*! The protected contructor receives the element type t, its label l, its desired (x,y),
		position and the desired minimum width and height (w,h). Actual dimensions will be 
		set by the specific derived implementation each element will have in build(). */
	UiElement ( Type t, const char* l=0, int x=0, int y=0, int mw=0, int mh=0 );

   public : // constructor below initilizes the element with its basic functionality of displaying its label

	/*! The public contructor creates and elment of type Label and receives its label text l,
		its desired (x,y) position,	and the desired minimum width and height (w,h). */
	UiElement ( const char* l=0, int x=0, int y=0, int mw=0, int mh=0 ) : UiElement(Label,l,x,y,mw,mh) {}

	/*! Destructor */
	virtual ~UiElement ();

	/*! Returns the "changed" state as encoded in enumerator ChangedState. The returned value has
		the following meaning: 0:no change, 1:needs draw, 2:needs build */
	gscenum changed () const { return _changed; }

	/*! If the current state does not have a higher or equal value than ns, updates the entire branch
		up to the manager, which is also set to the new state ns. If the new state is lower or equal
		as the current one, nothing is done. */
	void changed ( ChangedState ns );

    /*! The owner will be the panel managing this element, or the owner button in case this
        element is a submenu, or null. */
	UiElement* owner () { return _owner; }
	UiElement* root () { UiElement* e=this; while (e->owner()) e=e->owner(); return e; }
	void owner ( UiElement* e ) { _owner=e; }
	bool submenu () const { return _type==Submenu; }
	bool panel () const { return _type==Panel||_type==Submenu; }
	bool in_submenu () const { return _owner && _owner->submenu(); }
	bool in_panel () const { return _owner && _owner->panel(); }
	Type type () const { return (Type)_type; }

	State state () const { return (State)_state; }
	void state ( State s ) { _state=s; }
	void hide () { _state=Hidden; }
	void disable () { _state=Disabled; }
	void activate () { _state=Active; }
	bool active () { return _state==Active; }
	bool hidden () { return _state==Hidden; }
	bool disabled () { return _state==Disabled; }

	void separate () { _separated=1; }
	gscbool separated () { return _separated; }
	UiLabel& label () { return _label; }
	void format_label ( GsColor c, UiLabel::Alignment a ) { _color.lb=c; _label.align(a); }
	GsRect& rect () { return _rect; }
	float x () const { return _rect.x; }
	float y () const { return _rect.y; }
	float w () const { return _rect.w; }
	float h () const { return _rect.h; }

	Color& color () { return _color; }
	void color_set ( GsColor a, GsColor b, GsColor c, GsColor d ) { _color.bg=a; _color.fg=b; _color.ln=c; _color.lb=d; }

	void spacing ( int dx, int dy ) { _xspc=dx; _yspc=dy; }
	void xspacing ( int dx ) { _xspc=dx; }
	void yspacing ( int dy ) { _yspc=dy; }
	gsbyte xspacing () const { return _xspc; }
	gsbyte yspacing () const { return _yspc; }
	void lxspc ( int dx ) { _lxs=dx; }
	gsbyte lxspc () const { return _lxs; }
	void minw ( int mw ) { _minw=mw; }
	void minh ( int mh ) { _minh=mh; }
	int minw () const { return _minw; }
	int minh () const { return _minh; }
	void minsize ( int mw, int mh ) { minw(mw); minh(mh); }
	void priorspc ( int ps ) { _pspc=ps; }
	void nextspc ( int ns ) { _nspc=ns; }
	int priorspc () const { return _pspc; }
	int nextspc () const { return _nspc; }

	/*! Sets relevant internal parameters according to the given style, more specifically,
		the label alignment and color (_color.lb). This method is automatically called by 
		UiElement's constructor for the current style. */
	void set_style ( const UiStyle& s );

	/*! This method should be derived in order to update internal parameters according to
		a desired change in style. The UiElement implementation will just call set_style(s). */
	virtual void change_style ( const UiStyle& s );

	/*! Sets the top-left position. All coordinates are in global window coordinates */
	virtual void pos ( float x, float y );

	/*! Moves the element by the given displacements in x and y */
	virtual void move ( float dx, float dy );

	/*! Called when elements should declare global key commands */
	virtual void add_key_commands ( UiManager* uim ) { return; }

	/*! Computes the element size and internal layout for containing all content.
		This UiElement implementation builds the minimum rectangle containing the label.
		State changed is then set to 1 to indicate a new redraw is needed.  */
	virtual void build ();

	/*! Resize the element and adjust the internal layout accordingly */
	virtual void resize ( float w, float h );

	/*! Draw the element in its panel in global coordinates and set changed state to NoChange */
	virtual void draw ( UiPanel* panel );

	/*! Process the event and returns: 0 if event not used, and 1 if it was used */
	virtual int handle ( const GsEvent& e, UiManager* uim ) { return 0; }

	/*! Callback called when the element will be closed */
	virtual void close () {}
};

//================================ End of File =================================================

# endif // UI_ELEMENT_H
