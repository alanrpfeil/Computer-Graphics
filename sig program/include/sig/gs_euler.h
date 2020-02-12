/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
/** \file gs_euler.h 
 * euler angles tools */

# ifndef GS_EULER_H
# define GS_EULER_H

# include <sig/gs_mat.h>

//================================ Matrix to Angles ================================

/*! Enumerator to specify possible Euler angle combinations. */
enum gsEulerOrder { gsXYZ=123, gsXZY=132, gsYXZ=213, gsYZX=231, gsZXY=312, gsZYX=321 };

/*! Extract from the rotation matrix m the x,y,z Euler angles, 
	assuming they were encoded in m in the given order and format.
	Parameter order should be: 123 for XYZ, etc, according to gsEulerOrder enum.
	The other gs_angles* functions are similar, but specific to the desired order.
	Format fmt can be either 'C' or 'L' for column or line major representations.
	For example, following the assumed line-major format of GsMat, euler angles
	in gsXYZ order are to be multiplied in matrix form as Mz*My*Mx. */
void gs_angles ( gsEulerOrder order, const GsMat& m, float& rx, float& ry, float& rz, char fmt );

void gs_angles_xyz ( const GsMat& m, float& rx, float& ry, float& rz, char fmt );
void gs_angles_xzy ( const GsMat& m, float& rx, float& ry, float& rz, char fmt );
void gs_angles_yxz ( const GsMat& m, float& rx, float& ry, float& rz, char fmt );
void gs_angles_yzx ( const GsMat& m, float& rx, float& ry, float& rz, char fmt );
void gs_angles_zxy ( const GsMat& m, float& rx, float& ry, float& rz, char fmt );
void gs_angles_zyx ( const GsMat& m, float& rx, float& ry, float& rz, char fmt );

//================================ Angles to Matrix ================================

/*! Set the rotation matrix m according to the given x,y,z Euler angles and
	the given desired order and format.
	Parameter order should be: 123 for XYZ, etc, according to gsEulerOrder enum.
	For example, XYZ order in 'C' format is equivalent to Rx*Ry*Rz in 'C' format;
	however XYZ order in 'L' format is equivalent to Rz*Ry*Rx in 'L' format.
	See comments in the gs_mat.h header file for further explanations.
	Note: the 4th line and column of matrix m are not changed.
	The other gs_rot* functions are similar, but specific to the desired order. */
void gs_rot ( gsEulerOrder order, GsMat& m, float rx, float ry, float rz, char fmt='L' );

void gs_rot_xyz ( GsMat& m, float rx, float ry, float rz, char fmt );
void gs_rot_xzy ( GsMat& m, float rx, float ry, float rz, char fmt );
void gs_rot_yxz ( GsMat& m, float rx, float ry, float rz, char fmt );
void gs_rot_yzx ( GsMat& m, float rx, float ry, float rz, char fmt );
void gs_rot_zxy ( GsMat& m, float rx, float ry, float rz, char fmt );
void gs_rot_zyx ( GsMat& m, float rx, float ry, float rz, char fmt );

//================================ Angles to Quaternion ================================

class GsQuat;

/*! Set the quaternion q according to the given Euler angles and order.
	If orderfixed is true (default), angles a1, a2, and a3 are around X, Y and Z axes
	independent of the order encoded in parameter order. If orderfixed is false,
	angles a1, a2, a3 are around the axes specified in the given order. */
void gs_rot ( gsEulerOrder order, GsQuat& q, float a1, float a2, float a3 );

//================================ Quaternion to Angles ================================

/*! Converts the quaternion to Euler angles. The convertion is not direct but passing
	by a rotaiton matrix. Angles rx, ry, and rz are around X, Y and Z axesindependent
	of the order encoded in parameter order */
void gs_angles ( gsEulerOrder order, const GsQuat& q, float& rx, float& ry, float& rz );

//============================== end of file ===============================

# endif // GS_EULER_H
