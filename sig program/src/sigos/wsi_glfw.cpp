/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

//=================================  SIG-GLFW integration  ==========================================

# include <sig/gs.h> // needed here to define macro below

# if defined (GS_LINUX) || defined (GS_GLFW)

# include <stdio.h>
# include <string.h>
# include <stdlib.h>

# include <sig/gs_array.h>
# include <sig/gs_buffer.h>
# include <sig/gs_output.h>
# include <sig/gs_string.h>
# include <sigogl/ws_window.h>
# include <sigogl/ws_osinterface.h>
# include <sigogl/gl_loader.h>
# include <sigogl/gl_resources.h>
# include <sigogl/ui_dialogs.h>

//# define GS_USE_TRACE_COUNTER
//# define GS_USE_TRACE1 // trace main functions
//# define GS_USE_TRACE2 // trace more functions
//# define GS_USE_TRACE3 // trace more info
//# define GS_USE_TRACE4 // get ogl procedure
//# define GS_USE_TRACE5 // events
//# define GS_USE_TRACE6 // more events
//# define GS_USE_TRACE7 // mouse move event
//# define GS_USE_TRACE8 // glfw callbacks
# include <sig/gs_trace.h>

# ifdef GS_MSWIN
# define WIN32_LEAN_AND_MEAN 1
# include <windows.h>
# include <Shlobj.h>
# include <Commdlg.h>
# define GLFW_INCLUDE_NONE
# endif // GS_MSWIN

# include <GLFW/glfw3.h>

//===== Global Data ===========================================================================

struct OsWin; // fwd decl
static GsBuffer<OsWin*> AppWindows;
static gsint16		AppNumVisWindows=0;
static gsint16		DialogRunning=0;

//===== Window Data ===========================================================================
struct OsWin
{	char*	 label;			// window label
	WsWindow* swin;			// sig window
	GsEvent  event;			// event being sent to window
	gscbool  justresized;	// flag to allow client area correction
	gscbool  needsinit;		// flag to call client init funtion before first paint
	gscbool  visible;		// visible state according to calls to show and hide
	gscbool	 isdialog;		// dialogs need special treatment
	gscbool  needsredraw;	// dialogs need special treatment
	//gscbool  canbedestroyed;
	GLFWwindow* gwin;		// glfw window handle
	OsWin() { label=0; swin=0; }//canbedestroyed=1; }
   ~OsWin(); // destructor
};

OsWin::~OsWin()
{
	GS_TRACE1 ( "OsWin Destructor ["<<label<<']' );
	if ( gwin ) { wsi_win_hide(this); glfwDestroyWindow(gwin); gwin=0; } // no further callbacks will be called for this window
	delete swin; // destructor will call wsi_del_win()
	free ( label );
	if ( isdialog ) DialogRunning--;
	for ( int i=0, s=AppWindows.size(); i<s; i++ )
	{	if ( AppWindows[i]==this ) { AppWindows.remove(i); break; } }
}

//===== Functions =============================================================================

void wsi_del_win ( void *win ) // this function is called by the WsWindow destructor, possibly by the user
{
	GS_TRACE1 ( "wsi_del_win ["<<((OsWin*)win)->label<<"]..." );
	OsWin* ow = (OsWin*)win;
	if ( ow->isdialog ) { DialogRunning--; ow->isdialog=0; } // important to allow other windows to receive events

	if ( ow->gwin ) // the user called this function
	{	ow->swin=0; // if user deletes sw, this is being called before OsWin destructor to stop a 2nd delete
		delete ow;
	}
	else // this call came from OsWin desctructor triggered by a glfw event
	{	// nothing more to do
	}
}

static void _init_callbacks ( GLFWwindow* win ); // will set all glfw callbacks, defined later

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error: %s\n", description);
}

void* wsi_new_win ( int x, int y, int w, int h, const char* label, WsWindow* swin, int mode )
{
	GS_TRACE1 ( "wsi_new_win ["<<label<<"]..." );

	if ( AppWindows.empty() ) // glfw initializations before creating first window
	{	glfwSetErrorCallback(error_callback);
		if ( !glfwInit() ) gsout.fatal("wsi_new_win: Could not init GLFW!");
	}

	GlResources::load_configuration_file (); // only overall 1st call will actually load config file

	OsWin* ow = new OsWin;
	# ifdef GS_MSWIN
	ow->label = _strdup ( label );
	# else
	ow->label = strdup ( label );
	# endif
	ow->swin = swin;
	ow->justresized = 1;
	ow->needsinit = 1;
	ow->event.width = w;
	ow->event.height = h;
	ow->visible = 0;
	ow->isdialog = mode;
	ow->needsredraw = 0;
	ow->gwin = 0;

	if ( mode>0 ) DialogRunning++;

	// update default sizes:
	const int minw=50, minh=50; // it seems windows may not always accept sizes smaller than about 100
	if ( w<minw ) w=minw;
	if ( h<minh ) h=minh;

	// dialog boxes are simulated with mode>0 (DialogBox() function does not return until callback function terminates)
	GS_TRACE1 ( "Creating window..." );
	glfwWindowHint ( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint ( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint ( GLFW_RED_BITS, 8 );
	glfwWindowHint ( GLFW_BLUE_BITS, 8 );
	glfwWindowHint ( GLFW_GREEN_BITS, 8 );
	glfwWindowHint ( GLFW_ALPHA_BITS, 8 );
	glfwWindowHint ( GLFW_DOUBLEBUFFER, 1 );
	glfwWindowHint ( GLFW_DEPTH_BITS, 32 );
	GLFWwindow* sharedwin = AppWindows.size()>0? AppWindows[0]->gwin:NULL;
	ow->gwin = glfwCreateWindow ( w, h, label, NULL, sharedwin );
	if ( ow->gwin==NULL) gsout.fatal ( "glfwCreateWindow failed!" );
	glfwSetWindowUserPointer ( ow->gwin, ow );

	if ( AppWindows.empty() ) // glfw initializations when creating first window
	{	glfwMakeContextCurrent ( ow->gwin );
		gl_load_and_initialize (); // only overall 1st call will actually load ogl
		//ui_set_dialog_parent_window ( ow->swin );
		//notinit=false;
	}
	//old call: _init_callbacks ( ow->gwin );

	int scw, sch;
	wsi_screen_resolution ( scw, sch );
	if ( x<0 ) x = (scw-w)/2;
	if ( y<0 ) y = (sch-h)/2;
	# if defined GLFW_VERSION_MAJOR>=3 && GLFW_VERSION_MINOR>=2
		glfwSetWindowSizeLimits ( ow->gwin, 16, 8, GLFW_DONT_CARE, GLFW_DONT_CARE ); // added in version 3.2.
	# endif
	glfwSetWindowPos ( ow->gwin, x, y );
	glfwSetWindowSize ( ow->gwin, w, h );
	_init_callbacks ( ow->gwin );

	AppWindows.push(ow);
	return (void*) ow;
}

const char* wsi_win_label ( void* win )
{
	GS_TRACE2 ( "wsi_win_label ["<<((OsWin*)win)->label<<"] label requested." );
	return ((OsWin*)win)->label;
}

void wsi_win_label ( void* win, const char* label )
{
	GS_TRACE2 ( "wsi_win_label ["<<((OsWin*)win)->label<<"] to "<<label<<"..." );
	OsWin* ow = (OsWin*)win;
	free ( ow->label );
	# ifdef GS_MSWIN
	ow->label = _strdup ( label );
	# else
	ow->label = strdup ( label );
	# endif
	glfwSetWindowTitle ( ow->gwin, ow->label );
}

void wsi_win_show ( void* win )
{
	GS_TRACE2 ( "wsi_win_show..." );
	OsWin* ow = (OsWin*)win;
	if ( !ow->visible ) { AppNumVisWindows++; ow->visible=1; }
	glfwShowWindow ( ow->gwin );
}

void wsi_win_hide ( void* win )
{
	GS_TRACE2 ( "wsi_win_hide..." );
	OsWin* ow = (OsWin*)win;
	if ( ow->visible ) { AppNumVisWindows--; ow->visible=0; }
	glfwHideWindow ( ow->gwin );
}

void wsi_win_move ( void* win, int x, int y, int w, int h )
{
	GS_TRACE2 ( "wsi_win_move..." );
	OsWin* ow = (OsWin*)win;
	ow->justresized = true;
	glfwSetWindowPos ( ow->gwin, x, y );
	glfwSetWindowSize( ow->gwin, w, h );
}

void wsi_win_pos ( void* win, int& x, int& y )
{
	GS_TRACE2 ( "wsi_win_pos..." );
	OsWin* ow = (OsWin*)win;
	glfwGetWindowPos ( ow->gwin, &x, &y );
}

void wsi_win_size ( void* win, int& w, int& h )
{
	GS_TRACE2 ( "wsi_win_size..." );
	OsWin* ow = (OsWin*)win;
	glfwGetWindowSize ( ow->gwin, &w, &h );
	GS_TRACE3 ( "got size "<<w<<"x"<<h );
}

bool wsi_win_visible ( void* win )
{
	GS_TRACE2 ( "wsi_win_visible..." );
	return (bool) glfwGetWindowAttrib ( ((OsWin*)win)->gwin, GLFW_VISIBLE );
}

bool wsi_win_minimized ( void* win )
{
	GS_TRACE2 ( "wsi_win_minimized..." );
	return (bool) glfwGetWindowAttrib ( ((OsWin*)win)->gwin, GLFW_ICONIFIED );
}

void wsi_activate_ogl_context ( void* win )
{
	GS_TRACE2 ( "wsi_set_ogl_context..." );
	glfwMakeContextCurrent ( ((OsWin*)win)->gwin );
}

int wsi_num_windows ()
{
	GS_TRACE2 ( "wsi_num_windows..." );
	return AppNumVisWindows;
}

void* wsi_get_ogl_procedure ( const char *name )
{
	GS_TRACE4 ( "wsi_get_ogl_procedure..." );
	void* pt = (void *)glfwGetProcAddress(name);
	return pt;
}

void wsi_screen_resolution ( int& w, int& h )
{
	const GLFWvidmode* m = glfwGetVideoMode ( glfwGetPrimaryMonitor() );
	w = m->width;
	h = m->height;
}

//==== Callbacks ==============================================================================

// Friend functions of WsWindow enabling access to protected methods:
inline void osdraw ( WsWindow* win ) { win->draw(win->_glrenderer); };
inline void osinit ( WsWindow* win, int w, int h ) { win->init(win->_glcontext,w,h); }
inline void osresize ( WsWindow* win, int w, int h ) { win->resize(win->_glcontext,w,h); }
inline void osevent ( WsWindow* win, GsEvent& e ) { win->handle(e); }

int wsi_check ()
{
	glfwPollEvents();
	for ( int i=0; i<AppWindows.size(); i++ )
	{	OsWin* ow = AppWindows[i];
		if ( ow->needsredraw)
		{	GS_TRACE8 ( "drawing..." );
			GLFWwindow* gwin = ow->gwin;
			WsWindow* swin = ow->swin;
			glfwMakeContextCurrent ( gwin );
			if (ow->needsinit)
			{	int w, h;
				glfwGetWindowSize ( gwin, &w, &h );
				osinit ( swin, w, h );
				ow->needsinit=0;
			}
			osdraw ( swin ); // this will call the user's window draw method
			glfwSwapBuffers ( gwin );
			ow->needsredraw=0;
		}
	}
	return AppNumVisWindows;
}

static void draw_cb ( GLFWwindow* gwin )
{
	GS_TRACE8 ( "draw_cb" );
	((OsWin*)glfwGetWindowUserPointer(gwin))->needsredraw=1;
}

void wsi_win_redraw ( void* win )
{
	GS_TRACE8 ( "wsi_win_redraw..." );
	((OsWin*)win)->needsredraw=1;
}

void resize_cb ( GLFWwindow* gwin, int w, int h )
{
	GS_TRACE8 ( "resize_cb" );
	OsWin* ow = (OsWin*)glfwGetWindowUserPointer(gwin);
	WsWindow* swin = ow->swin;
	glfwMakeContextCurrent ( gwin );
	osresize ( swin, w, h );
}

//==== Event Callbacks ==============================================================================

static void setmodifs ( GsEvent& e, int modifs )
{
	e.alt = modifs&GLFW_MOD_ALT? 1:0;
	e.ctrl = modifs&GLFW_MOD_CONTROL? 1:0;
	e.shift = modifs&GLFW_MOD_SHIFT? 1:0;
	//gsout<<modifs<<": A:"<<e.alt<<" C:"<<e.ctrl<<" S:"<<e.shift<<gsnl;
}

static void setkeycode ( GsEvent& e, int key )
{
	//const gsbyte NS[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };
	if ( key<=162 ) e.key=key;
	if ( key>='A' && key<='Z') { e.character=(gsbyte)(e.shift?key:key-'A'+'a'); return; }

	e.character = 0;

	# define RET(x) e.key=GsEvent::x; return
	# define RET1(x) e.key=x; e.character=(gsbyte)x; return
	# define RET2(x,y) e.key=x; e.character=(gsbyte)(e.shift?y:x); return
	switch (key)
	{	case GLFW_KEY_LEFT	: RET(KeyLeft); case GLFW_KEY_RIGHT	: RET(KeyRight);
		case GLFW_KEY_UP	: RET(KeyUp);	case GLFW_KEY_DOWN	: RET(KeyDown);
		case GLFW_KEY_ESCAPE: RET(KeyEsc);	case GLFW_KEY_BACKSPACE	: RET(KeyBack);
		case GLFW_KEY_HOME	: RET(KeyHome); case GLFW_KEY_END	: RET(KeyEnd);
		case GLFW_KEY_TAB	: RET1('\t');	case GLFW_KEY_ENTER	: RET(KeyEnter);
		case GLFW_KEY_INSERT: RET(KeyIns);	case GLFW_KEY_DELETE: RET(KeyDel);
		case GLFW_KEY_PAGE_UP: RET(KeyPgUp); case GLFW_KEY_PAGE_DOWN: RET(KeyPgDn);

		case GLFW_KEY_KP_DIVIDE	: RET1('/'); case GLFW_KEY_KP_MULTIPLY: RET1('*');
		case GLFW_KEY_KP_ADD	: RET1('+'); case GLFW_KEY_KP_SUBTRACT: RET1('-');
		case 192: RET2('`','~'); case 189: RET2('-','_');  case 187: RET2('=','+');
		case 219: RET2('[','{'); case 221: RET2(']','}');  case 220: RET2('\\','|');
		case 186: RET2(';',':'); case 222: RET2('\'','"'); case 188: RET2(',','<');
		case 190: RET2('.','>'); case 191: RET2('/','?');
	}
	if ( key>=GLFW_KEY_F1 && key<=GLFW_KEY_F12 ) e.key=GsEvent::KeyF1+(key-GLFW_KEY_F1);
}

static void key_cb ( GLFWwindow* gwin, int key, int scancode, int action, int modifs )
{
	GS_TRACE8 ( "key_cb" );
	OsWin* ow = (OsWin*)glfwGetWindowUserPointer(gwin);
	GsEvent& e = ow->event;
	if ( key>=GLFW_KEY_LEFT_SHIFT && action==GLFW_REPEAT ) return; // do not send repeated modifier event
	e.type = action==GLFW_RELEASE? GsEvent::KeyRelease : GsEvent::Keyboard;
	e.wheelclicks = 0; 
	e.button = 0;
	setkeycode ( e, key );
	setmodifs ( e, modifs );
	if ( key==GLFW_KEY_LEFT_ALT || key==GLFW_KEY_RIGHT_ALT ) { e.alt = action==GLFW_PRESS? 1:0; }
	else if ( key==GLFW_KEY_LEFT_CONTROL || key==GLFW_KEY_RIGHT_CONTROL ) { e.ctrl = action==GLFW_PRESS? 1:0; }
	else if ( key==GLFW_KEY_LEFT_SHIFT || key==GLFW_KEY_RIGHT_SHIFT ) { e.shift = action==GLFW_PRESS? 1:0; }
	//gsout<<modifs<<": A:"<<e.alt<<" C:"<<e.ctrl<<" S:"<<e.shift<<gsnl;
	osevent ( ow->swin, e );
}

static void setmouseev ( GLFWwindow* gwin, GsEvent& e, GsEvent::Type t )
{
	e.type = t;
	e.button = 0;
	e.wheelclicks = 0; 
	e.key = 0;
	glfwGetFramebufferSize ( gwin, &e.width, &e.height ); // alternative: glfwGetWindowSize
	e.button1 = glfwGetMouseButton(gwin,GLFW_MOUSE_BUTTON_LEFT);
	e.button2 = glfwGetMouseButton(gwin,GLFW_MOUSE_BUTTON_MIDDLE);
	e.button3 = glfwGetMouseButton(gwin,GLFW_MOUSE_BUTTON_RIGHT);
}

static void setmousepos ( GsEvent& e, gsint16 x, gsint16 y )
{
	e.lmousex=e.mousex;	e.lmousey=e.mousey;
	e.mousex=x;	e.mousey=y;
}

static void mouse_cb ( GLFWwindow* gwin, int but, int action, int modifs )
{
	GS_TRACE8 ( "mouse_cb" );
	OsWin* ow = (OsWin*)glfwGetWindowUserPointer(gwin);
	GsEvent& e = ow->event;
	setmouseev ( gwin, e, action==GLFW_PRESS? GsEvent::Push : GsEvent::Release );
	setmodifs ( e, modifs );
	e.button = but==GLFW_MOUSE_BUTTON_LEFT? 1 : but==GLFW_MOUSE_BUTTON_RIGHT? 3 : 2;
	double x, y;
	glfwGetCursorPos ( gwin, &x, &y );
	setmousepos ( e, (gsint16)x, (gsint16)y );
	osevent ( ow->swin, e );
}

static void mousepos_cb ( GLFWwindow* gwin, double x, double y )
{
	GS_TRACE8 ( "mousepos_cb" );
	OsWin* ow = (OsWin*)glfwGetWindowUserPointer(gwin);
	GsEvent& e = ow->event;
	setmouseev ( gwin, e, GsEvent::Move );
	if ( e.button1 || e.button3 ) e.type=GsEvent::Drag;
	setmousepos ( e, (gsint16)x, (gsint16)y );
	osevent ( ow->swin, e );
}

static void close_cb ( GLFWwindow* gwin )
{
	GS_TRACE8 ( "close_cb" );
	OsWin* ow = (OsWin*)glfwGetWindowUserPointer(gwin);
	delete ow;
}

static void _init_callbacks ( GLFWwindow* gwin )
{
	glfwSetKeyCallback ( gwin, key_cb );
	glfwSetWindowRefreshCallback ( gwin, draw_cb );
	glfwSetWindowSizeCallback ( gwin, resize_cb );
	glfwSetMouseButtonCallback ( gwin, mouse_cb );
	glfwSetCursorPosCallback ( gwin, mousepos_cb );
	glfwSetWindowCloseCallback ( gwin, close_cb );

//TO LATER CALL:	glfwTerminate();

//
//GLFWAPI GLFWcharfun glfwSetCharCallback(GLFWwindow* window, GLFWcharfun cbfun);
//GLFWAPI GLFWcharmodsfun glfwSetCharModsCallback(GLFWwindow* window, GLFWcharmodsfun cbfun);
//
//GLFWAPI GLFWmonitorfun glfwSetMonitorCallback(GLFWmonitorfun cbfun);
//GLFWAPI GLFWwindowposfun glfwSetWindowPosCallback(GLFWwindow* window, GLFWwindowposfun cbfun);
//GLFWAPI GLFWwindowfocusfun glfwSetWindowFocusCallback(GLFWwindow* window, GLFWwindowfocusfun cbfun);
//GLFWAPI GLFWwindowiconifyfun glfwSetWindowIconifyCallback(GLFWwindow* window, GLFWwindowiconifyfun cbfun);
//GLFWAPI GLFWframebuffersizefun glfwSetFramebufferSizeCallback(GLFWwindow* window, GLFWframebuffersizefun cbfun);
//GLFWAPI GLFWcursorposfun glfwSetCursorPosCallback(GLFWwindow* window, GLFWcursorposfun cbfun);
//GLFWAPI GLFWcursorenterfun glfwSetCursorEnterCallback(GLFWwindow* window, GLFWcursorenterfun cbfun);
//GLFWAPI GLFWscrollfun glfwSetScrollCallback(GLFWwindow* window, GLFWscrollfun cbfun);
//GLFWAPI GLFWdropfun glfwSetDropCallback(GLFWwindow* window, GLFWdropfun cbfun);
//GLFWAPI GLFWjoystickfun glfwSetJoystickCallback(GLFWjoystickfun cbfun);
//
//GLFWAPI const char* glfwGetKeyName(int key, int scancode);
//GLFWAPI int glfwGetKey(GLFWwindow* window, int key);
//GLFWAPI int glfwGetMouseButton(GLFWwindow* window, int button);
//GLFWAPI void glfwGetCursorPos(GLFWwindow* window, double* xpos, double* ypos);
//

}


//
//static void setstate ( GsEvent& e )
//{
//	e.shift = GetKeyState(VK_SHIFT)&128? 1:0;
//	e.ctrl  = GetKeyState(VK_CONTROL)&128? 1:0;
//	e.alt   = GetKeyState(VK_MENU)&128? 1:0;
//	e.button1 = GetKeyState(VK_LBUTTON)&128? 1:0;
//	e.button2 = GetKeyState(VK_MBUTTON)&128? 1:0;
//	e.button3 = GetKeyState(VK_RBUTTON)&128? 1:0;
//}
//

//=============================== native file dialogs ==========================================

# define FILEBUFSIZE 256
static GsString FileBuf;

// cal call popen ( zenity --file-selection )

// filter format:  "*.txt;*.log"
static const char* filedlg ( bool open, const char* msg, const char* file, const char* filter, GsArray<const char*>* multif )
{
	return 0;
 }

const char* wsi_open_file_dialog ( const char* msg, const char* file, const char* filter, GsArray<const char*>* multif )
{
	return filedlg ( true, msg, file, filter, multif );
}

const char* wsi_save_file_dialog ( const char* msg, const char* file, const char* filter )
{
	return filedlg ( false, msg, file, filter, 0 );
}

const char* wsi_select_folder ( const char* msg, const char* folder )
{
	return 0;
}

//=============================== misc windows functions ==========================================

// Main Function Arguments:
static GsBuffer<char*> AppArgs;

char** wsi_program_argv ()
{
	return &AppArgs[0];
}

int wsi_program_argc ()
{
	return AppArgs.size()-1;
}

// It is also possible to run this glfw interface in windows:
# ifdef GS_MSWIN
extern int main ( int, char** ); // the main function entry point must be provided elsewhere
int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	GS_TRACE1 ( "WinMain..." );
	//AppInstance = hInstance;
	//AppPrevInstance = hPrevInstance;

	// Converting command line arguments to argv format:
	char* pt = GetCommandLine();
	if ( *pt=='"' ) pt++; // executable name may come between quotes, remove first one here
	while ( true )
	{	AppArgs.push(pt);
		while ( *pt && *pt!=' ' ) pt++; // skip argument
		if ( AppArgs.size()==1 && *(pt-1)=='"' ) *(pt-1)=0; // remove end quote in executable name
		if ( *pt==0 ) break;
		*pt++ = 0; // mark end of argument
		while ( *pt && *pt==' ' ) pt++; // skip any extra spaces
		if ( *pt==0 ) break;
	}
	AppArgs.push(0); // for compatibility last argv should be null

	// call the user-provided main:
	return main ( AppArgs.size()-1, &AppArgs[0] );

	// Reminder: SIG config file loading and OpenGL initialization are performed
	// automatically at the time of opening the first graphical window.
}
# endif // GS_MSWIN

# endif // GS_LINUX

//================================ End of File =================================================

