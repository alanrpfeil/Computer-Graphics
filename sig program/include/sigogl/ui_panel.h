/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file ui.h
 * OpenGL minimalistic user interface
 */

# ifndef UI_PANEL_H
# define UI_PANEL_H

# include <sig/gs_font.h>
# include <sigogl/ui_element.h>

class SnPlanarObjects;
class UiCheckButton;

/*! A panel contains a collection of UiElments.
	UiElement colors: bg:panel, fg:separator line, ln:panel frame, lb:title.
	Children elements are in global coordinates to speed up containment tests.
	The first scene nodes in the SnGroup are to draw the panel, and
	children elements start at index _firstelem. */
class UiPanel : public UiElement
{  public :
	/*! In Vertical layout all elements are stacked on top of each other,
		in Horizontal layout they are side by side on a single bar, and
		HorizCenter is as Horizontal but elements will be centered in case the panel is resized.
		Labels of the inserted elements will be alligned according to 
		UiStyle::Current().Alignment.Element; */
	enum Format { Vertical, HorizLeft, HorizCenter };
	enum Dock { Left, Right, Top, TopBar, Bottom, BottomBar, Float, Center, Corner };

   private :
	gscenum _format;
	gscenum _dock;
	gscbool _moving;
	gscbool _gotprevev;
    gscbool _inxclose;	// if pointer in close button
	gsint16 _xclose;	// <0: no close button, >0: contains x limit
	GsColor _sepcolor;
	UiManager* _uimparent; // uim pointer or null if not directly under a UiManager
	SnPlanarObjects* _pobs;
	SnGroup* _sngroup;
	GsArrayRef<UiElement> _elements;

   protected :
	friend UiButton;
	friend UiManager; // _uimparent is manipulated by UiManager
	void _init ( Format f, Dock d );

   public :
	UiPanel ( const char* l=0, Format f=Vertical, Dock d=Float, int x=0, int y=0, int mw=0, int mh=0 );
	UiPanel ( const char* l, Format f, int x, int y, int mw=0, int mh=0 ); // will set float dock
   ~UiPanel ();

	// Acess to the panel scene:
	SnGroup* sngroup () const { return _sngroup; }
	SnPlanarObjects* pobs () const { return _pobs; }

	// Access and manipulation of children elements:
	int elements () const { return _elements.size(); }
	UiElement* get ( int i ) const { return _elements[i]; }
	UiElement* add ( UiElement* e );
	UiElement* insert ( int pos, UiElement* e );
	UiElement* top () const { return _elements.top(); }
	template <class X> X* get ( int i ) const { return dynamic_cast<X*>(_elements[i]); }
	template <class X> X* top () const { return dynamic_cast<X*>(_elements.top()); }
	void init ( Format f, Dock d ); // remove all elements and re-initialize
	void pop (); // remove top element

	// Access to the manager:
    UiManager* uimparent () const { return _uimparent; }

	// Access to customizable colors:
	GsColor& separator_color () { return _sepcolor; }

	// Query panel layout:
	bool horizontal () const { return _format!=Vertical; }
	bool vertical () const { return _format==Vertical; }

	/*! Set the panel to have or not a close button near the title */
	void close_button ( bool );
	bool close_button () const { return _xclose<0? false:true; }

	/*! Set internal draging state to not moving. */
	void disable_dragging () { _moving=-1; }

	/*! Utility to search for a button, check button, or radio button containing the given
		event in this panel or any sub-panel. If not found, 0 is returned. */
	UiButton* find_button ( gsint16 ev );

	/*! Align the position to the window according to current docking state */
	int align ( int winw, int winh );

	/*! In Horizontal format will make each child have same width and then 
		centralizes them; in vertical format will only centralize the labels of the children.
		Has to be called after build(). */
	void center_children ();

	/* Repositions if outside window and evaluates entering into a docking state if the 2nd parameter is true */
	void adjust_position ( UiManager* uim, bool docking=true );

	/* If this panel is a submenu, make it a float panel at given position */
	void become_float_submenu ( float x, float y );

	void set_style ( const UiStyle& s );

	void rebuild () { changed(NeedsRebuild); build(); }

	// Overriden virtual methods:
	virtual void change_style ( const UiStyle& s ) override;
	virtual void pos ( float x, float y ) override;
	virtual void move ( float dx, float dy ) override;
	virtual void add_key_commands ( UiManager* uim ) override;
	virtual void build () override;
	virtual void resize ( float w, float h ) override;
	virtual int  handle ( const GsEvent& e, UiManager* uim ) override;
	virtual void draw ( UiPanel* panel ) override;
	virtual void close () override;
};

//================================ End of File =================================================

# endif // UI_PANEL_H
