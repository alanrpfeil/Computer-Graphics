/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>

# include <sig/gs.h>
# include <sig/gs_euler.h>
# include <sigkin/kn_joint_rot.h>

# define INSYNC _jr->insync(KnJointRot::EU)
# define NEWVAL _jr->setowner(KnJointRot::EU)
# define SYNC   _jr->syncto(KnJointRot::EU)

//============================= KnJointEuler ============================

KnJointEuler::KnJointEuler ( KnJointRot* jr )
{
	_jr = jr;
	_type = (char)TypeYXZ;
}

void KnJointEuler::init ( const KnJointEuler* e )
{ 
	_type = e->_type;
	KnVecLimits::init ( *e );
}

void KnJointEuler::type ( Type t )
{
	if ( _type == (char)t ) return;
   
	_type = (char)t;

	if ( _type==(char)TypeZY )
	{	value ( X, 0 );
		limits ( X, true );
		limits ( X, 0, 0 ); // will freeze X axis
	}
}

void KnJointEuler::value ( int d, float val )
{
	if ( INSYNC )
	{	if ( KnVecLimits::value(d,val) ) NEWVAL;
	}
	else
	{	KnVecLimits::value(d,val);
		NEWVAL;
	}
}

void KnJointEuler::value ( float rx, float ry, float rz )
{
	if ( INSYNC )
	{	bool nv=false;
		if ( KnVecLimits::value(0,rx) ) nv=true;
		if ( KnVecLimits::value(1,ry) ) nv=true;
		if ( KnVecLimits::value(2,rz) ) nv=true;
		if ( nv ) NEWVAL;
	}
	else
	{	KnVecLimits::value(0,rx);
		KnVecLimits::value(1,ry);
		KnVecLimits::value(2,rz);
		NEWVAL;
	}
}

void KnJointEuler::init ()
{
	KnVecLimits::init();
	NEWVAL;
}

float KnJointEuler::value ( int d ) const
{
	SYNC;
	return KnVecLimits::value(d);
}
	
const float* KnJointEuler::value () const
{
	SYNC;
	return KnVecLimits::valuept();
}

void KnJointEuler::get ( GsQuat& q ) const
{
	// make sure euler values are up to date:
	SYNC;

	// TodoNote: the conversions in this method have not been verified after GsMat became line-major
	// ->See also: void gs_rot ( gsEulerOrder order, GsQuat& q, float a1, float a2, float a3 ) (gs_euler.cpp)

	if ( _type==(char)TypeYXZ )
	{	// To verify due GsMat now being line-major:
		// We want to generate quaternion: q = QuatRz * QuatRx * QuatRy
		float cx, cy, cz, sx, sy, sz;

		cx = sx = KnVecLimits::value(X)/2;
		cy = sy = KnVecLimits::value(Y)/2;
		cz = sz = KnVecLimits::value(Z)/2;

		cx = cosf ( cx );
		cy = cosf ( cy );
		cz = cosf ( cz );
		sx = sinf ( sx );
		sy = sinf ( sy );
		sz = sinf ( sz );

		float w = cx*cy;
		float x = sx*cy;
		float y = sy*cx;
		float z = sx*sy;

		q.w = cz*w - sz*z;
		q.x = x*cz - sz*y;
		q.y = sz*x + y*cz;
		q.z = sz*w + cz*z;
	}
	else if ( _type==(char)TypeXYZ )
	{	// To verify due GsMat now being line-major:
		// We want to generate quaternion: q = QuatRz * QuatRy * QuatRx
		GsQuat Qx ( GsVec::i, KnVecLimits::value(X) );
		GsQuat Qy ( GsVec::j, KnVecLimits::value(Y) );
		GsQuat Qz ( GsVec::k, KnVecLimits::value(Z) );
		q = Qz * Qy * Qx; // we could optimize this as in case YXZ...
	}
	else if ( _type==(char)TypeZY )
	{	// To verify due GsMat now being line-major:
		// We want to generate quaternion: q = Qy*Qz

		float cy, cz, sy, sz;

		cy = sy = KnVecLimits::value(Y)/2;
		cz = sz = KnVecLimits::value(Z)/2;

		cy = cosf ( cy );
		cz = cosf ( cz );
		sy = sinf ( sy );
		sz = sinf ( sz );

		q.set ( cy*cz, sy*sz, sy*cz, sz*cy ); 
	}
	else if ( _type==(char)TypeYZX )
	{	// To verify due GsMat now being line-major:
		// We want to generate quaternion: q = QuatRz * QuatRy * QuatRx
		GsQuat Qx ( GsVec::i, KnVecLimits::value(X) );
		GsQuat Qy ( GsVec::j, KnVecLimits::value(Y) );
		GsQuat Qz ( GsVec::k, KnVecLimits::value(Z) );
		q = Qx * Qz * Qy; // we could optimize this as in case YXZ...
	}
}

void KnJointEuler::set ( const GsQuat& q )
{
	// this code uses matrix convertions and thus could be a lot
	// optimized however so far there is no need for that
	GsMat m(GsMat::NoInit);
	quat2mat ( q, m ); // convert quaternion to matrix

	// extract angles from matrix
	float rx, ry, rz;
	if ( _type==(char)TypeYXZ )
	{ 
		gs_angles_yxz ( m, rx, ry, rz, 'L' );
	}
	else if ( _type==(char)TypeXYZ )
	{
		gs_angles_xyz ( m, rx, ry, rz, 'L' );
	}
	else if ( _type==(char)TypeZY )
	{
		gs_angles_zyx ( m, rx, ry, rz, 'L' );
	}
	else if ( _type==(char)TypeYZX )
	{
		gs_angles_yzx ( m, rx, ry, rz, 'L' );
	}

	value ( rx, ry, rz );
}

//============================ End of File ============================
