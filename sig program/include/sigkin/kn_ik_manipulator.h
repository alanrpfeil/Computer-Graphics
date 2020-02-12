/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_IK_MANIPULATOR_H
# define KN_IK_MANIPULATOR_H

# include <sig/sn_lines.h>
# include <sig/sn_shape.h>
# include <sig/sn_group.h>
# include <sigkin/kn_ik_body.h>

//=================================== KnIkManipulator =====================================

class SnManipulator;
class SnTransform;
class KnScene;
class KnIk;

class KnIkManipulator : public SnGroup
{  public :
	typedef void (*CallBack)( KnIkManipulator* ikm, void* udata );
	enum SolveMethod { FixedOrbit, SearchOrbit, IkBody, UserCallback };
	static const char* class_name;

   protected :
	KnIk* _ik;
	KnIk::Result _res;
	KnIkBody* _ikbody;
	int _ikbodyid;
	KnIkOrbitSearch _iksearch;
	KnScene* _knscene;
	KnColdet* _coldet;
	CallBack _cbfunc, _postcbfunc;
	void* _udata;
	void* _postudata;
	SolveMethod _method;
	SnManipulator* _manip;
	SnLines* _lines;
	SnTransform* _matrix;

   protected :
	/*! Destructor only accessible through unref() */
   ~KnIkManipulator ();

   public :
	/*! Constructor */
	KnIkManipulator ();

	/*! Get and ref the given ik, attaching the manipulator to it.
		The ik pointer must be already properly initialized.
		The goalmodel, if given, is loaded in the goal position frame,
		otherwise the model attached to the end joint, if any, is used.
		See method callback() for specifying a user callback function, and
		method solve_method() for setting an automatic solve behavior.
		By default, the behavior is set to use a user callback.
		Returns true if initialization was succesfull and false otherwise. */
	bool init ( KnIk* ik, GsModel* goalmodel=0 );

	/*! Uses the given ikbody for solving the IK of the i-th ik only. 
		For example the default argument of 1 is the right wrist.
		KnIkBody must be properly initialized.
		Method solve_method() must be called afterwards. */
	bool init ( KnIkBody* ikbody, int i=1, GsModel* goalmodel=0 );

	/*! Specifies the desired solve method to be automatically performed
		by the manipulator:
		FixedOrbit: the desired orbit angle must be set in iksearch().oangle.
		SearchOrbit: search parameters are also set with iksearch().
		IkBody: the attached KnIkBody is used; only accepted if init(KnIkBody*) was used.
		UserCallback: the IK is not solved, and the user callback, if any, is called.
		A KnScene is required for updating the character's scene, and a coldet can
		be given for including collision detection. KnScene and KnColdet are referenced.
		Note1: With the IkBody method, coldet pointer is not used as it comes with KnBodyIk.
		Note2: If knscene is null, the mode becomes UserCallback */
	void solve_method ( SolveMethod sm, KnScene* knscene, KnColdet* coldet=0 );

	/*! Returns the result of the most recent call to the IK */
	KnIk::Result result () const { return _res; }

	/*! Access to the orbit angle search parameters; see solve_method() */
	KnIkOrbitSearch& iksearch () { return _iksearch; }
	
	/*! Set a user-defined callback to be called whenever the manipulator moves,
		completly replacing any defined solve method (the solve method becomes UserCallback)
		The user callback will then be responsible to do the job of: 
		calling ik()->solve() for current goal, updating the skeleton values,
		and drawing the new skeleton pose. */
	void callback ( CallBack cb, void* udata );
	
	/*! Set a callback to be called after each call to iksolve, ie, each time the manip moves */
	void post_callback ( CallBack cb, void* udata );
	
	/*! Returns the previously set user data pointer */
	void* udata () { return _udata; }

	/*! Set the manipulator to match the current configuration of the skeleton.
		Global matrices of the skeleton will be automatically updated. */
	void match ();

	/*! will turn on or off the drawing of the IK lines */
	void lines ( bool b );

	/*! Access to the used IkBody object (or null if not used) */
	KnIkBody* ikbody() const { return _ikbody; }
	int ikbodyid() const { return _ikbodyid; }

	/*! Access to the used IK object */
	KnIk* ik() { return _ik; }
	
	/*! Access to the SnManipulator object */
	SnManipulator* manip() { return _manip; }

	/*! Update axis position and call iksolve() as if the goal was moved */
	KnIk::Result update ();
	
	/*! Returns the current goal matrix in the manipulator */
	const GsMat& cmat () const;

	/*! This one can only be called after initialization!  */
	GsMat& mat ();

	/*! This method either calls the associated callback, if any, or proceed
		and solve the IK according to the specified solve method.
		As it is virtual, it can also be rewritten for custom behavior. */
	virtual KnIk::Result iksolve ();
};

/*! Adds up to four ik manipulators to given group g (each with label "IkManip").
	Returns the number of manipulators returned. */
int add_ik_manipulators ( SnGroup* g, KnIkBody* ikb, KnScene* knscene,
						  KnIkManipulator::CallBack postcb=0, void* udata=0, KnColdet* coldet=0 );

/*! For each IkManipulator child of g, calls its match() method. Return number of manips found. */
int match_ik_manipulators ( SnGroup* g );

/*! For each IkManipulator child of g, calls its update() method. Return number of manips found. */
int update_ik_manipulators ( SnGroup* g );

/*! Find the IkManipulator child of g which controls an ikbody with index id.
	Returns null if not found. */
KnIkManipulator* get_ik_manipulator ( SnGroup* g, int id );

//======================================= EOF =====================================

# endif // KN_IK_MANIPULATOR_H
