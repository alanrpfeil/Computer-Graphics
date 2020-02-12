/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_box.h>
# include <sig/gs_mat.h>
# include <sig/gs_quat.h>

//======================================== GsBox =======================================

GsBox::GsBox ( const GsBox& x, const GsBox& y )
	  : a ( GS_MIN(x.a.x,y.a.x), GS_MIN(x.a.y,y.a.y), GS_MIN(x.a.z,y.a.z) ),
		b ( GS_MAX(x.b.x,y.b.x), GS_MAX(x.b.y,y.b.y), GS_MAX(x.b.z,y.b.z) ) 
{
}

bool GsBox::empty () const
{
	return a.x>b.x || a.y>b.y || a.z>b.z;
}

float GsBox::volume () const
{
	return (b.x-a.x) * (b.y-a.y) * (b.z-a.z);
}

GsVec GsBox::center () const 
{
	return (a+b)/2.0f; // == a + (b-a)/2 == a + b/2 - a/2
}

void GsBox::center ( const GsPnt& p )
{
	(*this) += p-center();
}

void GsBox::size ( const GsVec& v )
{
	b = a+v;
}

GsVec GsBox::size () const 
{
	return b-a; 
}

float GsBox::maxsize () const
{
	GsVec s = b-a;
	return GS_MAX3(s.x,s.y,s.z);
}

float GsBox::minsize () const
{
	GsVec s = b-a;
	return GS_MIN3(s.x,s.y,s.z);
}

void GsBox::extend ( const GsPnt &p )
{
	if ( empty() ) { a=p; b=p; }
	GS_UPDMIN ( a.x, p.x ); GS_UPDMAX ( b.x, p.x );
	GS_UPDMIN ( a.y, p.y ); GS_UPDMAX ( b.y, p.y );
	GS_UPDMIN ( a.z, p.z ); GS_UPDMAX ( b.z, p.z );
}

void GsBox::extend ( const GsBox &box )
{
	if ( empty() ) *this=box;
	if ( box.empty() ) return;
	GS_UPDMIN ( a.x, box.a.x ); GS_UPDMAX ( b.x, box.b.x );
	GS_UPDMIN ( a.y, box.a.y ); GS_UPDMAX ( b.y, box.b.y );
	GS_UPDMIN ( a.z, box.a.z ); GS_UPDMAX ( b.z, box.b.z );
}

void GsBox::grow ( float dx, float dy, float dz )
{
	a.x-=dx; a.y-=dy; a.z-=dz;
	b.x+=dx; b.y+=dy; b.z+=dz;
}

void GsBox::centralize ( const GsBox& b, GsVec& translation, float& scale ) const
{
	translation = center()-b.center();
	float facx = dx()/b.dx();
	float facy = dy()/b.dy();
	if ( b.dz()==0 )
	{	scale = GS_MIN(facx,facy);
	}
	else
	{	float facz = dz()/b.dz();
		scale = GS_MIN3(facx,facy,facz);
	}
}

bool GsBox::contains ( const GsPnt& p ) const
{
	return p.x<a.x || p.y<a.y || p.z<a.z || p.x>b.x || p.y>b.y || p.z>b.z ? false : true;
}

bool GsBox::intersects ( const GsBox& box ) const
{
	if ( box.contains(a) ) return true;
	if ( box.contains(b) ) return true;
	GsVec x(a.x,a.y,b.z); if ( box.contains(x) ) return true;
	x.set (a.x,b.y,a.z); if ( box.contains(x) ) return true;
	x.set (b.x,a.y,a.z); if ( box.contains(x) ) return true;
	x.set (b.x,b.y,a.z); if ( box.contains(x) ) return true;
	x.set (b.x,a.y,b.z); if ( box.contains(x) ) return true;
	x.set (a.x,b.y,b.z); if ( box.contains(x) ) return true;
	return false;
}

void GsBox::get_side ( GsPnt& p1, GsPnt& p2, GsPnt& p3, GsPnt& p4, int s ) const
{
	switch (s)
	{ case 0 : p1.set ( a.x, a.y, a.z );
			   p2.set ( a.x, a.y, b.z );
			   p3.set ( a.x, b.y, b.z );
			   p4.set ( a.x, b.y, a.z );
			   break;
	  case 1 : p1.set ( b.x, a.y, a.z );
			   p2.set ( b.x, b.y, a.z );
			   p3.set ( b.x, b.y, b.z );
			   p4.set ( b.x, a.y, b.z );
			   break;
	  case 2 : p1.set ( a.x, a.y, a.z );
			   p2.set ( b.x, a.y, a.z );
			   p3.set ( b.x, a.y, b.z );
			   p4.set ( a.x, a.y, b.z );
			   break;
	  case 3 : p1.set ( a.x, b.y, a.z );
			   p2.set ( a.x, b.y, b.z );
			   p3.set ( b.x, b.y, b.z );
			   p4.set ( b.x, b.y, a.z );
			   break;
	  case 4 : p1.set ( a.x, a.y, a.z );
			   p2.set ( a.x, b.y, a.z );
			   p3.set ( b.x, b.y, a.z );
			   p4.set ( b.x, a.y, a.z );
			   break;
	  case 5 : p1.set ( a.x, a.y, b.z );
			   p2.set ( b.x, a.y, b.z );
			   p3.set ( b.x, b.y, b.z );
			   p4.set ( a.x, b.y, b.z );
			   break;
	}
}

void GsBox::rotate ( const GsQuat& q )
{
	GsBox x; // init as an empty box

	if ( empty() ) return;

	GsVec v(a); x.extend(q.apply(v));
	v.x=b.x; x.extend(q.apply(v));
	v.y=b.y; x.extend(q.apply(v));
	v.x=a.x; x.extend(q.apply(v));
	v.z=b.z; x.extend(q.apply(v));
	v.x=b.x; x.extend(q.apply(v));
	v.y=a.y; x.extend(q.apply(v));
	v.x=a.x; x.extend(q.apply(v));
	*this = x;
}

void GsBox::operator += ( const GsVec& v )
{
	a += v;
	b += v;
}

void GsBox::operator *= ( float s )
{
	a *= s;
	b *= s;
}

//============================== friends ========================================

GsBox operator * ( const GsBox& b, const GsMat& m )
{
	GsBox x; // init as an empty box

	if ( b.empty() ) return x;

	GsVec v(b.a); x.extend(v*m);
	v.x=b.b.x; x.extend(v*m);
	v.y=b.b.y; x.extend(v*m);
	v.x=b.a.x; x.extend(v*m);
	v.z=b.b.z; x.extend(v*m);
	v.x=b.b.x; x.extend(v*m);
	v.y=b.a.y; x.extend(v*m);
	v.x=b.a.x; x.extend(v*m);
	return x;
}

GsBox operator* ( const GsMat& m, const GsBox& b )
{
	GsBox x; // init as an empty box

	if ( b.empty() ) return x;

	GsVec v(b.a); x.extend(m*v);
	v.x=b.b.x; x.extend(m*v);
	v.y=b.b.y; x.extend(m*v);
	v.x=b.a.x; x.extend(m*v);
	v.z=b.b.z; x.extend(m*v);
	v.x=b.b.x; x.extend(m*v);
	v.y=b.a.y; x.extend(m*v);
	v.x=b.a.x; x.extend(m*v);
	return x;
}

GsOutput& operator<< ( GsOutput& o, const GsBox& box )
{
	return o << box.a << ' ' << box.b;
}

GsInput& operator>> ( GsInput& in, GsBox& box )
{
	return in >> box.a >> box.b;
}

//================================ End of File =================================================
