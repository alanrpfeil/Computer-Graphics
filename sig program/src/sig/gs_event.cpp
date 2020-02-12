/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
 
# include <sig/gs_event.h>

//# define GS_USE_TRACE1  
# include <sig/gs_trace.h>

//===================================== GsEvent =================================

GsEvent::GsEvent ()
{
	width = height = 0;
	pixelsize = 1.0f;
	mousex = mousey = 0;
	init ();
}

void GsEvent::init ()
{
	type = None;
	key = 0;
	button = 0;
	button1 = button2 = button3 = 0;
	ctrl = shift = alt = key = 0;
	wheelclicks = 0;
	lmousex = mousex;
	lmousey = mousey;
	mousex = mousey = 0;
}

const char *GsEvent::type_name () const
{
	static const char *types[] = { "None", "Push", "Release", "Drag", "Move", "Wheel", "KeyPress", "KeyRelease" };
	return types [ int(type) ];
}

void GsEvent::normalize_coordinates ( int w, int h ) const
{
	mouse.set ( (mousex*2.0f/float(w))-1.0f, -((mousey*2.0f/float(h))-1.0f) );
}

GsOutput& operator<< ( GsOutput& out, const GsEvent& e )
{
	out << e.type_name();

	if ( e.type==GsEvent::Keyboard || e.type==GsEvent::KeyRelease ) 
	{	if ( e.key>=1 && e.key<=12 ) out << " [F" << (int)e.key << ']';
		else if ( e.key>12 && e.key<255 ) out << " [" << char(e.key) << ':' << (int)e.key << ']';
		else out << " [" << (int)e.key << ']';
	}
	else if ( e.type==GsEvent::Wheel ) 
	{	out << " w:" << e.wheelclicks; }
	else
	{	out << " m:" << e.mousex << gspc << e.mousey; }

	out << " b:" << (int)e.button1<<(int)e.button2<<(int)e.button3<<
		   " acs:" << (int)e.alt<<(int)e.ctrl<<(int)e.shift;

	return out;
}

