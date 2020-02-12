/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef WS_OSINTERFACE_H
# define WS_OSINTERFACE_H

class WsWindow; // forward declaration of WsWindow

// This is an internal header file to connect to the OS window system
// These functions should not be called directly by the user

//========== Window ===========

// creates window: x,y,w,h may be <=0, label has to be a valid string, uwindow is simply stored
// w,h is the client area size, not the outer window size; mode can be: 0:normal, 1:dialog, 2:resizable dialog
void* wsi_new_win ( int x, int y, int w, int h, const char* label, WsWindow* uwindow, int mode );

// deletes the associated window and data
void wsi_del_win ( void* win );

// returns the window label
const char* wsi_win_label ( void* win );

// changes the window name
void wsi_win_label ( void* win, const char* label );

// show the window
void wsi_win_show ( void* win );

// hide the window
void wsi_win_hide ( void* win );

// move the window; if a parameter is <0 its respective dimension is not changed
void wsi_win_move ( void* win, int x, int y, int w, int h );

// returns position of window
void wsi_win_pos ( void* win, int& x, int& y );

// returns size of window
void wsi_win_size ( void* win, int& w, int& h );

// triggers a window redraw
void wsi_win_redraw ( void* win );

// returns if the window is on the screen
bool wsi_win_visible ( void* win );

// returns if the window is minimized
bool wsi_win_minimized ( void* win );

// returns number of existing open windows
int wsi_num_windows ();

// set the window OpenGL context to be active
void wsi_activate_ogl_context ( void* win );

// system-dependent function to load an OpenGL function by name
void* wsi_get_ogl_procedure ( const char *name );

//========== Events ===========

// get window events and send them to the respective windows; returns number of open windows
int wsi_check ();

//========== Sys Info ===========

// returns screen resolution of primary display in pixels 
void wsi_screen_resolution ( int& w, int& h );

// returns the list of arguments passed to the main function
char** wsi_program_argv ();

// returns the number of arguments passed to the main function
int wsi_program_argc ();

//========== File System Browsing ===========

# include <sig/gs_array.h>

// dialog to select files
const char* wsi_open_file_dialog ( const char* msg, const char* file, const char* filter, GsArray<const char*>* multif=0 );

// dialog to select enter file name to save
const char* wsi_save_file_dialog ( const char* msg, const char* file, const char* filter=0 );

// dialog to select folder
const char* wsi_select_folder ( const char* msg, const char* folder=0 );

//================================ End of File =================================================

# endif // WS_OSINTERFACE_H
