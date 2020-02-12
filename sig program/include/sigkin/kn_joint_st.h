/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_JOINT_ST
# define KN_JOINT_ST

# include <sig/gs_quat.h>

class KnJointRot;

/*! Implements the swing and twist decomposition of a rotation, which is
	suitable to define meaningfull swing limits with the use of an ellipse.
	The swing is parameterized in the axis-angle format, in the XY plane.
	The twist is then defined as a rotation around Z axis. */
class KnJointST
{  private :
	float _sx, _sy;	 // swing values X Y in axis-angle format
	float _ex, _ey;	 // ellipse radius to limit swing
	float _twist;	   // twist is a rotation around Z
	float _lolim, _uplim; // twist min/max limits
	gscbool _limit_twist; // if twist limits are active or not
	KnJointRot* _jq;

   public :
	/*! Constructor calls init() */
	KnJointST ( KnJointRot* jq );

	/*! Init swing to (0,0), limits to (PI,PI), and twist frozen to 0. */
	void init ();

	/*! Init parameters with same values as in st; only _jq pointer is not chnaged. */
	void init ( const KnJointST* st );

	/*! Returns true if swing (x,y) is inside the limits, and false otherwise.
		(if (x/ellipsex)^2 + (y/ellipsey)^2 - 1 <=0 then true is returned) */
	bool swing_inlimits ( float x, float y );
 
	/*! Set swing values, which are bounded to the ellipse limits before inserted.
		The local matrix of the associated joint will be set as changed only
		if a different value than the actual one is set.
		Note: (x,y) represents the rotation axis, and ||(x,y)|| is the rotation angle */
	void swing ( float x, float y );

	/*! Change the swing x value and calls swing(x,y) method to enforce limits */
	void swingx ( float x ) { swing(x,_sy); }

	/*! Change the swing y value and calls swing(x,y) method to enforce limits */
	void swingy ( float y ) { swing(_sx,y); }

	/*! Returns the x component of the current swing */
	float swingx ();

	/*! Returns the y component of the current swing */
	float swingy ();

	/*! Change the x/y radius of the limiting ellipse, ensuring swing remains valid.
		Given parameters are automatically bounded into (0,pi] */
	void ellipse ( float rx, float ry );
	
	/*! Get the x radius of the limiting ellipse, which is in (0,pi] */
	float ellipsex () const { return _ex; }

	/*! Get the y radius of the limiting ellipse, which is in (0,pi] */
	float ellipsey () const { return _ey; }

	/*! Set the twist value. In case twist limits are activated, the value is
		bounded to the limits before inserted. Changes only if a new value is set. */
	void twist ( float t );
	
	/*! Returns the current twist value */
	float twist ();

	/*! Set twist limits and ensures twist value remains valid if limits are active */
	void twist_limits ( float min, float max );

	/*! Returns the min twist limit value */
	float twist_lolim () const { return _lolim; }

	/*! Returns the min twist limit value */
	float twist_uplim () const { return _uplim; }

	/*! Activates or deactivates twist limits */
	void twist_limits ( bool b ) { _limit_twist = b? 1:0; }

	/*! Returns the activation state of twist limits */
	bool twist_limits () { return _limit_twist==1; }

	/*! Returns true if the given value respects current twist limits and false otherwise */
	bool twist_inlimits ( float t ) const
	 { return _lolim<=t && t<=_uplim; }

	/*! Activates limits and set min and max equal to the current twist value */
	void twist_freeze ();
	
	/*! Returns true if the twist value is frozen, i.e if it cannot receive new values.
		If the minimum limit is equal to the maximum limit, and limits are activated,
		then the twist is considered frozen. */
	bool twist_frozen () const;
	
	/*! Returns random swing values, limited by the current ellipse radius */
	void get_random_swing ( float& x, float& y ) const;

	/*! Returns random twist values, limited if twist limits are activated */
	float get_random_twist () const;

	/*! Convert and put in the given quaternion the rotation equal to: R=Rtwist*Rswing */
	void get ( GsQuat& q ) const;

	/*! Convert the given quaternion into the correct swing-twist values and set
		this KnJointST with the converted values */
	void set ( const GsQuat& q );
};

//==================================== End of File ===========================================

# endif  // KN_JOINT_ST
