/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>
# include <sig/gs_vec.h>

//===========================================================================

const GsVec GsVec::null ( 0, 0, 0 );
const GsVec GsVec::minusone ( -1.0f, -1.0f, -1.0f );
const GsVec GsVec::one ( 1.0f, 1.0f, 1.0f );
const GsVec GsVec::i ( 1.0f, 0.0f, 0.0f );
const GsVec GsVec::j ( 0.0f, 1.0f, 0.0f );
const GsVec GsVec::k ( 0.0f, 0.0f, 1.0f );

//============================== GsVec ====================================

void GsVec::rotx ( float radians )
{
	rotx ( sinf(radians), cosf(radians) );
}

void GsVec::roty ( float radians )
{
	roty ( sinf(radians), cosf(radians) );
}

void GsVec::rotz ( float radians )
{
	rotz ( sinf(radians), cosf(radians) );
}

void GsVec::normalize ()
{
	float f = sqrtf ( x*x + y*y + z*z );
	if ( f>0 ) { x/=f; y/=f; z/=f; }
}

float GsVec::len ( float n )
{
	float f = sqrtf ( x*x + y*y + z*z );
	if ( f>0 ) { n/=f; x*=n; y*=n; z*=n; }
	return f;
}

float GsVec::norm () const
{
	return sqrtf ( x*x + y*y + z*z );
}

float GsVec::normax () const
{
	float a = GS_ABS(x);
	float b = GS_ABS(y);
	float c = GS_ABS(z);
	return GS_MAX3 ( a, b, c );
}

void GsVec::cross ( const GsVec& v1, const GsVec& v2 )
 {
   x = v1.y*v2.z - v1.z*v2.y;
   y = v1.z*v2.x - v1.x*v2.z;
   z = v1.x*v2.y - v1.y*v2.x;
 }

//=================================== Friend Functions ===================================

GsVec cross ( const GsVec& v1, const GsVec& v2 )
 {
   return GsVec ( v1.y*v2.z - v1.z*v2.y,
				  v1.z*v2.x - v1.x*v2.z,
				  v1.x*v2.y - v1.y*v2.x  );
 }

void swap ( GsVec& v1, GsVec& v2 )
 {
   float tmp;
   GS_SWAP(v1.x,v2.x);
   GS_SWAP(v1.y,v2.y);
   GS_SWAP(v1.z,v2.z);
 }

float distmax ( const GsVec& v1, const GsVec& v2 )
 {
   float a = v1.x-v2.x;
   float b = v1.y-v2.y;
   float c = v1.z-v2.z;
   a = GS_ABS(a);
   b = GS_ABS(b);
   c = GS_ABS(c);
   return a>b? (a>c? a:c) : (b>c? b:c);
 }

float dist ( const GsVec& v1, const GsVec& v2 )
 {
   float a = v1.x-v2.x;
   float b = v1.y-v2.y;
   float c = v1.z-v2.z;
   return sqrtf (a*a + b*b + c*c);
 }

float dist2 ( const GsVec& v1, const GsVec& v2 )
 {
   float a = v1.x-v2.x;
   float b = v1.y-v2.y;
   float c = v1.z-v2.z;
   return a*a + b*b + c*c;
 }

float angle ( const GsVec& v1, const GsVec& v2 )
 {
   double n = sqrt(v1.x*v1.x+v1.y*v1.y+v1.z*v1.z) * sqrt(v2.x*v2.x+v2.y*v2.y+v2.z*v2.z);
   // n will be 0 if a given vector is null, so check that here:
   if ( n==0 ) return 0;
   // get the cosine of the angle:
   n = double(v1.x*v2.x + v1.y*v2.y + v1.z*v2.z) / n;
   // n has to be in [-1,1] and numerical aproximations may result in values
   // slightly outside the interval; so the test below is really needed:
   if ( n<-1.0 ) n=-1.0; else if ( n>1.0 ) n=1.0;
   return (float) acos ( n );
 }

float anglenorm ( const GsVec& v1, const GsVec& v2 )
 {
   return acosf ( dot(v1,v2) );
   double n = double(v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
   if ( n<-1.0 ) n=-1.0; else if ( n>1.0 ) n=1.0;
   return (float) acos ( n );
 }

GsVec normal ( const GsPnt& a, const GsPnt& b, const GsPnt& c )
 { 
   GsVec n; 
   n.cross ( b-a, c-a ); 
   n.normalize(); 
   return n; 
 }

GsVec barycentric ( const GsPnt& a, const GsPnt& b, const GsPnt& c, const GsVec &p )
 {
   GsVec u = cross ( c-a, b-a );
   float ulen = u.len(); // 2 * triangle {a, b, c} area
   GsVec ua = cross ( c-p, b-p ); 
   GsVec ub = cross ( a-p, c-p ); 
   GsVec uc = cross ( b-p, a-p ); 
   GsVec bc ( ua.len()/ulen, ub.len()/ulen, uc.len()/ulen );
   if ( dot(u,ua)<0 ) bc.x = -bc.x;
   if ( dot(u,ub)<0 ) bc.y = -bc.y;
   if ( dot(u,uc)<0 ) bc.z = -bc.z;
   return bc;
 }

void transbarycentric ( GsPnt& a, GsPnt& b, GsPnt& c, const GsVec &k, const GsVec& v )
 {
   float k2 = k.x*k.x + k.y*k.y + k.z*k.z;

   a += (k.x+1.0f-k2)*v;
   b += (k.y+1.0f-k2)*v;
   c += (k.z+1.0f-k2)*v;
 }

int GsVec::compare ( const GsVec& v1, const GsVec& v2 )
{
	if ( v1.x > v2.x ) return  1;
	if ( v1.x < v2.x ) return -1;
	if ( v1.y > v2.y ) return  1;
	if ( v1.y < v2.y ) return -1;
	if ( v1.z > v2.z ) return  1;
	if ( v1.z < v2.z ) return -1;
	return 0;
}

//================================== End of File ===========================================

