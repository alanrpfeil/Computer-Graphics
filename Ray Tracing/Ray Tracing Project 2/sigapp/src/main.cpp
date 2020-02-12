
# include "my_viewer.h"

# include <sigogl/ws_run.h>
# include <sigogl/gl_tools.h>

int main ( int argc, char** argv )
{
	// Initialize our viewer:
	MyViewer* v = new MyViewer ( -1, -1, 640, 480, "SnMyNode Test" );
	v->cmd ( WsViewer::VCmdAxis );
	v->view_all ();
	v->show ();

	if ( false ) // set this to true to display OpenGL info in the console
	{	// OpenGl is initialized when the first window is open, here we force
		// the viewer to open with ws_check() to then print the OpenGL version:
		ws_check(); // process next event, triggering OpenGL initialization
		v->activate_ogl_context(); // we need an active context
		gl_print_info(); // print info
		gsout << "Running application..." << gsnl; // send messages to the console with gsout
	}

	// Now just run the application:
	ws_run ();
	return 1;
}
