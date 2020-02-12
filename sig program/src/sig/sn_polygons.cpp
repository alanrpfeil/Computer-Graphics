/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_polygons.h>
# include <sig/sn_lines2.h>
# include <sig/sn_points.h>
# include <sig/sn_planar_objects.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Build
# include <sig/gs_trace.h>

//======================================= SnPolygons ====================================

const char* SnPolygons::class_name = "SnPolygons";

SnPolygons::SnPolygons ( GsPolygons* p ) : SnGroup ( class_name )
{
	GS_TRACE1 ( "Constructor" );
	_pols = p? p : new GsPolygons;
	_pols->ref();
	_solid = 1;
	_vertices = 0;
	add ( new SnPlanarObjects );
	add ( new SnLines2 );
	add ( new SnPoints );
	color ( GsColor::blue );
}

SnPolygons::~SnPolygons ()
{
	GS_TRACE1 ( "Destructor" );
	_pols->unref();
}

void SnPolygons::init ()
{
	_pols->init();
	touch();
}

void SnPolygons::touch ()
{ 
	_nodeuptodate = 0;
	if ( _pols->empty() )
	{	_colors.sizecap(0,0);
		triangles()->init();
		lines()->init();
		points()->init();
	}
}

void SnPolygons::draw_mode ( int solid, int vertices )
{
	if (solid!=-1) _solid=solid;
	if (vertices!=-1) _vertices=vertices;
	touch();
}

void SnPolygons::zcoordinate ( float z )
{
	triangles()->zcoordinate=z;
	lines()->zcoordinate=z;
	touch();
}

void SnPolygons::color ( GsColor ci  )
{
	triangles()->color(ci); 
	touch();
}

void SnPolygons::color ( GsColor ci, GsColor ce, GsColor cp  )
{
	triangles()->color(ci); 
	lines()->color(ce);
	points()->color(cp);
	touch();
}

GsColor SnPolygons::color ( int e ) const
{ 
	return	e==0? triangles()->color() :
			e==1? lines()->color() :
				  points()->color();
}

void SnPolygons::resolution ( float lw, float ps )
{ 
	lines()->line_width(lw);
	points()->point_size(ps);
}

void SnPolygons::polygons ( GsPolygons* p )
{
	_pols->unref();
	_pols = p? p : new GsPolygons;
	_pols->ref();
	touch();
}

void SnPolygons::get_bounding_box ( GsBox& b ) const
{ 
	_pols->get_bounding_box(b);
}

static void (*ExternalTriangulator) ( const GsPolygon& pol, GsArray<GsPnt2>& V, GsArray<int>& T )=0;

void SnPolygons::use_external_triangulator ( void (*f) (const GsPolygon& p,GsArray<GsPnt2>& V,GsArray<int>& T) )
{
	ExternalTriangulator = f;
}

static void draw_polygon (	const GsPolygon& pol, GsColor color,
							SnPlanarObjects& o, SnLines2& l, SnPoints& p, GsArray<GsPnt2>& P, GsArray<int>& T,
							gscenum solid, gscbool vertices, gscbool multicolor )
{
	GS_TRACE2 ( "Rebuilding poly size: "<<pol.size() );

	int i;
	if ( pol.size()<=1 )
	{	if ( pol.empty() ) return;
		if ( !vertices ) return;
	}

	if ( pol.open() || pol.size()<=2 )
	{	if ( multicolor ) l.push ( color );
		l.push_polyline(pol);
	}
	else
	{	const GsArray<GsPnt2>* V;
		T.size(0);
		if ( solid && pol.size()<1000 )
		{	if ( pol.size()==3 ) // no need to triangulate
			{	V = &pol;
				T.size(3);
				T[0]=0; T[1]=1; T[2]=2;
			}
			else if ( ExternalTriangulator )
			{	ExternalTriangulator ( pol, P, T );
				V = &P;
			}
			else
			{	pol.ear_triangulation ( T );
				V = &pol;
			}
			o.set_zero_index();
			o.push_points ( V->size(), color );
			for ( i=0; i<V->size(); i++ ) o.set ( i, V->cget(i) );
			for ( i=0; i<T.size(); i+=3 ) o.push_indices ( T[i], T[i+1], T[i+2] );
		}
		if ( solid==2 || T.empty() )
		{	if ( multicolor ) l.push ( color );
			l.push_polygon(pol);
		}
	}

	if ( vertices )
	{	float z = o.zcoordinate;
		for ( int i=0; i<pol.size(); i++ ) p.push(pol[i].x,pol[i].y,z);
	}
}

void SnPolygons::update_node ()
{
	if ( _nodeuptodate ) return;
	_nodeuptodate = 1;
	rebuild ();
}

void SnPolygons::rebuild ()
{ 
	GS_TRACE2 ( "Rebuilding "<<instance_name() );

	int size = _pols->size();
	if ( !size ) return;

	SnPlanarObjects* t = triangles ();
	SnLines2* l = lines ();
	SnPoints* p = points ();

	t->init();
	l->init();
	p->init();

	if ( _vertices )
	{	p->visible ( true );
	}
	else
	{	p->visible ( false );
	}

	GsArray<GsPnt2> P;
	GsArray<int> T;
	if ( _colors.size()==size )
	{	for ( int i=0; i<size; i++ )
			::draw_polygon ( _pols->get(i), _colors[i], *t, *l, *p, P, T, _solid, _vertices, 1 );
	}
	else
	{	for ( int i=0; i<size; i++ )
			::draw_polygon ( _pols->get(i), t->color(), *t, *l, *p, P, T, _solid, _vertices, 0 );
	}
}

//================================ EOF =================================================
