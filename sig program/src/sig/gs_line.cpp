/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>
# include <sig/gs_box.h>
# include <sig/gs_line.h>
# include <sig/gs_input.h>
# include <sig/gs_output.h>

//===========================================================================

// Important: Static initializations cannot use other statically-initialized
// variables (like GsVec::i etc.) because order of initialization is unknown

const GsLine GsLine::x ( GsPnt(0,0,0), GsVec(1,0,0) );
const GsLine GsLine::y ( GsPnt(0,0,0), GsVec(0,1,0) );
const GsLine GsLine::z ( GsPnt(0,0,0), GsVec(0,0,1) );

//============================== GsLine ====================================

#define EPSILON 0.00001 // floats have 7 decimals

bool GsLine::intersects_triangle ( const GsPnt &v0, const GsPnt &v1, const GsPnt &v2,
								   float &t, float &u, float &v ) const
{
	GsVec dir, edge1, edge2, tvec, pvec, qvec;
	float det, inv_det;

	dir   = p2 - p1;
	edge1 = v1 - v0;					  // find vectors for two edges sharing v0 
	edge2 = v2 - v0;
	pvec  = cross ( dir, edge2 );		 // begin calculating determinant - also used to calculate U parameter 
	det   = dot ( edge1, pvec );		  // if determinant is near zero, ray lies in plane of triangle 

	if ( GS_NEXTZ(det,EPSILON) )
	{	//gsout.warning("det in ray_triangle fails => %f",(float)det);
		return false;
	}
	inv_det = 1.0f / det;

	tvec = p1 - v0;					   // calculate distance from v0 to ray origin 
	u = dot(tvec, pvec) * inv_det;		// calculate U parameter and test bounds 
	if ( u<0.0 || u>1.0 ) return false;

	qvec = cross ( tvec, edge1 );		 // prepare to test V parameter 
	v = dot(dir, qvec) * inv_det;		 // calculate V parameter and test bounds 
	if ( v<0.0 || u+v>1.0 ) return false;
	t = dot(edge2,qvec) * inv_det;		// calculate t, ray intersects triangle 

	return true;
}

bool GsLine::intersects_square ( const GsPnt &v1, const GsPnt &v2,
								 const GsPnt &v3, const GsPnt &v4, float& t ) const
{
	float u, v;
	if ( intersects_triangle ( v1, v2, v3, t, u, v ) ) return true;
	if ( intersects_triangle ( v1, v3, v4, t, u, v ) ) return true;
	return false;
}

int GsLine::intersects_box ( const GsBox& box, float& t1, float& t2, GsPnt* vp ) const
{
	GsVec p1, p2, p3, p4, p;
	float t[6];
	int side[6];
	int tsize=0;

	# define INTERSECT(s) if ( intersects_square(p1,p2,p3,p4,t[tsize]) ) { side[tsize]=s; tsize++; }

	box.get_side ( p1, p2, p3, p4, 0 );
	INTERSECT(0);
	box.get_side ( p1, p2, p3, p4, 1 );
	INTERSECT(1);
	box.get_side ( p1, p2, p3, p4, 2 );
	INTERSECT(2);
	box.get_side ( p1, p2, p3, p4, 3 );
	INTERSECT(3);
	box.get_side ( p1, p2, p3, p4, 4 );
	INTERSECT(4);
	box.get_side ( p1, p2, p3, p4, 5 );
	INTERSECT(5);

	# undef INTERSECT

	if ( tsize==0 )
	{	t1=t2=0; }
	else if ( tsize==1 )
	{	t1=t2=t[0]; }
	else if ( tsize==2 )
	{	float tmpf;
		int tmpi;
		if ( t[1]<t[0] ) { GS_SWAPT(t[0],t[1],tmpf); GS_SWAPT(side[0],side[1],tmpi); }
		t1 = t[0];
		t2 = t[1];
	}
	else // sort according to t and take the two extremes
	{	int i, j, tmpi;
		float tmpf;
		for ( i=0; i<tsize; i++ )
			for ( j=i; j<tsize; j++ )
				if ( t[j]<t[i] ) { GS_SWAPT(t[i],t[j],tmpf); GS_SWAPT(side[i],side[j],tmpi); }
		t1 = t[0];
		t2 = t[tsize-1];
		tsize = 2;
	}

	if (tsize>0 && vp) box.get_side ( vp[0], vp[1], vp[2], vp[3], side[0] );
	return tsize;
}

int GsLine::intersects_sphere ( const GsPnt& center, float radius, GsPnt* vp ) const
{
	// set up quadratic Q(t) = a*t^2 + 2*b*t + c
	GsVec dir = p2-p1;
	GsVec kdiff = p1 - center;
	float a = dir.norm2();
	float b = dot ( kdiff, dir );
	float c = kdiff.norm2() - radius*radius;

	float aft[2];
	float discr = b*b - a*c;

	if ( discr < 0.0f )
	{	return 0;
	}
	else if ( discr > 0.0f )
	{	float root = sqrtf(discr);
		float inva = 1.0f/a;
		aft[0] = (-b - root)*inva;
		aft[1] = (-b + root)*inva;
		if ( vp )
		{	vp[0] = p1 + aft[0]*dir;
			vp[1] = p1 + aft[1]*dir;
			if ( dist2(vp[1],p1)<dist2(vp[0],p1) )
			{	GsPnt tmp;
				GS_SWAP(vp[0],vp[1]);
			}
		}
		return 2;
	}
	else
	{	aft[0] = -b/a;
		if ( vp ) vp[0] = p1 + aft[0]*dir;
		return 1;
	}
}

GsPnt GsLine::closestpt ( GsPnt p, float* k ) const
{
	GsVec v (p2-p1);
 
	float u = ( ( (p.x-p1.x) * (v.x) ) +
				( (p.y-p1.y) * (v.y) ) +
				( (p.z-p1.z) * (v.z) ) ) / ( v.norm2() );
 
	//if( u<0.0f || u>1.0f ) // closest point does not fall within the line segment
	if ( k ) *k=u;
   
	return p1 + u*v;
}
 
//============================== friends ====================================

GsOutput& operator<< ( GsOutput& o, const GsLine& l )
{
	return o << l.p1 <<" "<< l.p2;
}

GsInput& operator>> ( GsInput& in, GsLine& l )
{
	return in >> l.p1 >> l.p2;
}

//============================= End of File ===========================================
