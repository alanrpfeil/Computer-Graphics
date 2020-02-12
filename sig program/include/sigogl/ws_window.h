/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file ws_window.h
 * System wrapper for window management
 */

# ifndef WS_WINDOW_H
# define WS_WINDOW_H

# include <sig/gs_event.h>
# include <sigogl/ui_manager.h>
# include <sigogl/gl_renderer.h>

/*! \class WsWindow ws_window.h
	\brief System wrapper for window management
	WsWindow provides a minimalistic interface for opening an OpenGL window. 
	The first WsWindow open will share its OpenGL context with all others, 
	therefore the first window may only be destroyed when the application closes. */
class WsWindow
 { private : // internal data
	void  *_syswin;
	double _creationtime;
	int _w, _h;
	GlContext* _glcontext;
	GlRenderer* _glrenderer;
	UiManager* _uim;

   private : // declarations to allow the os to interface with the window:
	friend void osdraw ( WsWindow* );
	friend void osinit ( WsWindow*, int, int );
	friend void osevent ( WsWindow*, GsEvent& );
	friend void osresize ( WsWindow*, int, int );
	friend UiManager;

   public : //----> public definitions
	enum Mode { Normal, Dialog }; //!< possible types of window creation

   protected: //----> protected methods
	/*! Returns the OpenGL renderer attached to this window. 
		Note that certain methods of GlRenderer must only be called when the OpenGL
		context is active. This is always the case when the draw() method is called.
		The renderer is used to render any user interface attached to the window. */
	GlRenderer* glrenderer () const { return _glrenderer; }

   public : //----> public methods 

	/*! Activates the OpenGL context of the window. Only needed in special cases
		where OpenGL commands are needed outside of the draw method */
	void activate_ogl_context () const;

	/*! Constructor needs the client area size, location of the window, and a label. */
	WsWindow ( int x, int y, int w, int h, const char *label=0, Mode m=Normal );

	/*! Deletes all internal data including the associated OS window and any associated timers. */
	virtual ~WsWindow ();

	/*! Returns a pointer to the user interface manager of the window. The window 
		starts with an empty user interface but the pointer will always be valid. */
	UiManager* uim () { return _uim; }

	/*! Show the window. */
	void show ();

	/*! Hide the window. */
	void hide ();

	/*! Returns the visibility state of the window. */
	bool visible ();

	/*! Returns true if the window is minimized, false otherwise. */
	bool minimized ();

	/*! Returns the label (title) of the window */
	const char* label () const;

	/*! Sets the label (title) of the window */
	void label ( const char* l );

	/*! Notify that this window needs to receive a draw event as soon as possible. */
	void redraw ();

	/*! Move the window, if a parameter is <0 its respective dimension is not changed. */
	void move ( int x, int y, int w, int h );

	/*! Move the window so that it becomes centered in the primary screen.
		If w and h are given >0, the window is also resized */
	void center ( int w=-1, int h=-1 );

	/*! Returns the time in seconds since the creation of this window */
	double time () { return gs_time()-_creationtime; }

	/*! Returns the width of the window */
	int w () const { return _w; }

	/*! Returns the height of the window */
	int h () const { return _h; }

	/*! Returns the creation time of the window in seconds */
	double creation_time () const { return _creationtime; }

	/*! Save a snapshot of the current OpenGL buffer in .bmp format.
		If null or an empty string is given, a dialog will ask for the file name. */
	bool snapshot ( const char* filename );

	/*! Register an event to be called at regular intervals given in seconds. */
	void add_timer ( double interval, int ev );

	/*! Remove the previously inserted timer. */
	void remove_timer ( int ev );

   protected: //----> protected virtual methods to be extended by derived classes

	/*! Method init will be called by the system when the window is first displayed,
		or when the focus changes from another window to this window. Because the
		OpenGL context is shared between windows, here is the place to set the 
		window-specific configuration, in particular the OpenGL viewport. */
	virtual void init ( GlContext* c, int w, int h );

	/*! Method resize will be called by the system every time the window is resized.
		The WsWindow implementation updates the OpenGL viewport and uim, and locally stores 
		the new window size */
	virtual void resize ( GlContext* c, int w, int h );

	/*! Method draw() should never be called directly; it will be called by the system
		every time the window needs to be updated and is ready for OpenGL calls. 
		The WsWindow implementation just calls the render() method of the user interface
		manager. */
	virtual void draw ( GlRenderer* r );

   public: //----> public virtual methods to be extended by derived classes

	/*! Entry point of events coming from the window manager. The WsWindow implementation
		first sends the event to the UI manager (if it exists) and if the event is not used
		its mouse coordinates are normalized to [-1,1] with center (0,0). An extension
		of this method should first call this method before processing its own events. 
		Method handle() returns 1 if the event was used and zero otherwise. */
	virtual int handle ( const GsEvent &e );

	/*! This method will be called by the UI manager every time a UI event is generated.
		This method should return 1 if the event was used and zero otherwise.
		The WsWindow implementation is empty and simply returns 0. */
	virtual int uievent ( int e );

	/*! This method will be called by timers installed with a ws_add_timer() function. 
		This method should return 1 if the event was used and zero otherwise.
		The WsWindow implementation is empty and simply returns 0. */
	virtual int timer ( int e );
};

//================================ End of File =================================================

# endif // WS_WINDOW_H
