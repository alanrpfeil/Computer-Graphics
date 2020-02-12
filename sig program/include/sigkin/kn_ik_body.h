/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_IK_BODY_H
# define KN_IK_BODY_H

# include <sig/gs_shareable.h>
# include <sigkin/kn_coldet.h>
# include <sigkin/kn_posture.h>
# include <sigkin/kn_ik.h>

class SnGroup;
class SnLines;

//========================= KnIkBody ================================

/*! Contains all data needed for computing the Inverse Kinematics of
	the four limbs simultaneously. All classes are exchanged honoring the
	calls to ref()/unref() for efficient sharing and change of ownership */
class KnIkBody : public GsShareable
{  private :
	KnSkeleton* _skeleton;  // associated skeleton
	KnColdet*   _coldet;	// collision detector
	KnPosture	_posture;	// to save last posture in case IK failed
	KnIk*		_ik[4];		// larm, rarm, lleg, rleg (some may be null)
	KnIk::Result _result;	// last ik result
	GsMat		_locgoal;	// last local goal matrix
	SnLines*	_lines[4];	// lines for each IK

	KnIkOrbitSearch _osearch[4]; // parameters for the auto orbit search mode
	gscbool _osearch_active[4]; // if not active, oserach.init is used as fixed angle
	double _solvetime;		// if >=0, solve timer is activated

	GsMat _lfoot, _rfoot;	// global feet matrices
	KnJoint* _posroot;		// best root joint detected in capture_lower_body()
	GsVec _initpos;			// initial position of the detected root

   public :
	/*! Initializes the class with default parameters and orbit and style search
		set as non-usable class, init() must be properly called afterwards */
	KnIkBody ();

	/*! Destructor is public but be sure to respect ref()/unref() use */
   ~KnIkBody ();

	/*! Initializes with given end effectors: lwrist, rwrist, lankle, rankle.
		Null can be given to those not used.
		The corresponding KnIk will be null in case of an initialization error.
		Returns the number of IKs created.  */
	int init ( KnJoint* endef[4] );

	/*! Initializes with the names of the end effector joints to be used in a
		single string, in the following order: lwrist, rwrist, lankle, rankle.
		The corresponding KnIk will be null in case of an initialization error.
		Returns the number of IKs created.  */
	int init ( KnSkeleton* kn, const char* endefs );

	/*! Initializes with given IKs of types: larm, rarm, lleg, rleg.
		They must be properly initialized. Null can be given to those not used.
		Returns the number of IKs created. */
	int init ( KnIk* ik[4] );

	/*! Returns ik of given index: 0:larm, 1:rarm, 2:lleg, 3:rleg. Can be null. */
	KnIk* ik ( int i ) { return _ik[i]; }

	/*! Returns a reference to the orbit search parameters of ik index i */
	KnIkOrbitSearch& osearch ( int i ) { return _osearch[i]; }

	/* Calls solve_closest() to the ik of given index, or to all Iks if i<0 (default) */
	void solve_closest ( bool b, int i=-1 );

	/*! If orbit search is not activated, osearch(i).init is used as the
		requested, fixed, orbit angle */
	void osearch_activation ( int i, bool b ) { _osearch_active[i]=b?1:0; }
	bool osearch_activation ( int i );

	/*! Declares the coldet to be considered, which must be connected to the used skeleton.
		Null can be set in case no collision detection is to be used */
	void coldet ( KnColdet* cd );

	/*! Returns used coldet. Can be null if no collisions are checked. */
	KnColdet* coldet () { return _coldet; }

	/*! For each valid ik, set limits of the base, mid, end joints to the maximum range */
	void remove_joint_limits ();

	/*! Stores current feet global position, legs orbit angles and root position */
	void capture_init_posture ();

	/*! Returns the global matrix of the left foot when captured as initial posture */
	GsMat& lfoot () { return _lfoot; }

	/*! Returns the global matrix of the right foot when captured as initial posture */
	GsMat& rfoot () { return _rfoot; }

	/*! Returns the "translation root joint" detected during capture_init_posture().
		This will be the real root of the skeleton, or the root child if a single
		child is attached to the root and has all translations channels active.
		Null is returned in case capture_init_posture() was not called. */
	KnJoint* posroot() { return _posroot; }

	/*! Solve i-th ik, if available, according to the settings. */
	KnIk::Result solve ( int i, const GsMat& globgoal );

	/*! Call solve for each of the captured lfoot and rfoot positions */
	void fixfeet () { solve(2,_lfoot); solve(3,_rfoot); }

	/*! Returns the result of the last call to solveik */
	KnIk::Result result () const { return _result; }

	/*! Returns the local goal used during the last call to solveik */
	const GsMat& locgoal () const { return _locgoal; }
	
	/*! Adds up to four SnLines as children of given group g, also saving
		them in KnIkBody. Returns the number of SnLines inserted. */
	int add_lines ( SnGroup* g );

	/*! Returns the SnLines index i, can be null */
	SnLines* lines ( int i ) { return _lines[i]; }

	/*! Attach or detach SnLines to the IK index i */
	void lines ( int i, bool attach );

	/*! Set time measurement for each solve call on or off */
	void timer_activation ( bool on ) { if (on) _solvetime=0; else _solvetime=-1; }

	/*! Get the time (in secs) of the last call to a solve method.
		-1 is returned if the timer is not activated */
	double lsolvetime () { return _solvetime; }

   private:
	void _initskel ();
	void _unrefik ();
};

//============================= EOF ===================================

# endif // KN_IK_BODY_H
