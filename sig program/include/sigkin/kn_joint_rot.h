/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_JOINT_ROT_H
# define KN_JOINT_ROT_H

# include <sigkin/kn_joint_euler.h>
# include <sigkin/kn_joint_st.h>

class KnJoint;

/*! KnJointRot controls the joint rotation with multiple parameterizations that are 
	automatically converted and synchronized as needed. The value() methods will
	manipulate the local quaternion (the one before the pre/post rotations) or
	the total quaternion (after pre/post) depending on the quaternion access mode.
	If pre/post are not defined the mode makes no difference. */
class KnJointRot
{  private :
	struct PrePost { 
		GsQuat pre, post;
		GsQuat apply (const GsQuat& q) const { return pre*q*post; } // applies the pre and post to q
		GsQuat remove (const GsQuat& q) const { return pre.inverse()*q*post.inverse(); } // removes the pre and post from q
	};

   public:
	enum ValueMode { LocalMode, FullMode }; // quaternion value mode

   private:
	enum SyncRot { JT=1, FQ=2, LQ=4, ST=8, EU=16 };
	KnJoint* _joint;   // to notify the joint that the current rotation has changed
	PrePost* _prepost; // local frame adjustment rotations
	GsQuat   _quat;	   // the total rotation of the joint, after applying prerot/postrot
	GsQuat   _lquat;   // the local rotation of the joint, before applying prerot/postrot; if fullmode or no pre/post, will be same as _quat
	gsbyte   _sync;	   // encodes SyncRot flags
	gscbool  _frozen;  // if false KnJointRot will not allow setting new quat values
	gscenum  _mode;	   // stores the joint mode (default Full). Affects how the setvalue() and value() methods will set quaternion after pre/post
	mutable  KnJointST* _st;
	mutable  KnJointEuler* _euler;
	friend class KnJointST;
	friend class KnJointEuler;
	friend class KnJoint;

   public :
	 
	/*! Constructor initializes the rotation as frozen, ie, non active, and
		in "total quaternion mode". */
	KnJointRot ( KnJoint* j );

	/*! Destructor ensures prepost structure is deleted, in case it was used */
   ~KnJointRot ();

	/*! Set the quaternion to a null rotation and invalidate the sync flags. */
	void init () { value(GsQuat::null); }

	/*! Init with the same parameters as in the given jr object */
	void init ( const KnJointRot& jr );

	/*! Sets the internal mode (default is FullMode). If in FullMode the value set
		and retrieved with value() methods is considered already affected by the 
		pre/post rotations. If LocalMode is set value() sets and gets the quaternion 
		without pre/post. If prepost is not defined, then local and full values are
		the same. For all the joint parameterizations (for ex. ST and EU), if
		prepost is present they are expressed in local values.*/
	void setmode ( ValueMode mode ) { _mode=mode; }

	/*! Returns the joint mode. */
	ValueMode getmode() const { return (ValueMode)_mode; }

	/*! Set a new rotation and mark that the rotation is not in sync with
		the associated joint (so that its local matrix is changed when required),
		nor with the other parameterizations. The new rotation is only set if 
		this KnJointRot is active, ie, not frozen. The given quaternion will 
		become the total or local rotation (after or before pre/post rotations),
		according if local or total quaternion mode is set. */
	void value ( const GsQuat& q ) { value(q.e); }

	/*! Same value(q) method, but receiving a float pointer as input */
	void value ( const float* f );

	/*! Access the current full or local joint rotation according to the current mode. 
		If the most recent rotation was set from a local parameterization, it will be 
		automatically converted, multiplied by pre and post and then returned. */
	const GsQuat& value ();

	/*! Equivalent to value() but will always return the full rotation (same as local if no prepost). */
	const GsQuat& fullvalue ();

	/*! Equivalent to value() but will always return the local rotation (same as full if no prepost). */
	const GsQuat& localvalue ();

	/*! Freeze rotations, ie, make new calls to value(q) to have no effect */
	void freeze () { _frozen=1; }
	
	/*! Set the rotation active, i.e accepting new values. */
	void thaw () { _frozen=0; }

	/*! Returns true if the joint is active, and false otherwise  */
	bool frozen () const { return _frozen==1; }

	/*! Returns true if there are pre/post rotations assigned for the derived class */
	bool hasprepost () const { return _prepost!=0; }
	
	/*! Remove any existing prepost rotation */
	void initprepost ();

	/*! Specifies a pre-multiplied frame rotation adjustment to help placing the
		derived class rotation parameterization frame of reference as needed */
	void prerot ( const GsQuat& q );

	/*! Specifies a post-multiplied frame rotation adjustment for the derived class */
	void postrot ( const GsQuat& q );

	/*! Options for method adjust_prepost() */
	enum AlignType { AlignPre, AlignPost, AlignPrePost, AlignPreInv, AlignPostInv };

	/*! Adjust pre and post rotations for alignment in v direction. Options:
		AlignPre: align local frame so that v follows incoming parent link;
		AlignPost: align local frame so that v follows outgoing child(0) link;
		AlignPrePost: performs both Pre and Post alignments;
		AlignPreInv: performs Pre alignment and accumulates its inverse in pogsot;
		AlignPostInv: performs Post alignment and accumulates its inverse in prerot; */
	void align ( AlignType t, const GsVec& v );
	
	/*! Returns the current pre-multiplied rotation adjustment for the derived class
		note: quat.w is 1 if no rotation */
	const GsQuat& prerot () const { return _prepost? _prepost->pre:GsQuat::null; }

	/*! Returns the current pos-multiplied rotation adjustment for the derived class
		note: quat.w is 1 if no rotation */
	const GsQuat& postrot () const { return _prepost? _prepost->post:GsQuat::null; }

	/*! Returns a pointer to the swing-twist parameterization of this rotation.
		If the associated joint is not of swing-twist type a default swing-twist
		parameterization will be created and returned. The joint type is not changed. */
	KnJointST* st () { if(!_st) _st=new KnJointST(this); return _st; }

	/*! Const version of st() method */
	const KnJointST* cst () { if(!_st) _st=new KnJointST(this); return _st; }

	/*! Returns a pointer to the euler parameterization of this rotation.
		If the associated joint is not of euler type a default euler parameterization
		will be created and returned. The joint type is not changed. */
	KnJointEuler* euler () { if(!_euler)_euler=new KnJointEuler(this); return _euler; }
	
	/*! Const version of euler() method */
	const KnJointEuler* ceuler () { if(!_euler)_euler=new KnJointEuler(this); return _euler; }

   private : // recall that private methods are accessible to friend classes (joint/euler/st)

	void setowner ( SyncRot sr );
	void syncto ( SyncRot sr , bool force = false );
	int insync ( SyncRot sr ) const { return _sync&sr; }
};

//==================================== End of File ===========================================

# endif  // KN_JOINT_ROT_H
