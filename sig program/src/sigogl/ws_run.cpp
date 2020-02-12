/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>

# include <sig/gs_string.h>

# include <sigogl/ws_run.h>
# include <sigogl/ws_window.h>
# include <sigogl/ws_osinterface.h>

//===================================== xx  =================================================

struct SwTimerData
{	double interval;
	double lasttime;
	WsWindow* window;
	void (*callback) (void*);
	union { void* udata; int evid; };
};

static GsArray<SwTimerData> Timers;
static int NumTimers=0;
static double Time0=0;
static double TimeCur=0;

void ws_check_timers ()
{
	TimeCur = gs_time()-Time0;
	for ( int i=0; i<NumTimers; i++ )
	{	if ( TimeCur-Timers[i].lasttime > Timers[i].interval )
		{	if ( Timers[i].callback )
			{	Timers[i].callback ( Timers[i].udata ); }
			else
			{	WsWindow* win = Timers[i].window;
				if ( !win->minimized() )
					win->timer ( Timers[i].evid );
			}
			Timers[i].lasttime = TimeCur;
		}
	}
}

static void add_timer ( double interval, WsWindow* win, void(*cb)(void*) )
{
	if ( Time0==0 ) Time0 = gs_time();
	SwTimerData& t = Timers.push();
	t.interval = interval;
	t.lasttime = TimeCur;
	t.window = win;
	t.callback = cb;
	NumTimers++;
}

void ws_add_timer ( double interval, void(*cb)(void*), void* udata )
{
	add_timer ( interval, 0, cb );
	Timers.top().udata = udata;
}

void ws_add_timer ( double interval, WsWindow* win, int ev )
{
	add_timer ( interval, win, 0 );
	Timers.top().evid = ev;
}

void ws_remove_timer ( void(*cb)(void*) )
{
	for ( int i=0; i<Timers.size(); i++ )
	{	if ( Timers[i].callback == cb )
		{	Timers[i] = Timers.pop();
			NumTimers--;
			return;
		}
	}
}

void ws_remove_timer ( WsWindow* win, int ev )
{
	for ( int i=0; i<Timers.size(); i++ )
	{	if ( Timers[i].window==win && Timers[i].evid==ev )
		{	Timers[i] = Timers.pop();
			NumTimers--;
			return;
		}
	}
}

void ws_remove_timers ( WsWindow* win )
{
	for ( int i=0; i<Timers.size(); i++ )
	{	if ( Timers[i].window==win )
		{	Timers[i] = Timers.pop();
			NumTimers--;
			i--;
		}
	}
}

void ws_run ( int sleepms )
{
	if ( sleepms>0 )
	{	while ( wsi_check() )
		{	gs_sleep ( sleepms );
			if ( NumTimers ) ws_check_timers ();
		}
	}
	else if ( sleepms<0 )
	{	int n=sleepms;
		while ( wsi_check() )
		{	if ( NumTimers ) ws_check_timers ();
			if ( ++n==0 ) { gs_sleep(1); n=sleepms; }
		}
	}
	else
	{	while ( wsi_check() )
		{	if ( NumTimers ) ws_check_timers ();
		}
	}
}

void ws_check ( int sleepms )
{
	static int counter=0;
	if ( sleepms>0 ) gs_sleep ( sleepms );
	else if ( sleepms<0 && ++counter%-sleepms==0 ) gs_sleep(1);
   
	if ( NumTimers ) ws_check_timers ();
	if ( wsi_check()==0 ) exit(0);
}

int ws_fast_check ()
{
	if ( NumTimers ) ws_check_timers ();
	return wsi_check ();
}

void ws_exit ( int c )
{
	exit ( c );
}

void ws_screen_resolution ( int& w, int& h )
{
	wsi_screen_resolution ( w, h );
}

//================================ End of File =================================================
