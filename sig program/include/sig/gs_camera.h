/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_CAMERA_H
# define GS_CAMERA_H

/** \file gs_camera.h 
 * Manages camera transformations
 */

# include <sig/gs_mat.h>
# include <sig/gs_quat.h>

/*! \class GsCamera gs_camera.h
	\brief Manages projection and camera transformations

	GsCamera encodes parameters defining projection and camera transformations.
	Parameter znear should not be too small, consider 0.1 as minimal value. */
class GsCamera
{  public :
	GsPnt  eye;		//!< position of the center of projection, or the eye, default is (0,0,1)
	GsPnt  center;	//!< position where the eye is looking at, default is (0,0,0)
	GsVec  up;		//!< the up vector orients the camera around the eye-center vector, default is (0,1,0)
	float  fovy;	//!< the y field of view in radians. Default is pi/3 (60deg), range is [0.01,pi]
	float  znear;	//!< must be >0, default is 1.0
	float  zfar;	//!< must be >0, default is 1000
	float  aspect;	//!< normally is set to the screen width/heigh, default is 1

   public :
	
	/*! Initialize the camera with the default parameters, see init(). */
	GsCamera ();

	/*! Copy constructor. */
	GsCamera ( const GsCamera &c );

	/*! Set the parameters to their default values, which are :
		eye=(0,0,2), center=(0,0,0), up=(0,1,0), fovy=60 degs, znear=1, zfar=1000, aspect=1 */
	void init ();

	/*! Returns in c the camera matrix and in p the projection transformation;
		such that the full transformation affecting the scene will be p*c */
	void getmat ( GsMat& p, GsMat& c ) const
		{ c.lookat ( eye, center, up ); p.perspective ( fovy, aspect, znear, zfar ); }

	/*! Returns the full camera matrix m=p*c, where p,c are given by getmat(c,p) */
	void getmat ( GsMat& m ) const;

	/*! Returns the camera matrix */
	void getcmat ( GsMat& c ) const { c.lookat ( eye, center, up ); }

	/*! Returns the projection matrix */
	void getpmat ( GsMat& p ) const { p.perspective ( fovy, aspect, znear, zfar ); }

	/*! Gets the 3d ray (p1,p2) which projects exactly in the given window point
		according to the camera current parameters. Points p1 and p2 will be in
		the near and far planes respectively. Window points are considered to
		be in normalized coordinates, ranging in [-1,1]. */
	void getray ( float winx, float winy, GsPnt& p1, GsPnt& p2 ) const;

	/*! Update the camera parameters to achieve a rotation of dq around the camera
		center. Parameter dq is an incremental rotation in camera coordinates. */
	void rotate ( const GsQuat& dq );

	/*! Translate the camera eye and center by v */
	void translate ( const GsVec& v ) { eye+=v; center+=v; }

	/*! Output camview data values in format keyword1 value \n keyword2 value ...
		(keywords are: eye, center, up, etc*/
	friend GsOutput& operator<< ( GsOutput& out, const GsCamera& c );

	/*! Input camview data. Not all keywords are required to exist. The routine
		returns when a non-keyword entry is found (which is 'ungetted' in inp). */
	friend GsInput& operator>> ( GsInput& inp, GsCamera& c );
};

//================================ End of File =================================================

# endif // GS_CAMERA_H
