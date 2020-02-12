/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file ws_run.h
 * system-level application control functions
 */

# ifndef WS_RUN_H
# define WS_RUN_H

class WsWindow;

/*! Register the given cb function to be called at regular intervals given in seconds. */
void ws_add_timer ( double interval, void(*cb)(void*), void* udata );

/*! Register a timer as an event sent to the given window. */
void ws_add_timer ( double interval, WsWindow* win, int ev );

/*! Removes the first occurance of the given callback pointer from the list of active timers. */
void ws_remove_timer ( void(*cb)(void*) );

/*! Removes a timer registered for a window event. */
void ws_remove_timer ( WsWindow* win, int ev );

/*! Removes all timers registered for the given window. */
void ws_remove_timers ( WsWindow* win );

/*! Checks if a timer is to be executed. Can be called for controlling timers
	outside of ws_run() or ws_check() */
void ws_check_timers ();

/*! Process events until there are open windows. If sleepms>0 a sleep of time 
	sleepms miliseconds is called between every event processed.
	If sleepms<0, a sleep of 1ms is performed every |sleepms| iterations.
	No sleep is performed if sleepms is 0. */  
void ws_run ( int sleepms=-100 );

/*! Check function designed for local event processing such as for controlling an
	animation loop or waiting for a dialog box to be processed.
	It will call gs_sleep() according to parameter sleepms, as described in ws_run(),
	will check registered timers, and will automatically call exit(0) if all windows close. */
void ws_check ( int sleepms=-20 );

/*! Process system events and registered timers without a sleep and without checking
	for closed windows. Returns the number of active windows. */
int ws_fast_check ();

/*! Calls standard exit() with the given code */
void ws_exit ( int c=0 );

/*! Returns screen resolution of primary display in pixels. */
void ws_screen_resolution ( int& w, int& h );

/*! Returns the current number of open windows */
inline int ws_num_windows () { extern int wsi_num_windows(); return wsi_num_windows(); }

// returns the list of arguments passed to the main function
inline char** ws_program_argv () { extern char** wsi_program_argv(); return wsi_program_argv(); }

// returns the number of arguments passed to the main function
inline int ws_program_argc () { extern int wsi_program_argc(); return wsi_program_argc(); }

//================================ End of File =================================================

# endif // WS_RUN_H
