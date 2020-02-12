/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>

# include <sig/gs_trackball.h>
# include <sig/gs_mat.h>

//# define GS_USE_TRACE1  
# include <sig/gs_trace.h>

//=================================== GsTrackball ===================================

GsTrackball::GsTrackball ()
{
	init ();
}

GsTrackball::GsTrackball ( const GsTrackball& t )
			:rotation(t.rotation), lastspin(t.lastspin)
{
}

void GsTrackball::init () 
{ 
	rotation = GsQuat::null;
}

// Project x,y onto a sphere or hyperbolic surface if far from the sphere
static float tb_project_to_sphere ( float r, float x, float y )
{
	float d, t, z;
	d = sqrtf(x*x + y*y);
	if ( d < r*gsqrt2/2.0f )
	{	z=sqrtf(r*r-d*d); } // Inside sphere 
	else
	{	t=r/gsqrt2; z=t*t/d; } // On hyperbola
	return z;
}

void GsTrackball::get_spin_from_mouse_motion ( float p1x, float p1y, float p2x, float p2y, GsQuat& spin )
{
	GsVec p1, p2, d, a; // a==Axes of rotation
	const float size=0.8f;
	float t;
	if ( p1x==p2x && p1y==p2y ) { spin=GsQuat::null; return; }
	// figure out z-coordinates for projection of P1 and P2 to deformed sphere

	p1.set ( p1x, p1y, tb_project_to_sphere(size,p1x,p1y) );
	p2.set ( p2x, p2y, tb_project_to_sphere(size,p2x,p2y) );
	a = cross ( p1, p2 ); // axis

	d = p1-p2; // how much to rotate around axis
	t = d.norm() / (2.0f*size);
	t = GS_BOUND ( t, -1.0f, 1.0f ); // avoid problems with out-of-control values
	float ang = 2.0f * asinf(t);

	spin.set ( a, ang ); // a is normalized inside rot()
}

void GsTrackball::increment_from_mouse_motion ( float lwinx, float lwiny, float winx, float winy )
{
	get_spin_from_mouse_motion ( lwinx, lwiny, winx, winy, lastspin );
	rotation = lastspin * rotation;
}

void GsTrackball::increment_rotation ( const GsQuat& spin )
{
	lastspin = spin;
	rotation = lastspin * rotation;
}

//=============================== friends ==========================================

GsOutput& operator<< ( GsOutput& out, const GsTrackball& tb )
{
	out << "rotat: " << tb.rotation << gsnl <<
		   "spin: " << tb.lastspin << gsnl;

	return out;
}

//================================ End of File =========================================
