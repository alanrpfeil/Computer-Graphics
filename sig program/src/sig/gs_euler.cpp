/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_euler.h>
# include <sig/gs_quat.h>
# include <math.h>

//# define GS_USE_TRACE1
# include <sig/gs_trace.h>

# define ROUNDMAT ((GsMat&)m).round(gstiny)  // use this to test special cases

# define ISZERO(a) ( (a)>-(gstiny) && (a)<(gstiny) )

# define EQUAL(a,b) ( ((a)>(b)? ((a)-(b)):((b)-(a)))<=(gstiny) )

# define GETSINCOS(fmt) double cx=cos(rx); double cy=cos(ry); double cz=cos(rz); \
						double sx=sin(rx); double sy=sin(ry); double sz=sin(rz)

# define ATAN2(x,y) (float) atan2 ( (double)x, (double)y )
# define NORM(x,y) sqrt(double(x)*double(x)+double(y)*double(y))

//============================ Get Angles ================================

void gs_angles ( gsEulerOrder order, const GsMat& m, float& rx, float& ry, float& rz, char fmt )
{
	switch ( order )
	{	case 123: gs_angles_xyz(m,rx,ry,rz,fmt); break;
		case 132: gs_angles_xzy(m,rx,ry,rz,fmt); break;
		case 213: gs_angles_yxz(m,rx,ry,rz,fmt); break;
		case 231: gs_angles_yzx(m,rx,ry,rz,fmt); break;
		case 312: gs_angles_zxy(m,rx,ry,rz,fmt); break;
		case 321: gs_angles_zyx(m,rx,ry,rz,fmt); break;
		default: rx=ry=rz=0;
	}
}

void gs_angles_xyz ( const GsMat& m, float& rx, float& ry, float& rz, char fmt )
 {
   if ( fmt=='L' ) { gs_angles_zyx(m,rx,ry,rz,'C'); rx=-rx; ry=-ry; rz=-rz; return; }

   double n = NORM(m[0],m[1]);
   ry = ATAN2 ( -m[2], n );

   if ( !ISZERO(n) )
	{ rx = ATAN2 ( m[6], m[10] );
	  rz = ATAN2 ( m[1], m[0] );
	}
   else
	{ rx = -ATAN2 ( m[9], m[5] );
	  rz = 0;
	}
 }
 
void gs_angles_xzy ( const GsMat& m, float& rx, float& ry, float& rz, char fmt )
 {
   if ( fmt=='L' ) { gs_angles_yzx(m,rx,ry,rz,'C'); rx=-rx; ry=-ry; rz=-rz; return; }

   double n = NORM(m[5],m[9]);
   rz = ATAN2 ( m[1], n );

   if ( !ISZERO(n) )
	{ rx = ATAN2 ( -m[9], m[5] );
	  ry = ATAN2 ( -m[2], m[0] );
	}
   else
	{ rx = 0;
	  ry = ATAN2 ( m[8], m[10] );
	}
 }

void gs_angles_yxz ( const GsMat& m, float& rx, float& ry, float& rz, char fmt )
 {
   if ( fmt=='L' ) { gs_angles_zxy(m,rx,ry,rz,'C'); rx=-rx; ry=-ry; rz=-rz; return; }

   double n = NORM(m[2],m[10]);
   rx = ATAN2 ( m[6], n );

   if ( !ISZERO(n) )
	{ ry = ATAN2 ( -m[2], m[10] );
	  rz = ATAN2 ( -m[4], m[5] );
	}
   else
	{ ry = 0;
	  rz = ATAN2 ( m[1], m[0] );
	}
 }

void gs_angles_yzx ( const GsMat& m, float& rx, float& ry, float& rz, char fmt )
 {
   if ( fmt=='L' ) { gs_angles_xzy(m,rx,ry,rz,'C'); rx=-rx; ry=-ry; rz=-rz; return; }

   double n = NORM(m[0],m[8]);
   rz = ATAN2 ( -m[4], n );

   if ( !ISZERO(n) )
	{ rx = ATAN2 ( m[6], m[5] );
	  ry = ATAN2 ( m[8], m[0] );
	}
   else
	{ rx = 0;
	  ry = - ATAN2 ( m[2], m[10] );
	}
 }

void gs_angles_zxy ( const GsMat& m, float& rx, float& ry, float& rz, char fmt )
 {
   if ( fmt=='L' ) { gs_angles_yxz(m,rx,ry,rz,'C'); rx=-rx; ry=-ry; rz=-rz; return; }

   double n = NORM(m[1],m[5]);
   rx = ATAN2 ( -m[9], n );

   if ( !ISZERO(n) )
	{ rz = ATAN2 ( m[1], m[5] );
	  ry = ATAN2 ( m[8], m[10] );
	}
   else
	{ ry = 0;
	  rz = -ATAN2 ( m[4], m[0] );
	}
 }

void gs_angles_zyx ( const GsMat& m, float& rx, float& ry, float& rz, char fmt )
 {
   if ( fmt=='L' ) { gs_angles_xyz(m,rx,ry,rz,'C'); rx=-rx; ry=-ry; rz=-rz; return; }

   double n = NORM(m[0],m[4]);
   ry = ATAN2 ( m[8], n );

   if ( !ISZERO(n) )
	{ rx = ATAN2 ( -m[9], m[10] );
	  rz = ATAN2 ( -m[4], m[0] );
	}
   else
	{ rz = 0;
	  rx = ATAN2 ( m[6], m[5] );
	}
 }

//============================ Get Mat ================================

void gs_rot ( gsEulerOrder order, GsMat& m, float rx, float ry, float rz, char fmt )
{
	switch ( order )
	{	case 123: gs_rot_xyz(m,rx,ry,rz,fmt); break;
		case 132: gs_rot_xzy(m,rx,ry,rz,fmt); break;
		case 213: gs_rot_yxz(m,rx,ry,rz,fmt); break;
		case 231: gs_rot_yzx(m,rx,ry,rz,fmt); break;
		case 312: gs_rot_zxy(m,rx,ry,rz,fmt); break;
		case 321: gs_rot_zyx(m,rx,ry,rz,fmt); break;
		default: m.identity();
	}
}

void gs_rot_xyz ( GsMat& m, float rx, float ry, float rz, char fmt )
 {
   if ( fmt=='L' ) { gs_rot_zyx(m,-rx,-ry,-rz,'C'); return; }

   GETSINCOS(fmt);
   // the following is the same as: R=Rx*Ry*Rz (in column-major format)
   m[0]=float(cy*cz);		   m[1]=float(cy*sz);		   m[2]=float(-sy);
   m[4]=float(-cx*sz+sx*sy*cz); m[5]=float(cx*cz+sx*sy*sz);  m[6]=float(sx*cy);
   m[8]=float(sx*sz+cx*sy*cz);  m[9]=float(-sx*cz+cx*sy*sz); m[10]=float(cx*cy);
 }

void gs_rot_xzy ( GsMat& m, float rx, float ry, float rz, char fmt )
 {
   if ( fmt=='L' ) { gs_rot_yzx(m,-rx,-ry,-rz,'C'); return; }

   GETSINCOS(fmt);
   // the following is the same as: R=Rx*Rz*Ry (in column-major format)
   m[0]=float(cy*cz);		   m[1]=float(sz);	 m[2]=float(-cz*sy);
   m[4]=float(-cx*sz*cy+sx*sy); m[5]=float(cx*cz);  m[6]=float(cx*sy*sz+sx*cy);
   m[8]=float(sx*sz*cy+cx*sy);  m[9]=float(-sx*cz); m[10]=float(-sx*sy*sz+cx*cy);
 }

void gs_rot_yxz ( GsMat& m, float rx, float ry, float rz, char fmt )
 {
   if ( fmt=='L' ) { gs_rot_zxy(m,-rx,-ry,-rz,'C'); return; }

   GETSINCOS(fmt);
   // the following is the same as: R=Ry*Rx*Rz (in column-major format)
   m[0]=float(cy*cz-sx*sy*sz); m[1]=float(cy*sz+sx*sy*cz); m[2]=float(-cx*sy);
   m[4]=float(-cx*sz);		 m[5]=float(cx*cz);		  m[6]=float(sx);
   m[8]=float(cz*sy+sx*cy*sz); m[9]=float(sy*sz-sx*cy*cz); m[10]=float(cx*cy);
 }

void gs_rot_yzx ( GsMat& m, float rx, float ry, float rz, char fmt )
 {
   if ( fmt=='L' ) { gs_rot_xzy(m,-rx,-ry,-rz,'C'); return; }

   GETSINCOS(fmt);
   // the following is the same as: R=Ry*Rz*Rx (in column-major format)
   m[0]=float(cy*cz); m[1]=float(cx*cy*sz+sx*sy); m[2]=float(cy*sz*sx-cx*sy);
   m[4]=float(-sz);   m[5]=float(cx*cz);		  m[6]=float(cz*sx);
   m[8]=float(sy*cz); m[9]=float(cx*sy*sz-cy*sx); m[10]=float(sx*sy*sz+cx*cy);
 }

void gs_rot_zxy ( GsMat& m, float rx, float ry, float rz, char fmt )
 {
   if ( fmt=='L' ) { gs_rot_yxz(m,-rx,-ry,-rz,'C'); return; }

   GETSINCOS(fmt);
   // the following is the same as: R=Rz*Rx*Ry (in column-major format)
   m[0]=float(cy*cz+sx*sy*sz);  m[1]=float(cx*sz); m[2]=float(-sy*cz+sx*cy*sz);
   m[4]=float(-sz*cy+sx*sy*cz); m[5]=float(cx*cz); m[6]=float(sy*sz+sx*cy*cz);
   m[8]=float(cx*sy);		   m[9]=float(-sx);   m[10]=float(cx*cy);
 }

void gs_rot_zyx ( GsMat& m, float rx, float ry, float rz, char fmt )
 {
   if ( fmt=='L' ) { gs_rot_xyz(m,-rx,-ry,-rz,'C'); return; }

   GETSINCOS(fmt);
   // the following is the same as: R=Rz*Ry*Rx (in column-major format)
   m[0]=float(cy*cz);  m[1]=float(cx*sz+cz*sy*sx); m[2]=float(sx*sz-cx*cz*sy);
   m[4]=float(-sz*cy); m[5]=float(cx*cz-sx*sy*sz); m[6]=float(sx*cz+cx*sy*sz);
   m[8]=float(sy);	 m[9]=float(-sx*cy);		 m[10]=float(cx*cy);
 }

//================================ Angles to Quaternion ================================

void gs_rot ( gsEulerOrder order, GsQuat& q, float a1, float a2, float a3 )
{
	GsQuat Qx ( GsVec::i, a1 );
	GsQuat Qy ( GsVec::j, a2 );
	GsQuat Qz ( GsVec::k, a3 );

	// TodoNote: this file has not been verified after GsMat became line-major
	// -Use test_euler.cpp to test Quat cases

	// As quaternions are represented in mathematical notation
	// the angle order is right-to-left; this has been verified to be correctly
	// the inverse computation of gs_angles(order,q,rx,ry,rz).
	if ( order==gsXYZ )	  q = Qz * Qy * Qx;
	else if ( order==gsXZY ) q = Qy * Qz * Qx;
	else if ( order==gsYXZ ) q = Qz * Qx * Qy;
	else if ( order==gsYZX ) q = Qx * Qz * Qy;
	else if ( order==gsZXY ) q = Qy * Qx * Qz;
						else q = Qx * Qy * Qz;

	// to check if it is equivalent to passing by a transformation matrix:
	//GsMat m;
	//gs_rot ( order, m, a1, a2, a3, 'C' );
	//mat2quat(m,q);
}

//================================ Quaternion to Angles ================================

void gs_angles ( gsEulerOrder order, const GsQuat& q, float& rx, float& ry, float& rz )
{
	GsMat m;
	quat2mat ( q, m );
	gs_angles ( order, m, rx, ry, rz, 'L' );
}

//========================================================================

/*
Rotation matrices have:
=> The "line and row of the rot axis" is 0 or 1 because that coord remains the same
=> If ang is 0, mat becomes id => cos are in diag and sin elsewhere
=> Minus sign...? line major:up,dn,up, column-major:dn,up,dn
Notes:
=> RC(x,y,z) = RCx RCy RCz = RL-x RL-y RL-z = RL(-z,-y,-x)
=> RL(x,y,z) = RLz RLy RLx = RC-z RC-y RC-x = RC(-z,-y,-x)
=> Euler order xyz is around fixed axis, the "inverse order zyx" is moving axis
=> atan2 is a four-quadrant arctangent, such as the C++ library function atan2

Column-major Math:
	  |1   0   0|	|cy  0 -sy|	| cz  sz  0|
   Rx=|0  cx  sx| Ry=| 0  1   0| Rz=|-sz  cz  0|
	  |0 -sx  cx|	|sy  0  cy|	|  0   0  1|

		  |  cy   0  -sy|		 |cycz cysz -sy|		 | cycz sz -czsy|
   RxRy = |sxsy  cx sxcy|  RyRz = | -sz   cz   0|  RzRy = |-szcy cz  sysz|
		  |cxsy -sx cxcy|		 |sycz sysz  cy|		 |   sy  0	cy|

------------------------------------------------------------------

		 |	cycz		 cysz        -sy  |   | m0 m1 m2 |
RxRyRz = |-cxsz+sxsycz  cxcz+sxsysz  sxcy | = | m4 m5 m6 |
		 | sxsz+cxsycz -sxcz+cxsysz  cxcy |   | m8 m9 m10|
X: sx/cx = m6/m10
Z: sz/cz = m1/m0
Y: cy^2*cz^2 + cy^2*sz^2 = m0^2+m1^2 => cy = norm(m0,m1) => sy/cy = -m2/norm(m0,m1)

but if (cy=0) => sy=-+1, consider z=0 => cz=1, sz=0 =>
		 |   0   0  -+1 |   | m0 m1 m2 |
RxRyRz = | -+sx  cx  0  | = | m4 m5 m6 |
		 | -+cx -sx  0  |   | m8 m9 m10|
=> Z:0
   X: sx/cx = m4/m8

------------------------------------------------------------------

		 |	  cycz         sz	   -czsy     |   | m0 m1 m2 |
RxRzRy = | -cxszcy+sxsy   cxcz   cxsysz+sxcy | = | m4 m5 m6 |
		 |  sxszcy+cxsy  -sxcz  -sxsysz+cxcy |   | m8 m9 m10|
X: -sx/cx = m9/m5
Y: -sy/cy = m2/m0
Z: cx^2*cz^2 + cz^2*sx^2 = m5^2+m9^2 => cz = norm(m5,m9) => sz/cz = m1/norm(m5,m9)

but if (cz=0) => sz=-+1, consider x=0 => cx=1, sx=0 =>
RxRzRy = | -+cy  0 -+sy | = | m4 m5 m6 |
		 |   sy  0   cy |   | m8 m9 m10|
=> X:0
   Y: sy/cy = m8/m10

------------------------------------------------------------------

		 |  cycz+sxsysz  cxsz  -sycz+sxcysz |   | m0 m1 m2 |
RzRxRy = | -szcy+sxsycz  cxcz   sysz+sxcycz | = | m4 m5 m6 |
		 |	 cxsy        -sx       cxcy     |   | m8 m9 m10|
Z: sz/cz = m1/m5
Y: sy/cy = m8/m10
X: cx^2*sz^2 + cx^2*cz^2 = m1^2+m5^2 => cx = norm(m1,m5) => sx/cx = -m9/norm(m1,m5)

but if (cx=0) => sx=-+1, consider y=0 => cy=1, sy=0 =>
		 |  cz  0  -+sz |   | m0 m1 m2 |
RzRxRy = | -sz  0  -+cz | = | m4 m5 m6 |
=> Y:0
   Z: -sz/cz = m4/m0

------------------------------------------------------------------

		 |  cycz  cxsz+czsysx sxsz-cxczsy |   | m0 m1 m2 |
RzRyRx = | -szcy  cxcz-sxsysz sxcz+cxsysz | = | m4 m5 m6 |
		 |   sy	     -sxcy	     cxcy     |   | m8 m9 m10|
X: -sx/cx = m9/m10
Z: -sz/cz = m4/m0
Y: cy^2*cz^2 + sz^2*cy^2 = m0^2+m4^2 => cy = norm(m0,m4) => sy/cy = m8/norm(m0,m4)

but if (cy=0) => sy=-+1, consider z=0 => cz=1, sz=0 =>
		 |  0  -+sx -+cx |   | m0 m1 m2 |
RzRyRx = |  0   cx   sx  | = | m4 m5 m6 |
=> Z:0
   X: sx/cx = m6/m5

------------------------------------------------------------------

(the other cases are derived by symmetry)

*/

//================================ EOF ===================================
