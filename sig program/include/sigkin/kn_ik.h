/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_IK_H
# define KN_IK_H

# include <sig/gs_mat.h>
# include <sig/gs_shareable.h>
# include <sig/sn_shape.h>

class GsQuat;
class SnLines;
class KnJoint;
class KnColdet;
class KnSkeleton;
struct KnIkOrbitSearch; // defined in the end of this header file

//======================================= KnIk =====================================

/*! Provides the analytical IK solver for arms and legs, given the desired orbit angle.
	In addition, a solver with automatic orbit angle search is included.
	The final twist can be either applied before or after the end joint swing, according
	to the given parameters during initialization (see methods init()).
	Swing-twist axis convention, considering the character in a standing position, with
	arms completely outstreched to the sides:
	Arms: when looking from each hand to the shoulder, with the character's head up:
		  shoulder's X axis points to the right, and Y axis to the top. The thumb is up.
	Legs: when looking from the feet to the head, with feet toes pointing up:
		  hip's X axis points to the bottom, and Y axis to the right. 
	Note: when the swing-twist end joint has the "mid twist", ie the twist comes after
	the swing, the ellipse defining the swing limits is not twisted with the end joint,
	and this is a problem as the limits will become inversed.
	A simple "solution" is to use equal limits in X and Y, ie, use a circle to define
	swing limits. The same problem occurs with the twisting.
	As this configuration is not in use, a better solution for that is still missing.
	Note 2: **Arms and Legs offsets MUST be along one single axis*** */
class KnIk : public GsShareable
{  public :
	enum Type { LeftArm, RightArm, LeftLeg, RightLeg }; // enum order must not change
	enum RotAxis { X=0, Y=1, Z=2 };

   private :
	gscenum _type;				// linkage type
	gscbool _solve_rot_goal;	// if true (default) will compute/test the 6DOF goal
	gscbool _solve_closest;		// if true will compute closest solution (goal aim); default is false
	KnJoint *_base, *_mid, *_end; // skeleton joints
	RotAxis _midflexaxis;		// euler angle axis to put the mid flexion
	KnJoint* _midtwist;		    // if given, the twist is computed before the hand joint
	RotAxis  _midtwistaxis;		// euler angle axis to put the mid twist, if _midtwist is given
	SnLines* _snlines;			// to draw things if required
	float _result[7];			// stores found results
	float _d1, _d2;				// computed lengths of the upper arm and lower arm
	void _init ();				// private init method

   public :
	/*! Default constructor initializes the object, but it will only be ready for use
		after a succesfull call to a init() method */
	KnIk ();
   
	/*! Constructor which initializes the solver for a given type and end-effector. 
		The parameters are considered exactly as in the init(t,end) method */
	KnIk ( Type t, KnJoint* end );

	/*! Destructor */
	virtual ~KnIk ();

	/*! Give a SnLines node to let the IK draw the elbow orbit circle
		and the base-end lines each time solve() is called. 
		KnIk will then maintain a reference to sl (calling sl->ref()).
		Set this pointer to null to stop drawing. */
	void lines ( SnLines* sl );
	
	/*! Access to the scene node used for drawing lines; can be null. */
	SnLines* lines() { return _snlines; }

	Type type () const { return (Type)_type; } //<! Returns the current type
	KnJoint* base() { return _base; } //<! Retrieves base joint (usually shoulder or hip)
	KnJoint* mid() { return _mid; }   //<! Retrieves mid joint (usually elbow or knee)
	KnJoint* end() { return _end; }   //<! Retrieves end joint (usually hand or foot)

	/*! Retrieves the "mid-twist" joint. The meaning of this joint will vary according
		to how KnIk was initialized: if it is null, there is no mid twist joint and the
		final twist is considered to be at the swing-twist end joint.
		Otherwise it will be the euler joint used to set the mid twist, and it may or
		may not be the same as the elbow joint. */
	KnJoint* midtwist() { return _midtwist; }

	/*! Returns the euler axis of the mid joint used for the mid flexion rotation */
	RotAxis midflexaxis() const { return _midflexaxis; }

	/*! Returns the euler axis of the midtwist joint used for the mid twist rotation.
		The returned value will only have a meaning if a midtwist joint is being used. */
	RotAxis midtwistaxis() const { return _midtwistaxis; }
	
	/*! Init the IK according to the given parameters.
		Two types of linkages are handled:
		1. When the midtwist joint is given as null (the default):
		   Joints (base,mid,end) must have rotation types: (Swing-Twist,Euler,Swing-Twist)
		   In this case there is a twist rotation applied to the end joint, after its swing.
		2. When the midtwist joint is given:
		   Joints (base,mid,midtwist,end) must have rotation types:(Swing-Twist,Euler,Euler,Swing).
		   In this case the final twist is applied before the end joint (midtwist can be =mid).
		In all cases, midflexaxis indicates which axis corresponds to the flexion to be applied
		to the mid joint, similarly midtwistaxis is used for midtwist (when given).
		If success, true is returned. Otherwise the initalization fails and false is returned. */
	bool init ( Type t, KnJoint* base, KnJoint* mid, RotAxis midflexaxis, KnJoint* end,
				KnJoint* midtwist=0, RotAxis midtwistaxis=Z );
	
	/*! Same as the "main init() method", but receiving joint names instead of pointers. */
	bool init ( Type t, KnSkeleton* kn, const char* base, const char* mid, RotAxis midflexaxis,
				const char* end, const char* midtwist=0, RotAxis midtwistaxis=Z );

	/*! A smarter init() that determines all parameters by analyzing the linkage from the
		given end joint (typically the wrist or foot). The linkage joints must have properly
		defined rotation parameterizations, limits and activation for allowing a unique
		determination of the rotation axis to be used. For example, when both the flexion
		and twist are in a same mid joint, the flexion axis is determined by the axis 
		having one limit equal to 0. */
	bool init ( Type t, KnJoint* end );

	/*! Set limits of the base, mid, end joints to the maximum range [-pi,pi] */
	void remove_joint_limits ();

	/*! If the skeleton is not globally uptodate, updates the global matrices of the
		joints prior to the base joint by calling _base->parent()->update_gmat_up() */
	void update_base_up ();
	
	/*! Calls update_gmat_local for the joints inbetween the base and end joints */
	void update_base_end ();

	/*! Returns the global matrix of the base joint, incorporating its prerot(),
		but without its local swing/twist rotation.
		The result is the same as _base->gmat(), after calling _base->init_rot(),
		but we dont change any joint values here.
		Global matrices are required to be up to date, see update_base_up(). */
	void base_frame ( GsMat& bframe );

	/*! Transforms the goal matrix in global coordinates to local coordinates with:
		local = goal * base_frame().inverse(); */
	void set_local ( GsMat& goal );

	/*! Returns the length of the base-end linkage in maximum extension */
	float linkage_len ();

	/*! If true (default) the 6DOF pos/orientation are considered as goal in
		solve() methods. If set to false only the position is solved */
	void solve_rot_goal ( bool b ) { _solve_rot_goal=b; }

	/*! Returns true if the rotational goal constraint is being solved */
	gscbool solve_rot_goal () const { return _solve_rot_goal; }

	/*! Changes the state of the aiming/solve closest mode. Default is false */
	void solve_closest ( bool b ) { _solve_closest=b; }

	/*! Returns state of aiming/solve closest mode */
	gscbool solve_closest () const { return _solve_closest; }

	/*! Result enumerator used in the solve() methods */
	enum Result { Ok, NotReachable, NoMidFlexion, NoBaseSwing, 
				  NoBaseTwist, NoMidTwist, NoEndSwing, Collision, Undef };

	/*! Solve with **goal matrix in local coordinates** and given orbit angle.
		By default both pos and orientation are solved, but solve_rot_goal() can change this.
		For converting global coordinates in local coordinates, use set_local().
		Collisions are not tested here and the results are not applied to the skeleton.
		Use apply_last_result() to apply the values to the skeleton.
		Make sure the global matrices are up to date. */
	Result solve ( const GsMat& goal, float oang );
	
	/*! If coldet is null, this method is the same as the prior solve() method,
		except that **this method always call apply_last_result() in case of success**.
		When a valid coldet is given, this method also tests for collisions:
		when the IK can be solved respecting joint limits, the values are
		applied to the skeleton, the global matrices of the skeleton are updated,
		and a collision query is called, maybe leading to a Collision result. */
	Result solve ( const GsMat& goal, float oang, KnColdet* coldet );

	/*! This method will automatically search for an orbit angle respecting no
		collisions (if coldet!=0) and joint limits. The search is performed 
		according to the parameters in the OrbitSearch structure.
		Notes: - skeleton values are always changed during the search
			   - coldet can be a null pointer
			   - KnIkOrbitSearch structure is defined in the end of this header
			   - All solve methods assume **goal matrix in local coordinates** */
	Result solve ( const GsMat& goal, KnIkOrbitSearch& osearch, KnColdet* coldet=0 );

	/*! Computes the orbit angle existing in the current posture of the skeleton.
		Make sure the global matrices are up to date before calling this method. */
	float orbit_angle ();

	/*! Get a pointer to an array of 7 floats containing the 7 DOF values computed
		during the last call to solve(). The 7 values can come in two formats,
		according to how KnIk was initialized:
		1. Base Swing/Twist, Mid Euler flexion, Mid Euler Twist, End Swing
		2. Base Swing/Twist, Mid Euler flexion, End Swing/Twist
		PS: the values can be edited, for instance, to set the desired end joint
		orientation when only the position is being solved. */
	float* last_result () { return _result; }

	/*! Apply to the skeleton joints the values found during the last call to solve() */
	void apply_last_result () { apply_values(_result); }

	/*! Get the 7 values from the current skeleton joints and put in the user
		provided float array of 7 values */
	void get_sk_values ( float values[7] );

	/*! Put in the skeleton joints the 7 given values */
	void apply_values ( const float values[7] );

	/*! Returns a string message describing the result parameter */
	static const char* message ( Result res );

   /*! Configures local parameterizations and pre/post frames as expected by the IK
	   solver. It assumes the linkage has three joints (base, mid, end) and will
	   configure them as swing-twist, euler, and swing-twist. The given joint e
	   is the end joint, and the array of lower/upper limits is in the following order: 
	   base swing, base twist, mid Y, mid Z, end swing. 
	   Returns false if the linkage does not have 3 joints, true otherwise. */
	static bool configure_skeleton ( KnJoint* e, const GsQuat& pre, const GsQuat& post, float l[10] );

	/*! Configures local parameterizations and pre/post frames as expected by the IK
		solver with the typical recommended settings for the given linkage type. */
	static bool configure_skeleton ( KnJoint* e, Type t );
};

/*! This structure provides the parameters used for the automatic orbit angle search
	in method KnIk::solve ( const GsMat&, KnIkOrbitSearch&, KnColdet* )
	Notes: - all angles must be specified in radians.
		   - the last two parameters (oangle and iterations) are set by solve() */
struct KnIkOrbitSearch
{	float init;		//<! the initial orbit angle taken as starting point for the search
	float inc;		//<! the incremental step taken during each iteration
	float rate;		//<! to augment inc at each iteration for a greedy behavior
	float min;		//<! the lower limit acceptable for the orbit angle
	float max;		//<! the upper limit acceptable for the orbit angle
	float oangle;	//<! stores the last orbit angle tested in the last call to solve(), will be the solution after a successful call
	int iterations;	//<! stores the number of iterations performed in the last call to solve()

	/*! Constructor initilizes with default parameters of the given type or,
		if a type is not given, for the right arm */
	KnIkOrbitSearch ( KnIk::Type t=KnIk::RightArm ) { set_defaults(t); oangle=0; iterations=0; }

	/*! Defaults are (in degrees): 32, 2, 0.1; min/max are -15,130 for arms, and
		-30,90 for the legs. */
	void set_defaults ( KnIk::Type t );
};

//======================================= EOF =====================================

# endif // KN_IK_H
