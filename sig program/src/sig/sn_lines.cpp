/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_quat.h>
# include <sig/sn_lines.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Methods
# include <sig/gs_trace.h>

//======================================= SnLines ====================================

const char* SnLines::class_name = "SnLines";
SN_SHAPE_RENDERER_DEFINITIONS(SnLines);

SnLines::SnLines () : SnShape ( class_name )
{
	GS_TRACE1 ( "SnLines Constructor" );
	patfactor = 0;
	_colorspervertex = 0;
	_polyline = 0;
	_lwidth = 1.0;
}

SnLines::~SnLines ()
{
	GS_TRACE1 ( "SnLines Destructor" );
}

void SnLines::init ()
{
	GS_TRACE2 ( "SnLines Init" );
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

void SnLines::compress ()
{
	P.compress();
	Pc.compress();
	V.compress();
	Vc.compress();
	I.compress();
	Is.compress();
}

void SnLines::begin_polyline ()
{ 
	I.push() = V.size();
	_polyline = 1;
}

void SnLines::end_polyline ()
{
	int size = V.size()-I.top();
	if ( size<=1 ) { I.pop(); if(size==1) V.pop(); return; }
	Is.push() = size;
	_polyline = 0;
}

void SnLines::push ( float x, float y, float z )
{
	touch();
	if ( _polyline ) // polyline being defined
	{	V.push().set(x,y,z);
		if ( _colorspervertex ) Vc.push()=_material.diffuse;
	}
	else // segment being defined
	{	P.push().set(x,y,z);
		if ( _colorspervertex ) Pc.push()=_material.diffuse;
	}
}

void SnLines::push ( const GsColor &c )
{
	if ( !_colorspervertex )
	{	while ( Vc.size()<V.size() ) Vc.push()=_material.diffuse;
		while ( Pc.size()<P.size() ) Pc.push()=_material.diffuse;
		_colorspervertex = 1;
	}
	_material.diffuse = c;
}

void SnLines::push_cross ( GsPnt2 c, float r, float z )
{
	GsVec2 p(r,r);
	push ( c-p, c+p, z ); p.x*=-1;
	push ( c-p, c+p, z );
}

void SnLines::push_cross ( GsPnt c, float r )
{
	const float z = 0;
	GsVec p(r,z,z);
	push ( c-p, c+p ); p.set(z,r,z);
	push ( c-p, c+p ); p.set(z,z,r);
	push ( c-p, c+p );
}

static float kl ( float k, int i )
{
	return i%10? k : kl(k*1.6f,i/10);
}

static void drawrule ( SnLines* l, char c, int i, float k, float z )
{
	if ( c=='X' )
		l->push ( (float)i, z, z, (float)i, kl(k,i), z );
	else if ( c=='Y' )
		l->push ( z, (float)i, z, -kl(k,i), (float)i, z );
	else
		l->push ( z, z, (float)i, z, kl(k,i), (float)i );
}

static void drawrule ( SnLines* l, char c, float mr, float r, float k, float z )
{
	int i, inc=1;
	int mini=GS_CEIL(mr); int maxi=GS_FLOOR(r);

	bool cut=false;
	float f = (mr-r)/100000.0f;
	f = GS_ABS(f);
	if ( f>1.0f ) { inc += 10*int(f); cut=true; }

	l->push ( c=='X'? GsColor::red : c=='Y'? GsColor::green : GsColor::blue );

	if ( cut )
	{	if ( c=='X' )
		{	l->push ( GsColor::magenta ); }
		else if ( c=='Z' )
		{	l->push ( GsColor::cyan ); }
	}

	for ( i=inc;  i<maxi; i+=inc ) drawrule(l,c,i,k,z);
	for ( i=-inc; i>mini; i-=inc ) drawrule(l,c,i,k,z);
}

void SnLines::push_axis ( const GsPnt& orig, float len, int dim, const char* let,
						  bool rule, GsBox* box )
{
	float r, mr;
	float a, b, c, k;
	const float z = 0.0f;

	r = box? box->maxsize()/2.0f : len;
	mr = -r;
	a=r/25.0f;  b=a/2.0f; c=a*3.0f; k=a/8.0f;

	bool letx=false, lety=false, letz=false;
	int pi = P.size(); // P array contain segment points

	if ( let )
	{	while ( *let )
		{	switch ( GS_UPPER(*let) )
			{	case 'X': letx=true; break;
				case 'Y': lety=true; break;
				case 'Z': letz=true; break;
			}
			let++;
		}
	}

	if ( dim>=1 )
	{	if ( box ) { mr=box->a.x; r=box->b.x; }
		push ( GsColor::red );
		push ( mr,  z,  z, r, z, z ); // X axis
		if ( letx && r>0 )
		{	push (   r, -a, z, r-a, -c, z ); // Letter X
			push ( r-a, -a, z, r,   -c, z );
		}
		if ( rule && r>1.0 ) drawrule(this,'X',mr,r,k,z);
	}

	if ( dim>=2 )
	{	if ( box ) { mr=box->a.y; r=box->b.y; }
		push ( GsColor::green );
		push (  z, mr,  z, z, r, z ); // Y axis
		if ( lety && r>0 )
		{	push (   a,  r, z, a+b, r-a,   z ); // Letter Y
			push ( a+a,  r, z, a,   r-a-a, z );
		}
		if ( rule && r>1.0 ) drawrule(this,'Y',mr,r,k,z);
	}

	if ( dim>=3 ) 
	{	if ( box ) { mr=box->a.z; r=box->b.z; }
		push ( GsColor::blue );
		push (  z,  z, mr, z, z, r ); // Z axis
		if ( letz && r>0 )
		{	begin_polyline ();
			push ( z, -a, r-a ); // Letter Z
			push ( z, -a,   r ); 
			push ( z, -c, r-a ); 
			push ( z, -c,   r );
			end_polyline ();
		}
		if ( rule && r>1.0 ) drawrule(this,'Z',mr,r,k,z);
	}

	if ( orig!=GsPnt::null )
	{	for ( int i=pi; i<P.size(); i++ ) P[i]+=orig;
	}
}

void SnLines::push_box ( const GsBox& box, const GsColor* cx, const GsColor* cy, const GsColor* cz )
{
	const GsPnt& a = box.a;
	const GsPnt& b = box.b;

	if ( cx ) push ( *cx );
	push ( a.x, a.y, a.z, b.x, a.y, a.z );
	push ( a.x, a.y, b.z, b.x, a.y, b.z );
	push ( a.x, b.y, a.z, b.x, b.y, a.z );
	push ( a.x, b.y, b.z, b.x, b.y, b.z );

	if ( cy ) push ( *cy );
	push ( a.x, a.y, a.z, a.x, b.y, a.z );
	push ( a.x, a.y, b.z, a.x, b.y, b.z );
	push ( b.x, a.y, b.z, b.x, b.y, b.z );
	push ( b.x, a.y, a.z, b.x, b.y, a.z );

	if ( cy ) push ( *cz );
	push ( a.x, a.y, a.z, a.x, a.y, b.z );
	push ( a.x, b.y, a.z, a.x, b.y, b.z );
	push ( b.x, b.y, a.z, b.x, b.y, b.z );
	push ( b.x, a.y, a.z, b.x, a.y, b.z );
}

void SnLines::push_polyline ( const GsArray<GsVec2>& a, float z )
{
	if ( a.size()<2 ) return;
	begin_polyline();
	for ( int i=0; i<a.size(); i++ ) push ( a[i].x, a[i].y, z );
	end_polyline();
}

void SnLines::push_polygon ( const GsArray<GsVec2>& a, float z )
{
	if ( a.size()<2 ) return;
	begin_polyline();
	for ( int i=0; i<a.size(); i++ ) push ( a[i].x, a[i].y, z );
	push ( a[0].x, a[0].y, z );
	end_polyline();
}

void SnLines::push_lines ( const float* pt, int n, float z )
{
	for ( int i=0; i<n; i++ )
	{	push ( pt[0], pt[1], z );
		push ( pt[2], pt[3], z );
		pt+=4;
	}
}

void SnLines::push_polygon ( const GsPolygon& p, float z )
{ 
	if ( p.open() )
		push_polyline ( (const GsArray<GsVec2>&)p, z );
	else
		push_polygon ( (const GsArray<GsVec2>&)p, z );
}

void SnLines::push_polygon ( const float* pt, int nv, bool open, float z )
{
	if ( nv<2 ) return;
	const float* origpt = pt;
	begin_polyline();
	for ( ; nv>0; nv--, pt+=2 ) push ( pt[0], pt[1], z );
	if ( !open ) push ( origpt[0], origpt[1], z );
	end_polyline();
}

void SnLines::push_circle_approximation ( const GsPnt& center, const GsVec& radius,
										  const GsVec& normal, int nvertices, int iniv, int endv )
{
	GsVec x = radius; // rotating vec to draw the circle
	GsVec x1st = x;

	float dr = gs2pi / (float)nvertices;
	GsQuat dq ( normal, dr );

	begin_polyline ();
	if ( iniv==0 && endv==0 )
	{	while ( nvertices-->0 )
		{	push ( center + x );
			x = dq.apply(x);
		}
		push ( center+x1st ); // to make it close exactly
	}
	else // draw an arc
	{	if ( endv>=nvertices ) endv=nvertices-1;
		for ( int i=0; i<=endv; i++ )
		{	if ( i>=iniv ) push ( center + x );
			x = dq.apply(x);
		}
	}
	end_polyline ();
}

void SnLines::get_bounding_box ( GsBox& b ) const
{
	int i;
	b.set_empty ();
	for ( i=0; i<P.size(); i++ ) b.extend ( P[i] );
	for ( i=0; i<V.size(); i++ ) b.extend ( V[i] );
}

//================================ EOF =================================================
