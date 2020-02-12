/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_JOINT_H
# define KN_JOINT_H

# include <sig/gs_mat.h>
# include <sig/gs_quat.h>
# include <sig/gs_array.h>

# include <sigkin/kn_ik_solver.h>
# include <sigkin/kn_joint_pos.h>
# include <sigkin/kn_joint_rot.h>
# include <sigkin/kn_joint_name.h>

class GsModel;

/*! KnJoint defines one joint and controls its translations and
	rotations. The translation is always set by using the KnJointPos
	object returned by method pos(), while the rotation can be set with
	different types of parameterization and joint limit control via the
	KnJointRot object returned by method rot() (methods euler() and st()
	are shortcuts to the methods with same name in KnJointRot). */
class KnJoint
{  public :
	/*! RotType specifies the specified default joint rotation parameterization */
	enum RotType { TypeQuat,  // Quaternion without limits, fastest convertion to matrix
				   TypeST,	  // Swing-twist: axis-angle (x,y) swing, z rotation twist, and ellipse limits
				   TypeEuler, // Euler angles with min/max limits per dof
				   TypeUndef  // Type not defined, no rotation channel should exist, but values can still be set
				 };

   private :
	GsModel* _visgeo;		// the attached geometry to visualize this joint
	GsModel* _colgeo;		// the attached geometry used for collision detection
	KnJoint* _parent;		// the parent joint
	GsArray<KnJoint*> _children; // the children joints
	GsMat _gmat;			// global matrix: from the root to the children of this joint
	GsMat _lmat;			// local matrix: from this joint to its children
	gscbool _lmattodate;	// true if lmat is up to date
	gscenum _rtype;			// one of the RotType enumerator
	KnJointName _name;		// the given name
	int   _index;			// its index in KnSkeleton::_joints
	int   _coldetid;		// index used in collision detection
	KnSkeleton* _skeleton;	// pointer to the associated skeleton
	GsVec _offset;			// offset from the parent joint to this joint
	KnJointPos _pos;		// controls the translation parameterization
	KnJointRot _rot;		// access to the rotation in any parameterization
	KnIkSolver* _ik;		// a generic ik solver if this joint has one, null otherwise

	friend class KnSkeleton;
	friend class KnColdet;

   public :
	void* udata; // initialized as null and then completely up to user maintainance

   private:
	// constructor sets all dofs to non active, and rotation type as euler
	KnJoint ( KnSkeleton* kn, KnJoint* parent, RotType rtype, int i );
   ~KnJoint ();

   public :

	/*! Init this joint with the same parameters as given joint j.
		The considered parameters copied are:
		_rtype, _name, _offset, _pos, and _rot. */
	void init ( const KnJoint* j );

	/*! Get a pointer for the attached geometry for visualization,
		or null if no such geometry was loaded. The geometry pointers
		can be shared ( see GsModel::ref/unref). */
	GsModel* visgeo () const { return _visgeo; }

	/*! Get a pointer for the attached geometry for collision detection,
		or null if no such geometry was loaded. The geometry pointers
		can be shared ( see GsModel::ref/unref). */
	GsModel* colgeo () const { return _colgeo; }

	/*! Replaces the visualization geometry pointer with the new one.
		The ref/unref methods are used during replacement and null can be
		passed in order to only unref the visualization geometry */
	void visgeo ( GsModel* m ) { updrefs(_visgeo,m); }

	/*! Replaces the collision geometry pointer with the new one.
		The ref/unref methods are used during replacement and null can be
		passed in order to only unref the collision geometry */
	void colgeo ( GsModel* m ) { updrefs(_colgeo,m); }

	/*! Get a pointer to the skeleton owner of this joint */
	KnSkeleton* skeleton () { return _skeleton; }

	/*! traverse hierarchy */
	KnJoint* parent() const { return _parent; }
	KnJoint* child ( int i ) const { return _children[i]; }
	int children () const { return _children.size(); }

	/*! Set the name of this joint */
	void name ( KnJointName jn ) { _name=jn; }
	KnJointName name () const { return _name; }

	/*! Returns the index of this joint in the KnSkeleton list
		of joints */
	int index () const { return _index; }

	/*! Returns the collision detection id of the colgeo attached
		to this joint (or -1 if no id). */
	int coldetid () const { return _coldetid; }

	/*! returns the fixed frame translation relative to the parent */
	const GsVec& offset () const { return _offset; }
 
	/*! change the fixed frame translation relative to the parent */
	void offset ( const GsVec& o );

	/*! Access the translation parameterization of the joint. */
	KnJointPos* pos () { return &_pos; }
	const KnJointPos* cpos () const { return &_pos; }

	/*! Set the desired rotation model. The rotation model is important for
		defining how the rotation of the joint should be represented in 
		channels, what will determine the values that are stored in postures
		and motions. The default value is TypeUndef. */
	void rot_type ( RotType t ) { _rtype = (gscenum)t; }

	/*! Returns the current used rotation parameterization */
	RotType rot_type () const { return (RotType)_rtype; }

	/*! Access the Euler rotation parameterization model of the joint. */
	KnJointEuler* euler () { return _rot.euler(); }
	const KnJointEuler* ceuler () { return _rot.ceuler(); }
	
	/*! Access the Swing and Twist rotation parameterization model of the joint. */
	KnJointST* st () { return _rot.st(); }
	const KnJointST* cst () { return _rot.cst(); }

	/*! Access the joint rotation class that controls the rotation of the joint. */
	KnJointRot* rot () { return &_rot; }
	const KnJointRot* crot () { return &_rot; }

	/*! This method is the same as rot(), it access the joint rotation class so
		that quaternions can be manipulated. Note that depending on the KnJointRot mode,
		the local (before pre/post) or total (after pre/post) quaternion will be accessed. */
	KnJointRot* quat () { return &_rot; }
	const KnJointRot* cquat () { return &_rot; }

	/*! Set the position values to zero */
	void initpos () { pos()->value(0,0,0); }

	/*! Set the rotation to zero in the joint rotation type */
	void initrot ();

	/*! Set the active channels (both position and rotation) to their "zero" values */
	void init_values () { initpos(); initrot(); }
	
	/*! If the current local matrix is not up to date, it will
		recalculate the local matrix based on the current
		local translation and rotation parameterization */
	void update_lmat ();

	/*! Recursivelly updates the local matrix and the global
		matrices of the joint and all its children. It assumes
		that the global matrix of the parent is up to date.
		Note: whenever this method is called, the whole branch
		under the joint is updated */
	void update_gmat ();

	/*! Same as update_gmat(), but it stops at the given stopjoints */
	void update_gmat ( KnJoint*stopjoint1, KnJoint*stopjoint2 );

	/*! Optimized version of update_gmat() for single branch types of
		skeletons. It will just traverse the first child of each joint.
		Optional stopjoint will terminate early when stopjoint is found */
	void update_branch_gmat ( KnJoint* stopjoint=0 );

	/*! Updates the local matrix and set the global matrix of this joint
		to be the local matrix multiplied by the parent global matrix */
	void update_gmat_local ();

	/*! Finds and updates the local and global matrices of all joints
		in the branch leading to the root joint, until stop_joint or
		until the root is found (stop_joint is not updated). */
	void update_gmat_up ( KnJoint* stop_joint=0 );

	/*! Ensures that the local matrix is updated and returns it. */
	const GsMat& lmat () { update_lmat(); return _lmat; }

	/*! Will force the reconstruction of the local matrix from the
		rotation and position parameters. The skeleton is also notified
		with a call to invalidate_global_matrices() */
	void set_lmat_changed ();

	/*! Returns the current global matrix. Be sure that it is up to
		date by calling one of the several updated methods. It gives
		the transformation from the root to the children of this joint */
	const GsMat& gmat () const { return _gmat; }

	/*! Returns the translation encoded in the current global matrix.
		Be sure that the global matrix is up to date */
	GsVec gcenter () const { return GsVec(_gmat.e14,_gmat.e24,_gmat.e34); }

	/*! Get a single visualization model for this node and all the
		children (update_gmat) is called */
	void unite_visgeo ( GsModel& m );

	/*! Get a single collision model for this node and all the
		children (update_gmat) is called */
	void unite_colgeo ( GsModel& m );

	/*! Recursivelly push to the given list all joints in the subtree of this
		joint, which is not included in the list */
	void subtree ( GsArray<KnJoint*>& jointlist ) const;

	/*! Initializes the ik solver, allocating it if needed, ref/unref rules followed */
	bool ikinit ( KnIk::Type t );

	/*! Defines or updates the ik solver currenlty being referenced */
	void ik ( KnIkSolver* iks ) { updrefs(_ik,iks); }

	/*! Access the ik solver; will be null if none is associated */
	KnIkSolver* ik () { return _ik; }

	/*! Solve and apply the ik to the given target position, which is expected
		to be in global coordinates if fr=='G' and in base coordinates otherwise. */
	KnIk::Result iksolve ( const GsVec& p, char fr='G' );

	/*! Solve and apply the ik to the given target position and orientation, which are expected
		to be in global coordinates if fr=='G' and in base coordinates otherwise, */
	KnIk::Result iksolve ( const GsVec& p, const GsQuat& q, char fr='G' );

	/*! Solve and apply the ik to the given target position and orientation, which are expected
		to be in global coordinates if fr=='G' and in base coordinates otherwise */
	KnIk::Result iksolve ( const GsMat& m, char fr='G' );

	/*! Compute the transformation matrix of this end effector with respect
		to the base joint and save it in ik()->goal */
	void ikgetmat ();
};

//==================================== End of File ===========================================

# endif  // KN_JOINT_H
