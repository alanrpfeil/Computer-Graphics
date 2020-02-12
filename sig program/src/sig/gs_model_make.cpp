/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>

# include <sig/gs_model.h>
# include <sig/gs_quat.h>
# include <sig/gs_polygon.h>

//# define GS_USE_TRACE1 // 
# include <sig/gs_trace.h>

//=================================== GsModel =================================================

# define MINRADIUS 0.001f

void GsModel::make_sweep ( const GsPolygon& pol, const GsVec& v, bool bot, bool top )
{
	if ( pol.size()<3 ) return;

	init ();
	name = "sweep";

	// Add all vertices needed:
	int i, ps = pol.size();
	for ( i=0; i<ps; i++ ) V.push().set ( pol[i].x, pol[i].y, 0.0f );
	for ( i=0; i<ps; i++ ) { V.push().set ( pol[i].x, pol[i].y, 0.0f ); V.top()+=v; }

	// Add bottom and/or top:
	if ( bot || top )
	{	GsArray<int> t;
		pol.ear_triangulation ( t );
		int ts = t.size();
		if ( bot ) { for ( i=0; i<ts; i+=3 ) F.push().set ( t[i], t[i+2], t[i+1] ); }
		if ( top ) { for ( i=0; i<ts; i+=3 ) F.push().set ( t[i]+ps, t[i+1]+ps, t[i+2]+ps ); }
	}

	// Add sides:
	for ( i=0; i<ps; i++ )
	{	int i2 = (i+1)%ps;
		F.push().set ( i, i2, i+ps );
		F.push().set ( i+ps, i2, i2+ps );
	}
}

void GsModel::make_tube ( const GsPnt& a, const GsPnt& b, float ra, float rb, int nfaces, bool smooth )
{
	if ( ra<=0 ) ra=MINRADIUS;
	if ( rb<=0 ) rb=MINRADIUS;

	init ();
	name = "tube";
	if ( nfaces<3 ) nfaces = 3;
   
	// compute vertical axis:
	float dang = gs2pi/float(nfaces);
	GsVec vaxis = b-a; 
	GsVec va = vaxis; 
	va.normalize(); // axial vector

	// compute radial vectors:
	GsVec vra;
	float deg = GS_TODEG ( angle(GsVec::i,va) );
	if ( deg<10 || deg>170 )
		vra = cross ( GsVec::j, va );
	else
		vra = cross ( GsVec::i, va );
	GsVec vrb(vra);
	vra.len ( ra );
	vrb.len ( rb );

	// compute points of first face:
	GsQuat rot ( va, dang );
	GsPnt a1 = a+vra;
	GsPnt b1 = b+vrb;
	GsPnt a2 = a+rot.apply(vra);
	GsPnt b2 = b+rot.apply(vrb);

	// compute points around body:
	int i=1;
	do
	{	if ( smooth )
		{	if ( ra==rb )
			{	N.push()=(a1-a)/ra; } // normalized normal
			else // adjust normal for our cylinder with a "cone body"
			{	GsVec v = b1-a1;
				N.push() = ::normalize ( cross ( v, cross(v,GsVec::j) ) );
			}
		}
		V.push()=a1; V.push()=b1;
		if ( i==nfaces ) break;
		a1=a2; a2=a+rot.apply(a1-a);
		b1=b2; b2=b+rot.apply(b1-b);
		i++;
	} while ( true );

	// compute the faces:
	int n1, n=0;
	int i1, i2, i3;
	int size = V.size();
	for ( i=0; i<size; i+=2 )
	{	i1 = V.vidpos ( i+1 );
		i2 = V.vidpos ( i+2 );
		i3 = V.vidpos ( i+3 );
		F.push().set ( i, i2, i1 );
		F.push().set ( i1, i2, i3 );
		if ( smooth )
		{	n1 = N.vid ( n+1 );
			Fn.push().set ( n, n1, n );
			Fn.push().set ( n, n1, n1 );
			n++;
		}
	}

	if ( smooth )
	{	_geomode=Hybrid;
	}
	else
	{	_geomode=Flat; 
	}
	compress ();
}

void GsModel::make_box ( const GsBox& b )
{
	init ();

	name = "box";
	V.size ( 8 );

	// side 4 has all z coordinates equal to a.z, side 5 equal to b.z
	if ( b.a==b.b )
	{	GsVec min (MINRADIUS,MINRADIUS,MINRADIUS);
		GsBox box ( b.b+min, b.a-min );
		box.get_side ( V[0], V[1], V[2], V[3], 4 );
		box.get_side ( V[4], V[5], V[6], V[7], 5 );
	}
	else
	{	b.get_side ( V[0], V[1], V[2], V[3], 4 );
		b.get_side ( V[4], V[5], V[6], V[7], 5 );
	}

	F.size ( 12 );
	F[0].set(1,0,4); F[1].set(1,4,7); // plane crossing -X
	F[2].set(3,2,6); F[3].set(3,6,5); // plane crossing +X
	F[4].set(7,4,6); F[5].set(4,5,6); // plane crossing +Z
	F[6].set(0,1,2); F[7].set(0,2,3); // plane crossing -Z
	F[8].set(2,1,7); F[9].set(2,7,6); // plane crossing +Y
	F[10].set(0,3,4); F[11].set(4,3,5); // plane crossing -Y

	compress();
}

static int getv ( GsArray<GsVec>& V, int a, int b, float r )
{
	GsPnt p = (V[a]+V[b])/2;
	p.len(r);

	// PerfNote: this is not very efficient, but ok for our needs:
	int i;
	for ( i=V.size()-1; i>=0; i-- )
		if ( next(V[i],p,gstiny) ) break;
   
	if ( i<0 ) { i=V.size(); V.push()=p; } // append

	return i;
}

static void subtriface ( GsModel* m, int fac, float r, int depth )
{
	if ( depth==1 ) return;
	int a=m->F[fac].a; int b=m->F[fac].b; int c=m->F[fac].c;

	int d=getv(m->V,a,b,r);
	int e=getv(m->V,b,c,r);
	int f=getv(m->V,c,a,r);

	m->F[fac].set ( d, e, f ); subtriface ( m, fac, r, depth-1 );
	m->F.push().set ( a, d, f ); subtriface ( m, m->F.size()-1, r, depth-1 );
	m->F.push().set ( b, e, d ); subtriface ( m, m->F.size()-1, r, depth-1 );
	m->F.push().set ( c, f, e ); subtriface ( m, m->F.size()-1, r, depth-1 );
}

void GsModel::make_sphere ( const GsPnt& c, float r, int nfaces, bool smooth )
{
	if ( r<=0 ) r=MINRADIUS;
	init ();

	name = "sphere";

	int depth = (int)sqrt((double)nfaces);
	if ( depth<1 ) depth=1;
	if ( depth>6 ) depth=6;

	const float z = 0.0f;
	V.push().set(r,z,z);
	V.push().set(z,z,-r);
	V.push().set(-r,z,z);
	V.push().set(z,z,r);
	V.push().set(z,-r,z);
	V.push().set(z,r,z);

	F.push().set ( 0, 1, 5 );
	F.push().set ( 1, 2, 5 );
	F.push().set ( 2, 3, 5 );
	F.push().set ( 3, 0, 5 );
	F.push().set ( 1, 0, 4 );
	F.push().set ( 2, 1, 4 );
	F.push().set ( 3, 2, 4 );
	F.push().set ( 0, 3, 4 );

	int i;
	for ( i=0; i<8; i++ ) subtriface ( this, i, r, depth );

	if ( smooth )
	{	int vs=V.size();
		N.sizecap(vs,vs);
		for ( i=0; i<vs; i++ ) { N[i]=V[i]; N[i].normalize(); }
		_geomode=Smooth;
	}

	if ( c!=GsPnt::null ) translate(c);
	compress();
}

void GsModel::make_ellipsoid ( const GsPnt& c, float r, float ratio, int nfaces, bool smooth )
{
	if ( r<MINRADIUS ) r=MINRADIUS;
	if ( ratio<=MINRADIUS ) ratio=MINRADIUS;

	init ();

	name = "ellipsoid";

	int depth = (int)sqrt((double)nfaces);
	if ( depth<1 ) depth=1;
	if ( depth>6 ) depth=6;

	const float z = 0.0f;
	V.push().set(r,z,z);
	V.push().set(z,z,-r);
	V.push().set(-r,z,z);
	V.push().set(z,z,r);
	V.push().set(z,-r,z);
	V.push().set(z,r,z);

	F.push().set ( 0, 1, 5 );
	F.push().set ( 1, 2, 5 );
	F.push().set ( 2, 3, 5 );
	F.push().set ( 3, 0, 5 );
	F.push().set ( 1, 0, 4 );
	F.push().set ( 2, 1, 4 );
	F.push().set ( 3, 2, 4 );
	F.push().set ( 0, 3, 4 );

	int i;
	for ( i=0; i<8; i++ )
		subtriface ( this, i, r, depth );

	for ( i=0; i<V.size(); i++ ) // just scale down X & Z axes from a sphere
	{
		V[i].x = V[i].x * ratio;
		V[i].z = V[i].z * ratio;
	}

	if ( smooth ) GsModel::smooth(); // smooth() may be slow, ok for low face count, but should be avoided

	if ( c!=GsPnt::null ) translate(c);
	compress();
}

void GsModel::make_cylinder ( const GsPnt& a, const GsPnt& b, float ra, float rb, int nfaces, bool smooth )
{
	if ( ra<=0 ) ra=MINRADIUS;
	if ( rb<=0 ) rb=MINRADIUS;
	make_tube ( a, b, ra, rb, nfaces, smooth );
	name = "cylinder";
   
	GsVec vaxis = b-a; 
	GsVec va = vaxis; 
	va.normalize(); // axial vector
	GsVec minus_va = va * -1.0f;

	// make top and bottom:
	int i, i1, i2, i3, n=0;
	int size = V.size();
	for ( i=0; i<size; i+=2 )
	{	i1 = V.vidpos ( i+1 );
		i2 = V.vidpos ( i+2 );
		i3 = V.vidpos ( i+3 );

		F.push().set ( size, i2, i );
		F.push().set ( size+1, i1, i3 );

		if ( smooth )
		{	Fn.push().set ( N.size(), N.size(), N.size() );
			Fn.push().set ( N.size()+1, N.size()+1, N.size()+1 );
			n++;
		}
	}

	V.push(a);
	V.push(b);
	if ( smooth )
	{	N.push()=minus_va;
		N.push()=va;
	}

	compress ();
}

static void arc ( GsArray<GsVec>& V, const GsPnt& c, const GsVec& vaxis, const GsPnt& ini, int nfaces )
{
	int i;
	float dang = gspidiv2/float(nfaces);
	GsPnt p=ini-c;
	GsQuat rot ( cross(p,vaxis), dang );

	for ( i=1; i<nfaces; i++ )
	{	p = rot.apply(p);
		V.push() = c+p;
	}
}

void GsModel::make_capsule ( const GsPnt& a, const GsPnt& b, float ra, float rb, int nfaces, bool smooth )
{
	// smooth capsule not implemented yet

	if ( ra<=0 ) ra=MINRADIUS;
	if ( rb<=0 ) rb=MINRADIUS;

	make_tube ( a, b, ra, rb, nfaces, false );
	name = "capsule";

	nfaces = (nfaces+2)/4;  // arc will be 1/4 of a circle
	if ( nfaces<3 ) nfaces = 3;
	GsVec vaxis = b-a; 
	int vsize = V.size(); // 2 times the num of vertices around a face

	// add north and south poles:
	vaxis.len(ra); GsPnt southp=a-vaxis; int southi=V.size(); V.push()=southp;
	vaxis.len(rb); GsPnt northp=b+vaxis; int northi=V.size(); V.push()=northp;
	vaxis.normalize();

	// make 1st top arc:
	int arc1 = V.size();
	arc ( V, b, vaxis, V[1], nfaces );
	int arcsize = V.size()-arc1;
	int arc2;

	// complete:
	int i, j, i1, i2, i3;
	for ( i=0; i<vsize; i+=2 )
	{	i1 = (i+1)%vsize;
		i3 = (i+3)%vsize;

		if ( i<vsize-2 )
		{	arc2 = V.size();
			arc ( V, b, vaxis, V[i3], nfaces );
		}
		else
		{	arc2 = vsize+2; // back to the very first arc
		}

		F.push().set ( arc1, i1, i3 );
		F.push().set ( arc2, arc1, i3 );

		for ( j=1; j<arcsize; j++ )
		{	F.push().set ( arc1+j, arc1+j-1, arc2+j-1 );
			F.push().set ( arc1+j, arc2+j-1, arc2+j );
		}
		F.push().set ( northi, arc1+arcsize-1, arc2+arcsize-1 );
		arc1=arc2;
	}

	// make 1st bottom arc:
	arc1 = V.size();
	int arcini = arc1;
	arc ( V, a, -vaxis, V[0], nfaces );

	// complete:
	for ( i=0; i<vsize; i+=2 )
	{	i1 = (i+1)%vsize; 
		i2 = (i+2)%vsize; 

		if ( i<vsize-2 )
		{	arc2 = V.size();
			arc ( V, a, -vaxis, V[i2], nfaces );
		}
		else
		{	arc2 = arcini; } // back to the very first arc

		F.push().set ( arc1, i2, i );
		F.push().set ( arc1, arc2, i2 );
		for ( j=1; j<arcsize; j++ )
		{	F.push().set ( arc1+j-1, arc1+j, arc2+j-1 );
			F.push().set ( arc2+j-1, arc1+j, arc2+j );
		}
		F.push().set ( arc1+arcsize-1, southi, arc2+arcsize-1 );
		arc1=arc2;
	}

	if ( smooth ) GsModel::smooth(0);
	compress ();
}

void GsModel::make_primitive ( const GsPrimitive& p, MakePrimitiveMtlChoice mtlchoice )
{
	GsMaterial mtl = p.material;
	if ( mtlchoice==UseModelMtl && M.size()>0 ) mtl=M[0];

	switch ( p.type )
	{	case GsPrimitive::Box :
		{	GsBox b ( GsPnt(-p.ra,-p.rb,-p.rc), GsPnt(p.ra,p.rb,p.rc) );
			make_box ( b );
		} break;

		case GsPrimitive::Sphere :
		{	make_sphere ( GsPnt::null, p.ra, p.nfaces, p.smooth==1 );
		} break;

		case GsPrimitive::Cylinder :
		{	GsVec v ( 0.0f, p.rc, 0.0f );
			make_cylinder ( -v, v, p.ra, p.rb, p.nfaces, p.smooth==1 );
		} break;

		case GsPrimitive::Capsule :
		{	GsVec v ( 0.0f, p.rc, 0.0f );
			make_capsule ( -v, v, p.ra, p.rb, p.nfaces, p.smooth==1 );
		} break;

		case GsPrimitive::Ellipsoid :
		{	make_ellipsoid ( GsPnt::null, p.ra, p.rc, p.nfaces, p.smooth==1 );
		} break;
	}

	GsPrimitive* modelprim = primitive? primitive : new GsPrimitive;
	primitive=0; // set as NULL so that rotate() and translate() will not accumulate transformations in primitive

	if ( p.orientation!=GsQuat::null ) rotate ( p.orientation );
	if ( p.center!=GsVec::null ) translate ( p.center );

	primitive = modelprim;
	*(primitive) = p;

	if ( mtlchoice!=UseNoMtl ) set_one_material ( mtl );
}

//================================ End of File =================================================
