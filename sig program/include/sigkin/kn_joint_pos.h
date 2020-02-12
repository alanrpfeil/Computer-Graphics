/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_JOINT_POS_H
# define KN_JOINT_POS_H

# include <sigkin/kn_vec_limits.h>

class KnJoint;

/*! KnJointPos controls values and min/max limits for three (x,y,z) translations. */
class KnJointPos : public KnVecLimits
 { private :
	KnJoint* _joint; // to notifiy the joint that the current values changed
	friend class KnJoint;  // used only to fix a constructor-related warning
	
   public :
   
	/* Constructor calls init(), which set all values frozen in 0. */
	KnJointPos ( KnJoint* j ) { _joint=j; }

	/*! Set values per degree of freedom. In case joint limits are activated,
		values are bounded to the limits before inserted. The associated
		joint local matrix is set to changed if needed. */
	void value ( int d, float val );

	/*! Set all values at once. */
	void value ( float x, float y, float z );
	
	/*! Set all values at once, from a GsVec v. */
	void value ( const GsVec& v ) { value(v.x,v.y,v.z); }

	/*! Simply calls the same method in the base class */
	float value ( int d ) const { return KnVecLimits::value(d); }
	
	/*! Simply calls value(0) */
	float valuex () const { return KnVecLimits::value(0); }

	/*! Simply calls value(1) */
	float valuey () const { return KnVecLimits::value(1); }

	/*! Simply calls value(2) */
	float valuez () const { return KnVecLimits::value(2); }

	/*! Simply calls the same method in the base class */
	const float* valuept () const { return KnVecLimits::valuept(); }

	/*! Simply calls the same method in the base class */
	GsVec value () const { return KnVecLimits::value(); }
};

//==================================== End of File ===========================================

# endif  // KN_JOINT_POS_H
