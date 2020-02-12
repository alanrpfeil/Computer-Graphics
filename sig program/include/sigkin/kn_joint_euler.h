/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_JOINT_EULER_H
# define KN_JOINT_EULER_H

# include <sig/gs_quat.h>
# include <sigkin/kn_vec_limits.h>

class KnJointRot;

//============================= KnJointEuler ============================

/*! KnJointEuler parameterizes a rotation with Euler angles. 
	Values and limits management follows the interface of KnVecLimits. */
class KnJointEuler : public KnVecLimits
 { private :
	KnJointRot* _jr;
	char _type;
	
   public :

	/*! The currently supported euler types */
	enum Type { TypeXYZ, TypeYXZ, TypeZY, TypeYZX };
	
	/*! Constructor sets type to YXZ and calls init(), 
		which sets all values frozen in 0. */
	KnJointEuler ( KnJointRot* jr );

	/*! Init parameters with same values as in e; only _jr pointer is not chnaged. */
	void init ( const KnJointEuler* e );
	
	/*! Change the current type (non-used rotations are frozen to zero) */
	void type ( Type t );

	/*! Get the current type */
	Type type () const { return (Type)_type; }

	/*! Set values per degree of freedom. In case joint limits are activated,
		values are bounded to the limits before inserted. The associated
		joint local rotation is set to changed only if needed. */
	void value ( int d, float val );
	
	/*! Set all values at once. */
	void value ( float rx, float ry, float rz );

	/*! Init values and set the rotation as changed */
	void init ();

	/*! Simply calls the same method in the base class */
	float value ( int d ) const;
	
	/*! Simply calls the same method in the base class */
	const float* value () const;

	/*! Convert and put in the quaternion the rotation equivalent to the
		current type. For example in type YXZ (the default), the
		rotation will be equal to the rotation matrix R=Ry*Rx*Rz in
		math notation and Rz*Rx*Ry in opengl notation. */
	void get ( GsQuat& q ) const;

	/*! Convert the given quaternion into the correct euler format and set
		the available Euler dofs with the converted values */
	void set ( const GsQuat& q );
};

//==================================== End of File ===========================================

# endif  // KN_JOINT_EULER_H
