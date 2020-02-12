/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_box.h>
# include <sig/gs_polygons.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
# include <sig/gs_trace.h>

//============================= GsPolygons ==================================

GsPolygons::GsPolygons ()
{
	GS_TRACE1 ( "Default Constructor" );
}

GsPolygons::GsPolygons ( const GsPolygons& polys ): GsShareable()
{
	GS_TRACE1 ( "Copy Constructor" );
	_data.size ( polys.size() );
	for ( int i=_data.size()-1; i>=0; i-- )
	{	_data[i] = new GsPolygon ( polys.cget(i) );
	}
}

GsPolygons::~GsPolygons ()
{
	GS_TRACE1 ( "Destructor" );
	size (0);
}

void GsPolygons::size ( int ns )
{
	int i, s=_data.size();
	if ( ns>s )
	{	_data.size(ns);
		for ( i=s; i<ns; i++ ) _data[i] = new GsPolygon;
	}
	else if ( ns<s )
	{	for ( i=ns; i<s; i++ ) delete _data[i];
		_data.size(ns);
	}
}

void GsPolygons::capacity ( int nc )
{
	int i, s=_data.size();
	if ( nc<0 ) nc=0;
	if ( nc<s )
	{	for ( i=nc; i<s; i++ ) delete _data[i];
	}
	_data.capacity(nc);
}

void GsPolygons::compress_all ()
{
	compress ();
	for ( int i=_data.size()-1; i>=0; i-- )
	{	_data[i]->compress();
	}
}

void GsPolygons::swap ( int i, int j )
{
	GsPolygon* tmp;
	GS_SWAP ( _data[i], _data[j] );
}

void GsPolygons::insert ( int i, const GsPolygon& x )
{
	_data.insert ( i );
	_data[i] = new GsPolygon(x);
}

void GsPolygons::insert ( int i, int n )
{
	_data.insert ( i, n );
	for ( int j=0; j<n; j++ )
		_data[i+j] = new GsPolygon;
}

void GsPolygons::remove ( int i, int n )
{
	for ( int j=0; j<n; j++ ) delete _data[i+j];
	_data.remove ( i, n );
}

GsPolygon* GsPolygons::extract ( int i )
{
	GsPolygon* p = _data[i];
	_data.remove ( i );
	return p;
}

bool GsPolygons::pick_vertex ( const GsVec2& p, float epsilon, int& pid, int& vid ) const
{ 
	int i, j, s;
	float distmin, dist;

	int ps = size();
	if ( ps==0 ) return false;

	pid = vid = -1;
	distmin = 0;

	for ( i=0; i<ps; i++ )
	{	s = cget(i).size();
		for ( j=0; j<s; j++ )
		{	dist = dist2 ( cget(i,j), p );
			if ( dist<distmin || pid<0 ) { distmin=dist; pid=i; vid=j; }
		}
	}

	if ( pid>=0 && distmin<=epsilon*epsilon )
		return true;
	else
		return false;
}

int GsPolygons::pick_polygon ( const GsVec2& p ) const
{
	for ( int i=0; i<size(); i++ )
		if ( cget(i).contains(p) ) return i;
	return -1;
}

bool GsPolygons::pick_edge ( const GsVec2& p, float epsilon, int& pid, int& vid ) const
{
	float dist2, mindist2=-1;
	int i, id;

	pid = vid = -1; 

	for ( i=0; i<size(); i++ )
	{	id = cget(i).pick_edge ( p, epsilon, dist2 );
		if ( id>=0 )
		{	if ( vid<0 || dist2<mindist2 )
			{	vid = id;
				pid = i;
				mindist2 = dist2;
			}
		}
	}

	return vid>=0;
}

void GsPolygons::get_bounding_box ( GsBox& b ) const
{
	int i, j, s;
	GsVec p;
	b.set_empty ();
	for ( i=0; i<size(); i++ )
	{	s = cget(i).size();
		for ( j=0; j<s; j++ )
		{	const GsVec2& p2 = cget(i,j);
			p.set ( p2.x, p2.y, 0.0f );
			b.extend ( p );
		}
	}
}

void GsPolygons::translate ( const GsVec2& dv )
{
	for ( int i=size()-1; i>=0; i-- ) get(i).translate(dv);
}

void GsPolygons::scale ( float s )
{
	for ( int i=size()-1; i>=0; i-- ) get(i).scale(s);
}

void GsPolygons::operator = ( const GsPolygons& p )
{
	int s=p.size();
	size ( s );
	for ( int i=0; i<s; i++ ) get(i)=p.cget(i);
}

GsOutput& operator<< ( GsOutput& o, const GsPolygons& p )
{
	int i, m;
	m = p.size()-1;
	o << '[';
	for ( i=0; i<=m; i++ )
	{	o << p.cget(i);
		if ( i<m ) o<<"\n ";
	}
	return o << "]\n";
}

GsInput& operator>> ( GsInput& in, GsPolygons& p )
{
	p.init();
	in.get(); // get '['
	while (true)
	{	if ( in.check()==GsInput::Delimiter ) // found ']'
		{	in.get(); break; }
		p.push();
		in >> p.top();
	}
	return in;
}

//================================ EOF =================================================
