/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_primitive.h>
# include <sig/gs_string.h>
# include <math.h>

//===================================== GsPrimitive =======================================

const char* GsPrimitive::typestring () const
{
	switch ( type )
	{	case Box : return "box";
		case Sphere : return "sphere";
		case Cylinder : return "cylinder";
		case Capsule : return "capsule";
		case Ellipsoid : return "ellipsoid";
	}
	return "unknown";
}

void GsPrimitive::box ( const GsBox& b )
{
	type = Box;
	center = b.center();
	ra = b.dx()/2.0f;
	rb = b.dy()/2.0f;
	rc = b.dz()/2.0f;
}

void GsPrimitive::cylinder ( const GsPnt& a, const GsPnt& b, float radius, int nf )
{
	type = Cylinder;
	rc = dist(a,b)/2.0f;
	center = (a+b)/2.0f;
	orientation.set ( GsVec::j, b-a );
	if ( radius>0 ) ra = rb = radius;
	if ( nf>0 ) nfaces = nf;
}

void GsPrimitive::get_bounding_box ( GsBox& box ) const
{
	float r = GS_MAX(ra,rb);
	switch ( type )
	{	case Box : box.set ( GsVec(-ra,-rb,-rc), GsVec(ra,rb,rc) ); break;
		case Sphere : box.set ( center, ra ); return; // sphere case has no need to account for orientation
		case Cylinder : box.set ( GsVec(-r,-rc,-r), GsVec(r,rc,r) ); break;
		case Capsule : box.set ( GsVec(-r,-rc-ra,-r), GsVec(r,rc+rb,r) ); break;
		case Ellipsoid : r*=rc; box.set ( GsVec(-r,-ra,-r), GsVec(r,ra,r) ); break;
		default: box.set ( center, ra ); return;
	}

	if ( orientation!=GsQuat::null ) box.rotate(orientation);
	if ( center!=GsVec::null ) box+=center;
}

GsOutput& operator<< ( GsOutput& o, const GsPrimitive& p )
{
	o.outm(); o<<p.typestring()<<gspc;

	if ( p.type==GsPrimitive::Sphere )
	{	o<<p.ra; }
	else
	{	o<<p.ra<<gspc<<p.rb<<gspc<<p.rc; }

	o<<gspc<<p.nfaces;

	GsMaterial mtl; // mtl will have default material values

	if ( p.center!=GsVec::null ) { o<<gsnl; o.outm(); o << "center " << p.center; }
	if ( p.orientation!=GsQuat::null ) { o<<gsnl; o.outm(); o << "orientation " << p.orientation; }
	if ( p.material!=mtl ) { o<<gsnl; o.outm(); o << "material " << p.material; }
	if ( p.smooth==0 ) { o<<gsnl; o.outm(); o << "smooth"; }
	o << ";\n";
	return o;
}

GsInput& operator>> ( GsInput& in, GsPrimitive& p )
{
	GsVec r=GsVec::one;

	in.get();
	if ( in.ltoken()=="box" )
	{	p.type=GsPrimitive::Box; in>>r; in>>p.nfaces; }
	else if ( in.ltoken()=="sphere" )
	{	p.type=GsPrimitive::Sphere; in>>r.x; in>>p.nfaces; }
	else if ( in.ltoken()=="cylinder" )
	{	p.type=GsPrimitive::Cylinder; in>>r; in>>p.nfaces; }
	else if ( in.ltoken()=="capsule" )
	{	p.type=GsPrimitive::Capsule; in>>r; in>>p.nfaces; }

	p.ra=r.x; p.rb=r.y; p.rc=r.z;

	p.center = GsVec::null;
	p.orientation = GsQuat::null;
	p.material.init();
	p.smooth = true;

	while ( in.get()==GsInput::String )
	{
		if ( in.ltoken()=="center" )
		{	in >> p.center; }
		else if ( in.ltoken()=="orientation" )
		{	in >> p.orientation; }
		else if ( in.ltoken()=="material" )
		{	in >> p.material; }
		else if ( in.ltoken()=="color" )
		{	in >> p.material.diffuse; }
		else if ( in.ltoken()=="flat" )
		{	p.smooth=false; }
		else if ( in.ltoken()=="smooth" )
		{	p.smooth=true; }
		else break;
	}

	if ( in.ltype()!=GsInput::Delimiter ) in.unget(); // prevent errors in case ';' is absent
	return in;
}

//===================================== End of File ========================================

// Supper Ellipsoid code for future extension:

// SupperEllip: a,b,c:scales, m,n:shape exponents.
// enumt Type { ..., SuperEllip};
//# define DPOW(a,b) pow(double(a),double(b))

/*! Evaluates a supper ellipsoid represented by the following parameters:
	a,b,c: scalings on each axis, and m,n: shape control exponents.
	For example, m=n=1:sphere, m=n=.5=rounded square, m=1,n=0:cylinder, etc.
	The implicit form is: [ (x/a)^(2/n)+(y/b)^(2/n) ]^(n/m) + (z/c)^(2/m) */
//float gs_eval_superellip ( const GsPnt& p, float a, float b, float c, float m, float n );

/*! Evaluates the supper ellipsoid parametric equation, u in [-pi/2,pi/2], v in [-pi,pi]:
	[ a cos^m(u) cos^n(v), b cos^m(u) sin^n(v), c sin^m(u) ] */
//GsPnt gs_eval_superellip ( float u, float v, float a, float b, float c, float m, float n );

/*float gs_eval_superellip ( const GsPnt& p, float a, float b, float c, float m, float n )
 {
   if ( n==0 || m==0 ) return 1;
   float no2 = 2/n;
   return (float) ( DPOW( DPOW(p.x/a,no2)+DPOW(p.y/b,no2), n/m ) + DPOW ( p.z/c, 2/m ) );
 }

GsPnt gs_eval_superellip ( float u, float v, float a, float b, float c, float m, float n )
 {
   double pcosum = pow(cos(u),m);

   return GsPnt ( (float) ( a*pcosum*DPOW(cos(v),n) ),
				  (float) ( b*pcosum*DPOW(sin(v),n) ),
				  (float) ( c*DPOW(sin(u),m) ) 
				);
   // -pi/2<=u<=pi/2, -pi<=v<=pi
 }
*/
