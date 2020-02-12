/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_TRACKBALL_H
# define GS_TRACKBALL_H

/** \file gs_trackball.h 
 * trackball manipulation
 */

# include <sig/gs_vec.h>
# include <sig/gs_quat.h>

class GsMat;

/*! \class GsTrackball gs_trackball.h
	\brief trackball manipulation

	GsTrackball maintains a rotation with methods implementing a
	trackball-like manipulation interface. */
class GsTrackball
{  public :
	GsQuat rotation; //!< current rotation
	GsQuat lastspin;

   public :
	
	/*! Initialize the trackball with the default parameters, see init(). */
	GsTrackball ();

	/*! Copy constructor. */
	GsTrackball ( const GsTrackball& t );

	/*! Set the parameters to their default values, which is a null rotation. */
	void init ();

	/*! Gets the rotation induced by a mouse displacement,
		according to the trackball metaphor. Window coordinates must be
		normalized in [-1,1]x[-1,1]. */
	static void get_spin_from_mouse_motion ( float lwinx, float lwiny, float winx, float winy, GsQuat& spin );

	/*! Accumulates the rotation induced by a mouse displacement,
		according to the trackball metaphor. Window coordinates must be
		normalized in [-1,1]x[-1,1]. */
	void increment_from_mouse_motion ( float lwinx, float lwiny, float winx, float winy );

	/*! Accumulates the rotation with the given quaternion (left-multiplied). */
	void increment_rotation ( const GsQuat& spin );

	/*! Output of trackball data for inspection. */
	friend GsOutput& operator<< ( GsOutput& out, const GsTrackball& tb );
};

//================================ End of File =================================================

# endif // GS_TRACKBALL_H

