/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>
# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

# include <sig/sn_planar_objects.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= SnPlanarObjects ====================================

const char* SnPlanarObjects::class_name = "SnPlanarObjects";
SN_SHAPE_RENDERER_DEFINITIONS(SnPlanarObjects);

SnPlanarObjects::SnPlanarObjects () : SnShape ( class_name )
{
	GS_TRACE1 ( "Constructor" );
	zcoordinate=0;
	_zi=0;
}

SnPlanarObjects::SnPlanarObjects ( const char* classname ) : SnShape ( classname )
{
	GS_TRACE1 ( "Constructor w/ class_name="<<classname );
	zcoordinate=0;
	_zi=0;
}

SnPlanarObjects::~SnPlanarObjects ()
{
	GS_TRACE1 ( "Destructor" );
}

void SnPlanarObjects::init ()
{
	P.size(0);
	C.size(0);
	T.size(0);
	I.size(0);
	G.size(0);
	_zi=0;
	touch();
}

void SnPlanarObjects::compress ()
{
	P.compress();
	C.compress();
	T.compress();
	I.compress();
	G.compress();
}

// Comment: groups are needed to ensure drawing order by calling separate drawing calls;
// this can be optimized to a single draw call by using depth buffer to sort primitives,
// and using white point in shared texture in order to remove the need to change shader 
// uniform mode value between calls. This would however make this class rather specific.

# define GSET(g,t,ti,i) g.type=t; g.texid=ti; g.ind=i

void SnPlanarObjects::start_group ( GroupType t, gsuint tid, bool forcenew )
{
	if ( G.size() && G.top().ind==(gsuint)I.size() ) G.pop(); // previous group is empty

	if ( forcenew ) // force ensures drawing order among subsequent groups of same type
	{ G.push(); GSET(G.top(),t,tid,I.size());
	}
	else if ( t==Colored )
	{	if ( G.size() )
		{	if ( G.top().type!=t ) // add new group only if it is a different one
			{	G.push(); GSET(G.top(),t,tid,I.size());	}
		}
	}
	else if (I.empty()) // starting first group
	{	G.size(1); GSET(G.top(),t,tid,I.size());
	}
	else
	{	if ( G.empty() ) { G.size(1); GSET(G.top(),Colored,0,0); } // add non-declared 1st group
		if ( G.top().type!=t || G.top().texid!=tid ) // add new group only if different
		{	G.push(); GSET(G.top(),t,tid,I.size());
		}
	}
	set_zero_index();
	touch();
	//for(int i=0;i<G.size();i++) gsout<<G[i].type<<":"<<G[i].texid<<","; gsout<<gsnl; // to inspect groups
}

int SnPlanarObjects::push_points ( int n )
{
	int os = P.size();
	int ns = os + n;
	P.size(ns);
	C.size(ns);
	if (G.size()) T.size(ns);
	return os;
}

int SnPlanarObjects::push_points ( int n, GsColor c )
{
	int i0 = push_points(n);
	for ( int i=i0; i<P.size(); i++ ) C[i]=c;
	return i0;
}

int SnPlanarObjects::push_rect ( const GsRect& r, char cs )
{
	int i=push_points ( 4 );
	if ( cs=='s' ) // scene coordinates
		r.get_vertices ( P[i], P[i+1], P[i+3], P[i+2] ); // P will have: bl, br, tr, tl
	else // window coordinates
		r.get_vertices ( P[i+2], P[i+3], P[i+1], P[i] ); // P will have: bl, br, tr, tl
	push_global_indices ( i, i+1, i+2 );
	push_global_indices ( i, i+2, i+3 );
	return i;
}

void SnPlanarObjects::push_rect ( const GsRect& r, GsColor c, char cs )
{
	int i=push_rect ( r, cs );
	C[i]=c; C[i+1]=c; C[i+2]=c; C[i+3]=c;
}

void SnPlanarObjects::push_rect ( const GsRect& r, GsColor c1, GsColor c2, GsColor c3, GsColor c4, char cs )
{
	int i=push_rect ( r, cs );
	C[i]=c1; C[i+1]=c2; C[i+2]=c3; C[i+3]=c4;
}

void SnPlanarObjects::push_quad ( const GsPnt2& p1, const GsPnt2& p2, const GsPnt2& p3, const GsPnt2& p4, GsColor c )
{
   int i = push_points ( 4, c );
   P[i]=p1; P[i+1]=p2; P[i+2]=p3; P[i+3]=p4;
   push_global_indices ( i, i+1, i+2 );
   push_global_indices ( i, i+2, i+3 );
}

void SnPlanarObjects::push_triangle ( const GsPnt2& p1, const GsPnt2& p2, const GsPnt2& p3, GsColor c )
{
	int i = push_points ( 3, c );
	P[i]=p1; P[i+1]=p2; P[i+2]=p3;
	push_global_indices ( i, i+1, i+2 );
}

void SnPlanarObjects::push_line ( const GsPnt2& p1, const GsPnt2& p2, float thickness, GsColor c )
{
	GsVec2 v = (p2-p1).ortho();
	v.len ( thickness/2.0f );
	push_quad ( p1+v, p1-v, p2-v, p2+v, c );
}

void SnPlanarObjects::push_lines ( const GsRect& r, float thickness, GsColor c )
{
	const float half = thickness/2.0f;
	GsRect rs = r;
	rs.grow ( -half, -half );
	GsPnt2 p1, p2, p3, p4;
	rs.get_vertices ( p1, p2, p4, p3 );
	push_line ( p1, p2, thickness, c );
	push_line ( p2, p3, thickness, c );
	push_line ( p3, p4, thickness, c );
	push_line ( p4, p1, thickness, c );
	// This can be improved to avoid corner overlap and to share endpoints: use future polyline method
}

void SnPlanarObjects::get_bounding_box ( GsBox& b ) const
{ 
	int i;
	b.set_empty ();
	for ( i=0; i<P.size(); i++ ) b.extend ( GsPnt(P[i].x,P[i].y,zcoordinate) );
}

//================================ EOF =================================================
