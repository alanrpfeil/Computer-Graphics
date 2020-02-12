/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_EVENT_H
# define GS_EVENT_H

/** \file gs_event.h 
 * A user-generated window-event
 */

# include <sig/gs_vec.h>
# include <sig/gs_line.h>
# include <sig/gs_output.h>

/*! \class GsEvent gs_event.h
	\brief a window event

	GsEvent is used to describe a mouse or keyboard event, in a
	system-independent way, for sending to the Gs scene graph. */
class GsEvent 
{  public :
	/*! Enumerators for the type of event. */
	enum Type { None,		//!< No event occured.
				Push,		//!< A mouse button was pushed.
				Release,	//!< A mouse button was released.
				Drag,		//!< The mouse moved with a button down.
				Move,		//!< The mouse moved without a button down.
				Wheel,		//!< The mouse wheel rotated.
				Keyboard,	//!< A key was pressed.
				KeyRelease	//!< A key was released.
			  };

	/*! Enumerators with codes for special keys. */
	enum KeyCodes { KeyF1=1, KeyF2, KeyF3, KeyF4, KeyF5, KeyF6,
					KeyF7, KeyF8, KeyF9, KeyF10, KeyF11, KeyF12,
					KeyEsc=65307, KeyDel=65535, KeyIns=65379, KeyBack=65288,
					KeyUp=65362, KeyDown=65364, KeyLeft=65361, KeyRight=65363,
					KeyEnd=65367, KeyPgDn=65366, KeyPgUp=65365, KeyHome=65360,
					KeyShift=653505, KeyCtrl=65507, KeyAlt=65313, KeySpace=32,
					KeyEnter=65293 };

   public : //--- event occured :
	Type type;	 //!< The type of the occured event
	int  key;	 //!< The ascii code / code of the key pressed (lowercase) if it was a keyboard event, 0 otherwise
	int  width;  //!< The width of the window when the event occured
	int  height; //!< The height of the window when the event occured
	int  wheelclicks; //!< Number of clicks the wheel rotated, sign indicates direction, 0 if not event occurred
	char button;	  //!< The button number 1, 2 or 3 for: left, midlle, right, if event type was mouse related, 0 otherwise
	gsbyte character; //!< Character code taking into account the shift state; 0 if not a keyboard event or not a character

   public : //--- states at event time :
	gscbool button1; //!< Contains 1 if the left mouse button state is pressed, 0 otherwise
	gscbool button2; //!< Contains 1 if the middle mouse button state is pressed, 0 otherwise
	gscbool button3; //!< Contains 1 if the right mouse button state is pressed, 0 otherwise
	gscbool alt;	 //!< Contains 1 if the Alt modifier key state is pressed, 0 otherwise
	gscbool ctrl;	 //!< Contains 1 if the Ctrl modifier key state is pressed, 0 otherwise
	gscbool shift;   //!< Contains 1 if the Shift modifier key state is pressed, 0 otherwise

   public : //--- mouse coordinates :
	gsint16 mousex, mousey;   //!< Current mouse position in window coordinates
	gsint16 lmousex, lmousey; //!< Last mouse position in window coordinates

   public : //--- scene-related information :
	mutable GsVec2 mouse;	//!< Current mouse position in normalized coords [-1,1]
	mutable GsVec2 lmouse;  //!< Last mouse position in normalized coords [-1,1]
	mutable GsPnt  mousep;  //!< mouse point in scene coordinates at plane z=0
	mutable GsPnt  lmousep; //!< last mouse point in scene coordinates at plane z=0
	mutable GsLine ray;		//!< current mouse ray in scene coordinates
	mutable GsLine lray;	//!< last mouse ray in scene coordinates
	mutable float pixelsize; //!< the average size of a screen pixel in scene units

   public : //--- methods :

	/*! Initialize as a None event type, by calling init(). */
	GsEvent ();

	/*! Makes the event to be of None type, stores mouse in lmouse variables, 
		and initializes the event-related data members. */
	void init ();

	/*! Returns a string with the name of the event type. */
	const char *type_name () const;

	/*! Set mouse coordinates to normalized coordinates in [-1,1] and center (0,0) */
	void normalize_coordinates ( int w, int h ) const;

	/*! Returns the difference: mousex-lmousex. */
	gsint16 mousedx () const { return mousex-lmousex; }

	/*! Returns the difference: mousey-lmousey. */
	gsint16 mousedy () const { return mousey-lmousey; }

	/*! Returns the difference: mousep-lmousep. */
	GsVec mousedp () const { return mousep-lmousep; }

	/*! Returns true if the event type is push, drag, release or wheel; and false otherwise. */
	bool mouse_event () const { return type>=Push&&type<=Wheel; }

	/*! Returns true if the event type is push, release or drag; and false otherwise. */
	bool button_event () const { return type>=Push&&type<=Drag; }

	/*! Outputs data of this event for data inspection. */
	friend GsOutput& operator<< ( GsOutput& out, const GsEvent& e );
};

//================================ End of File =================================================

# endif // GS_EVENT_H

