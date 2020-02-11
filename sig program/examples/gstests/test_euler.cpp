/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdio.h>
# include <stdlib.h>

# include <sig/gs_quat.h>
# include <sig/gs_mat.h>
# include <sig/gs_euler.h>
# include <sig/gs_random.h>
# include <sig/gs_timer.h>

# define EPS gsmall

static bool test_euler ( gsEulerOrder order, float rx, float ry, float rz )
{
	GsMat m1, m2;
	GsMat mx; mx.rotx(rx);
	GsMat my; my.roty(ry);
	GsMat mz; mz.rotz(rz);
 
	switch ( order )
	{	case gsZYX: m1 = mx*my*mz; break;
		case gsYZX: m1 = mx*mz*my; break;
		case gsZXY: m1 = my*mx*mz; break;
		case gsXZY: m1 = my*mz*mx; break;
		case gsYXZ: m1 = mz*mx*my; break;
		case gsXYZ: m1 = mz*my*mx; break;
	}

	// Test if gs_rot() correctly generates the matrix:
	gs_rot ( order, m2, rx, ry, rz );

	if ( !next(m1,m2,EPS) )
	{	gsout<<"Mat Error "<<order<<" :\n"; 
		m1.round(EPS); m2.round(EPS);
		gsout<<m1<<gsnl<<m2<<gsnl;
		return false;
	}

	// Test if gs_angles() correctly extracted the angles:
	float ex, ey, ez;
	gs_angles ( order, m1, ex, ey, ez );
	gs_rot ( order, m2, ex, ey, ez );

	if ( !next(m1,m2,EPS) )
	{	gsout<<"Angles Error "<<order<<gspc<<" :\n"; 
		gsout<<GS_TODEG(rx)<<gspc<<GS_TODEG(ry)<<gspc<<GS_TODEG(rz)<<gsnl;
		gsout<<GS_TODEG(ex)<<gspc<<GS_TODEG(ey)<<gspc<<GS_TODEG(ez)<<gsnl;
		gsout<<gsnl;
		m1.round(EPS); m2.round(EPS);
		gsout<<m1<<gsnl<<m2<<gsnl;
		return false;
	}

	// Test convertion to/from a quaternion:
	GsQuat q;
	q.set ( m1 );
	q.get ( m2 );
	if ( !next(m1,m2,EPS) )
	{	gsout<<"Quaternion convertion Error !\n"; 
		m1.round(EPS); m2.round(EPS);
		gsout<<m1<<gsnl<<m2<<gsnl;
		return false;
	}
   
	return true;
}

static float rang ()
{
	static GsRandom<int> rang (-360,360);
	static GsRandom<int> ri (-4,4);
 
	if ( gs_random()<0.50 ) // true random angle
	{
		return GS_TORAD(rang.get());
	}
	else // special cases
	{
		return float(ri.get())*gspidiv2;
	}
}
 
void test_euler ()
{
	int i;
	float rx, ry, rz;

	double t1 = gs_time();
	int max = 100000;
	int error;

	for ( i=0; i<max; i++ )
	{
		rx = rang();
		ry = rang();
		rz = rang();

		error=0;
		if ( !test_euler ( gsXYZ, rx, ry, rz ) ) error++;
		if ( !test_euler ( gsXZY, rx, ry, rz ) ) error++;
		if ( !test_euler ( gsYXZ, rx, ry, rz ) ) error++;
		if ( !test_euler ( gsYZX, rx, ry, rz ) ) error++;
		if ( !test_euler ( gsZXY, rx, ry, rz ) ) error++;
		if ( !test_euler ( gsZYX, rx, ry, rz ) ) error++;

		if ( error>0 ) break;
		if ( i%5000==0 ) gsout<<(max-i)<<"...\n";
	}

	double t2 = gs_time();

	gsout<<"Tests: "<<i<<gsnl;
	gsout<<"Errors: "<<error<<gsnl;
	gsout<<"Total time: "<<(t2-t1)<<" secs\n";
}
