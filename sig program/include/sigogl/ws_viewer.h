/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file ws_viewer.h
 * System wrapper for an opengl viewer
 */

# ifndef WS_VIEWER_H
# define WS_VIEWER_H

# include <sigogl/ws_window.h>
# include <sig/gs_color.h>
# include <sig/gs_quat.h>
# include <sig/gs_camera.h>

class SnNode;
class GsQuat;
class GsLight;
class GsEvent;
class WsViewerData;

/*! \class WsViewer ws_viewer.h
	\brief An opengl viewer

	WsViewer provides visualization for a graphkim scene graph. 
	It uses its own internal GlRenderer for rendering the scene, while
	it re-uses the WsWindow renderer to render the UI components created by the viewer. */
class WsViewer : public WsWindow 
 {
   public : // enumerators

	enum ViewMode { ModeExaminer, ModePlanar };

	enum RenderMode { ModeAsIs,
					  ModeDefault,
					  ModePhong,
					  ModeGouraud,
					  ModeFlat,
					  ModeLines,
					  ModePoints
					};

	/*! MenuCmds are negative in order to not conflict with user events, which should start at 0.
		ViewerCmd events start at -400, see comments in UiManager::ManagerCmd about ranges of events. */
	enum ViewerCmd { VCmdHelp=-400,	//!< display the help window
				   VCmdViewAll,		//!< adjust camera to viewl entire scene
				   VCmdBackground,	//!< change the backgournd color
				   VCmdExaminer,	//!< examiner model manipulation
				   VCmdPlanar,		//!< planar manipulation
				   VCmdAsIs,		//!< restore original mode specified in each scene shape
				   VCmdDefault,		//!< set default mode for each scene shape
				   VCmdPhong,		//!< set per-pixel smooth Phong shading
				   VCmdGouraud,		//!< set smooth shading by interpolation of Phong-illuminated vertices
				   VCmdFlat,		//!< set flat shading mode
				   VCmdLines,		//!< set line rendering mode
				   VCmdPoints,		//!< set each node to point drawing mode
				   VCmdAxis,		//!< display the global axis
				   VCmdBoundingBox,	//!< display the scence bounding box
				   VCmdStatistics,	//!< display rendering statistics
				   VCmdSpinAnim		//!< turn on/off spin animation
				};

   private : // internal data

	WsViewerData *_data;

   public : //----> public methods 

	/*! Constructor needs the size and location of the window. */
	WsViewer ( int x, int y, int w, int h, const char *label=0 );

	/*! Destructs all internal data, and calls unref() for the root node. */
   ~WsViewer ();

	/*! Rebuilds the default viewer user interface. */
	void build_ui ();

	/*! Shows the help dialog box */
	void show_help();

	/*! Executes one of the Ctrl+Shift+c key commands. Method show_help() shows
		the list of commands available. */
	int key_cmd ( char c );

	/*! Access to the scene root (without calling unref() for it).
		WsViewer initializes an empty SnGroup to be used as the user's root node,
		but because the user can change it to any node, a SnNode is returned here. */
	SnNode* root () const;

	/*! This access to the scene root is an utility which applies a type cast to a SnGroup.
		It can only be called if the user has not changed the type of the root node. */
	SnGroup* rootg () const;

	/*! Changes the scene root pointer. When the new node r is given, r->ref() is 
		called, and the old root node has its unref() method called. If r is null,
		an empty SnGroup is created and used as root. */
	void root ( SnNode* r );

	/*! Set a text to appear in the status bar of the graphics window. 
		Use s as 0 to erase current message. */
	void message ( const char* s ) { message().set(s); }

	/*! Access the string of the status bar message text for direct manipulation. */
	GsString& message ();

	/*! Access the string of the status bar message text only for inspection. */
	const GsString& cmessage () const;

	/*! Change the color of the message text */
	void message_color ( GsColor c );

	/*! Set a possibly multi-line text to appear at the graphics window. 
		Use s as 0 to erase current message. */
	void output ( const char* s );

	/*! Access the string of the in-window output text. */
	GsString& output ();

	/*! Set position for the output text in windows coordinates, default is 80,40 */
	void output_pos ( int x, int y );

	/*! Change the color of the output text */
	void output_color ( GsColor c );

	/*! Change the increment amount when zooming*/
	void zoomfactor (float z);

	/*! Executes a command available from the right button menu of the viewer. 
		Returns 1 if the cmd was understood and 0 otherwise. */
	int cmd ( ViewerCmd c );

	/*! Returns true if the cmd is currently activated. */
	bool cmd_activated ( ViewerCmd c );

	/*! Returns the most recently computed bounding box of the scene. */
	GsBox bbox () const;

	/*! Forces to recompute the bounding box of the scene, displayable with cmd();
		returns the computed box. */
	void update_bbox ( GsBox* boxpt=0 );

	/*! Forces to recompute the axis lenght of the scene, displayable with cmd().
		Returns the semiaxis length, which is half the longest bounding box side length.
		If boxpt is null the scene is traversed for computing the axis length. */
	float update_axis ( GsBox* boxpt=0 );

	/*! Sets the camera to see the whole bounding box of the scene. The camera
		center is set at the center of the bounding box, and the eye is set at
		the (0,0,2) point. The scene transformations (of Camview) are then computed
		in order to place the entire scene to be visible from the camera.
		If boxpt is given it is used instead of traversing the scene. */
	void view_all (  GsBox* boxpt=0 );
	
	/*! Sets the camera to see the bounding box of a node. The camera
		center is set at the center of the bounding box*/
	void view_node ( SnNode* n, float fovy );

	/*! Will make WsViewer to render the scene in the next event loop. If the
		current scene is spinning, then the scene is already being rendered by the
		spin animation timeout with a defined frequency, an in this case a call to 
		render() will have no effect. */
	void render ();

	/*! Returns true if the scene is spinning, false otherwise. When the scene is 
		spinning, then a spin animation timeout function is created in fltk that
		stays calling render() with a calculated frequency. */
	bool spinning () ;

	/*! Sets data to be considered by the spin animation. */
	void set_spin_data ( const GsQuat &delta, float interval, float activation=0.1f );

	/*! Gets the current spin animation data being used. */
	void get_spin_data ( GsQuat &delta, float &interval, float& activation );

	/*! Forces spin animation to start or stop, using current spin data.
		For disabling/enabling trigering spin animation with the mouse,
		use cmd(CmdSpinAnim) */
	void spinning ( bool onoff );

	/*! Applys an orbit rotation to the camera. If a timestamp is given it is
		used to determine spin animation start and speed */
	void rotate_camera ( const GsQuat &dq, double timestamp=-1 );

	/*! Returns the current frames per seconds rate achieved */
	double fps ();

	/*! Returns the number of the last frame rendered */
	gsuint curframe ();

	/*! Returns the current background color */
	GsColor background ();

	/*! Sets a new background color */
	void background ( GsColor c );

	/*! Returns the current view mode */
	ViewMode viewmode ();

	/*! Access to the camera parameters */
	GsCamera& camera ();

	/*! Sets a new camera to be used and resets any current spin animation */
	void camera ( const GsCamera &cam );

	/*! Returns the number of lights in use, the default value is 1 */
	int num_lights () const;

	/*! Sets number of lights to use. The given value will be corrected to be in range
		1 <= n <= GlContext::MaxLights before being used. */
	void num_lights ( int n );

	/*! Returns a reference to light index i, 0<=i<max lights, and mark light as changed. */
	GsLight& light ( int i=0 );

	/*! Sets new light parameters for light index i and mark the light as changed. */
	void light ( const GsLight& l, int i=0 );

	/*! Exports all GsModels in the scene to files, and in global coordinates */
	void export_all_models ( const char* prefix=0, const char* dir=0 );

	/*! Turns on or off snapshot saving per frame. Optional parameters specify the
		desired base file name and the start number (>1) for enumerating files.
		The file name extension defines the image format: bmp, tga, or otherwise png. */
	void snapshots ( bool onoff, const char* file=0, int n=-1 );

   protected : //----> methods overriding WsWindow virtual methods

	/*! Extends WsWindow::init() and initializes OpenGL depth test, back face culling, 
		light 0 (if using 1.1 API), transparency blending, anti-aliasing and camera settings. */
	virtual void init ( GlContext* c, int w, int h ) override;

	/*! Extends WsWindow::resize() to update the aspect ratio of the camera. */
	virtual void resize ( GlContext* c, int w, int h ) override;

	/*! Method draw() sets the viewer options and render the scene root.
		This is a derived method and should not be called directly.
		To draw the window contents, use render() instead. */
	virtual void draw ( GlRenderer* wr ) override;

	/*! Method handle() processes all basic events and will then call the
		specialized handle methods below according to the generated event. 
		This is the handle event entry point overriding WsWindow::handle(e). */
	virtual int handle ( const GsEvent& e ) override;

	/*! Method uievent() handles commands generated by the menu attached to a right-button
		click of the mouse. The event codes are of MenuCmd type and are all negative. */
	virtual int uievent ( int e ) override;

   public : //----> virtual methods allowing the user to override WsViewer functionality

   /*! This method is called to route mouse events to the different camera
		manipulation handlers or to the scence. */
	virtual int handle_mouse ( const GsEvent& e );

	/*! Takes mouse events to rotate, scale and translate the scene. 
		If the event is not used, it is passed to the scene by calling
		handle_scene_event(). */
	virtual int handle_examiner_manipulation ( const GsEvent& e );

	/*! Takes mouse events to rotate, scale and translate the scene in planar mode. 
		If the event is not used, it is passed to the scene by calling
		handle_scene_event(). */
	virtual int handle_planar_manipulation ( const GsEvent& e );
   
	/*! Applies an event action to the scene */
	virtual int handle_scene_event ( const GsEvent& e );

   /*! All key press events are passed to this method. The WsViewer
	   implementation checks if crtl+shift+m is pressed to display
	   the mouse menu, crtl+shift+x to exit the application, 
	   crtl+shift+e to call the eps export action;
	   otherwise it passes the event to the scene graph. */
	virtual int handle_keyboard ( const GsEvent& e );

	/*! All key release events are passed to this method.
		Currently the WsViewer implementation is empty and always returns 0. */
	virtual int handle_key_release ( const GsEvent& e );

	/*! Will be called each time a spin animation accured. The WsViewer
		implementation of this virtual method does nothing. */
	virtual void spin_animation_occured ();

	/*! Called when it is time to set the OpenGL light.
		The viewer uses one GsLight object and this method automatically sets the light
		attenuation to be the inverse of the (Camview) scene scale and makes sure the
		light is not directional. Only called when using OpenGL 1.1 API. */
	virtual void set_light ();
};

//================================ End of File =================================================

# endif // WS_VIEWER_H
