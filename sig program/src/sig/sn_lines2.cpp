/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>

# include <sig/sn_lines2.h>
# include <sig/gs_polygon.h>
# include <sig/gs_font.h>
# include <sig/gs_polygon.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= SnLines2 ====================================

const char* SnLines2::class_name = "SnLines2"; // static
SN_SHAPE_RENDERER_DEFINITIONS(SnLines2);

SnLines2::SnLines2 () : SnShape ( class_name )
{
	GS_TRACE1 ( "Constructor" );
	patfactor = 0; // SgDev: line pattern in SnLines2 and SnLines not yet implemented
	_colorspervertex = 0;
	_polyline = 0;
	_lwidth = 1.0;
	zcoordinate = 0;
}

SnLines2::~SnLines2 ()
{
	GS_TRACE1 ( "Destructor" );
}

void SnLines2::init ()
{
	P.size(0);
	Pc.size(0);
	V.size(0);
	Vc.size(0);
	I.size(0);
	Is.size(0);
	_colorspervertex = 0;
	_polyline = 0;
	touch();
}

void SnLines2::compress ()
{
	P.compress();
	Pc.compress();
	V.compress();
	Vc.compress();
	I.compress();
	Is.compress();
}

void SnLines2::begin_polyline ()
{
	I.push() = V.size();
	_polyline = 1;
}

void SnLines2::end_polyline ()
{
	int size = V.size()-I.top();
	if ( size<=1 ) { I.pop(); if(size==1) V.pop(); return; }
	Is.push() = size;
	_polyline = 0;
}

void SnLines2::append_to_last_polyline ( GsVec2 p )
{ 
	if ( _polyline ) return;
	if ( I.empty() ) return;
	_polyline=1;
	push(p);
	_polyline=0;
	Is.top()++;
}

void SnLines2::push ( float x, float y )
{
	touch();
	if ( _polyline ) // polyline being defined
	{	V.push().set(x,y);
		if ( _colorspervertex ) Vc.push()=_material.diffuse;
	}
	else // segment being defined
	{	P.push().set(x,y);
		if ( _colorspervertex ) Pc.push()=_material.diffuse;
	}
}

void SnLines2::push ( GsColor c )
{
	if ( !_colorspervertex )
	{	while ( Vc.size()<V.size() ) Vc.push()=_material.diffuse;
		while ( Pc.size()<P.size() ) Pc.push()=_material.diffuse;
		_colorspervertex = 1;
	}
	_material.diffuse = c;
}

void SnLines2::push_arrow ( GsPnt2 p1, GsPnt2 p2, float l, float w )
{
	GsVec2 v=p1-p2;	v.len(l);
	GsVec2 o=v.ortho(); o.len(w);
	push ( p1, p2 ); v+=p2;
	push ( p2, v+o );
	push ( p2, v-o );
}

void SnLines2::push_cross ( GsPnt2 c, float r )
{
	GsVec2 p(r,r);
	push ( c-p, c+p ); p.x=-p.x;
	push ( c-p, c+p );
}

void SnLines2::push_rect ( const GsRect& r )
{
	float ryh = float(r.y+r.h);
	float rxw = float(r.x+r.w);
	begin_polyline();
	push ( r.x, r.y );
	push ( r.x, ryh );
	push ( rxw, ryh );
	push ( rxw, r.y );
	push ( r.x, r.y );
	end_polyline();
}

void SnLines2::push_polygon ( const float* pt, int nv, bool open )
{
	if ( nv<2 ) return;
	const float* origpt = pt;
	begin_polyline();
	for ( ; nv>0; nv--, pt+=2 ) push ( pt[0], pt[1] );
	if ( !open ) push ( origpt[0], origpt[1] );
	end_polyline();
}

void SnLines2::push_polygon ( const GsArray<GsVec2>& a, bool open )
{
	if ( a.size()<2 ) return;
	push_polygon ( a[0].e, a.size(), open );
}

void SnLines2::push_polygon ( const GsPolygon& p )
{
	push_polygon ( p.fpt(), p.size(), p.open() );
}

void SnLines2::push_lines ( const float* pt, int nl )
{
	if ( nl<4 ) return;
	for ( ; nl>0; nl--, pt+=4 ) push ( pt[0], pt[1], pt[2], pt[3] );
}

void SnLines2::push_circle_approximation ( GsPnt2 center, float radius, int nvertices )
{
	GsVec2 p;
	int i0 = V.size();
	float ang=0, incang = ((float)GS_2PI) / nvertices;
	begin_polyline ();
	while ( nvertices>0 )
	{	p.set ( radius*sinf(ang), radius*cosf(ang) );
		p += center;
		push ( p );
		ang += incang;
		nvertices--;
	}
	push ( V[i0] ); // to make it close exactly
	end_polyline ();
}

void SnLines2::get_bounding_box ( GsBox& b ) const
{
	int i;
	b.set_empty ();
	for ( i=0; i<P.size(); i++ ) b.extend ( GsPnt(P[i].x,P[i].y,zcoordinate) );
	for ( i=0; i<V.size(); i++ ) b.extend ( GsPnt(V[i].x,V[i].y,zcoordinate) );
}

//================================ EOF =================================================
