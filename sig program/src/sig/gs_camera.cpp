/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
 
# include <math.h>
# include <sig/gs_box.h>
# include <sig/gs_plane.h>
# include <sig/gs_camera.h>
# include <sig/gs_string.h>

//# define GS_USE_TRACE1 // ray
# include <sig/gs_trace.h>

//=================================== GsCamera ===================================

GsCamera::GsCamera ()
{
	init ();
}

GsCamera::GsCamera ( const GsCamera &c )
		 :eye(c.eye), center(c.center), up(c.up)
{
	fovy  = c.fovy;
	znear = c.znear;
	zfar  = c.zfar;
	aspect = c.aspect;
}

void GsCamera::init () 
{
	eye.set ( 0.0f, 0.0f, 1.0f ); 
	center = GsVec::null;
	up = GsVec::j;

	fovy  = GS_TORAD(60);
	znear = 1.0f; 
	zfar  = 1000.0f; 
	aspect = 1.0f;
}

void GsCamera::getmat ( GsMat& m ) const
{
	GsMat c(GsMat::NoInit);
	getmat ( m, c );
	m *= c;
}

// screenpt coords range in [-1,1]
void GsCamera::getray ( float winx, float winy, GsPnt& p1, GsPnt& p2 ) const
{
	p1.set ( winx, winy, znear ); // p1 is in the near clip plane
	GS_TRACE1 ( "Ray A: "<< p1 );

	GsMat M(GsMat::NoInit);
	getmat(M);
	M.invert();

	p1 = M * p1;
	GS_TRACE1 ( "Ray B: "<< p1 );
	p2 = p1-eye; // ray is in camera coordinates
	p2.len(zfar-znear); // zfar and znear are >0
	p2 += p1;
	GS_TRACE1 ( "Ray C: "<< p1 <<" : "<< p2 );
}

void GsCamera::rotate ( const GsQuat& dq )
{
	GsMat C ( GsMat::NoInit );
	C.lookat ( eye, center, up );
	GsQuat q(C);
	q = q.inverse()*dq*q;

	eye -= center;
	eye = q.apply(eye);
	eye += center;
	up = q.apply(up);
}

//=============================== friends ==========================================

GsOutput& operator<< ( GsOutput& out, const GsCamera& c )
{
	out <<	"eye    " << c.eye << gsnl <<
			"center " << c.center << gsnl <<
			"up     " << c.up << gsnl <<
			"fovy   " << GS_TODEG(c.fovy) << gsnl <<
			"znear  " << c.znear << gsnl <<
			"zfar   " << c.zfar << gsnl <<
			"aspect " << c.aspect << gsnl;

	return out;
}

GsInput& operator>> ( GsInput& inp, GsCamera& c )
{
	int n=0;
	while ( n<7 && inp.get()!=GsInput::End )
	{	if ( inp.ltoken()=="eye" ) { n++; inp>>c.eye; }
		else if ( inp.ltoken()=="center" ) { n++; inp>>c.center; }
		else if ( inp.ltoken()=="up" ) { n++; inp>>c.up; }
		else if ( inp.ltoken()=="fovy" ) { n++; inp>>c.fovy; c.fovy=GS_TORAD(c.fovy); }
		else if ( inp.ltoken()=="znear" ) { n++; inp>>c.znear; }
		else if ( inp.ltoken()=="zfar" ) { n++; inp>>c.zfar; }
		else if ( inp.ltoken()=="aspect" ) { n++; inp>>c.aspect; }
		else { inp.unget(); break; }
	}
	return inp;
}

//================================ End of File =========================================
