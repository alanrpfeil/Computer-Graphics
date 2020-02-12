/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_SKELETON_H
# define KN_SKELETON_H

# include <sig/gs_vars.h>
# include <sig/gs_dirs.h>
# include <sig/gs_table.h>
# include <sig/gs_string.h>
# include <sig/gs_shareable.h>
# include <sigkin/kn_joint.h>
# include <sigkin/kn_channels.h>
# include <sigkin/kn_posture.h>

class KnColdet;
class KnSkin;

//================================ KnSkeleton =================================================

/*! A Skeleton defined as a hierarchy of joints */
class KnSkeleton : public GsShareable
 { protected :
	// basic data:
	GsString _name;
	char* _filename;
	KnJoint* _root;
	GsArray<KnJoint*> _joints;
	mutable GsTable<KnJoint*> _jhash;
	bool _gmat_uptodate;
	bool _enforce_rot_limits;

	// collision detection:
	GsArray<KnJoint*> _colfreepairs;
	int _coldetid;
	friend class KnColdet;

	// posture management:
	KnChannels* _channels;
	GsArray<KnPosture*> _postures;
	KnPostureDfJoints* _dfjoints;

	// skinning:
	KnSkin* _skin;

	// user data:
	GsVars* _userdata;

   public :
	void* udata; // initialized as null and then completely up to user maintainance

   public :
	/*! This is a static boolean variable that, by default, says that bhv data files
		are converted to Quaternion format at loading time (so not remaining EulerYXZ type).
		This variable convertes both joint parameterization and motion rotation data */
	static bool ConvertBvhToQuat;

	/*! Constructor */
	KnSkeleton ();

	/*! Destructor is public but pay attention to the use of ref()/unref() */
	virtual ~KnSkeleton ();

	/*! Set a name for the skeleton */
	void name ( const char* s ) { _name=s; }

	/*! Access to the skeleton name */
	const char* name () { return _name; }

	/*! Access to the user data GsVars, which is always a valid pointer that can be shared */
	GsVars* userdata () { return _userdata; }

	/*! Deletes all data and set the skeleton to be an empty hierarchy */
	void init ();

	/*! Access to the skin object, will be null if non existing */
	KnSkin* skin () { return _skin; }

	/*! Associates by referencing a skin object for this skeleton, unreferencing the previous if any */
	void skin ( KnSkin* skin );

	/*! Set a file name to be associated with the skeleton.
		This information is not saved in the skeleton file and is 
		not used by KnSkeleton. It is here just as a convenient 
		place to store the information */
	void filename ( const char* n ) { gs_string_set(_filename,n); }

	/*! Get the file name associated with the motion */
	const char* filename () const { return _filename? _filename:""; }

	/*! Init the active channels (both position and rotation) of each active joint */
	void init_values ();

	/*! Set if limits defined in local rotation parameterizations should be enforced */
	void enforce_rot_limits ( bool epl ) { _enforce_rot_limits=epl; }

	/*! Returns if location rotation parameterization limits are being enforced */
	bool enforce_rot_limits () const { return _enforce_rot_limits; }

	/*! Adds a joint to the hierarchy. If parent==0 the root joint is created.
		The created joint is pushed to the joint list and becomes the last element.
		If a name is given, the joint will be assigned the equivalent KnJointName. */
	KnJoint* add_joint ( KnJoint::RotType rtype, KnJoint* parent, const char* name=0 );

	/*! Returns the collision detection id (or -1 if no id). */
	int coldetid () const { return _coldetid; }

	/*! Returns a flat list with all joints. Note that method
		KnJoint::index() returns the index of the joint in this list */
	const GsArray<KnJoint*>& joints () const { return _joints; }

	/*! Array with the pairs that should be deactivated for collision
		detection at connection time with GsColdet. The pairs can be
		declared in the skeleton .ks file */
	GsArray<KnJoint*>& coldet_free_pairs () { return _colfreepairs; }

	/*! Returns the channel array of all the active channels */
	KnChannels& channels () { return *_channels; }

	/*! Returns the array of pre-defined postures loaded from the .s file */
	GsArray<KnPosture*>& postures () { return _postures; }

	/*! Returns the KnPostureDfJoints being shared but the loaded postures,
		if nothing was specified, null is returned */
	KnPostureDfJoints* dfjoints () const { return _dfjoints; }

	/*! Rebuilds the array of channels from the active and free joint DOFs,
		and according to their specific rotation parameterizations */
	void make_channels () { _channels->make(this); }
	
	/*! Returns a pointer to the root joint of the skeleton */
	KnJoint* root () const { return _root; }

	/*! Returns the joint with name n. A hash table is used to perform the search.
		0 is returned in case the joint is not found.
		If the hash table is not up to date, it is automatically re-created.
		This method should not be called while the skeleton is being built. */
	KnJoint* joint ( const char* n ) const;

	/*! Returns the joint with name n performing a linear search.
		0 is returned in case the joint is not found.
		This method should be used when the joint list is being constructed,
		as it does not rely on the hash table. */
	KnJoint* lsearch_joint ( const char* n ) const;

	/*! Calls root()->update_gmat() only if it is required due to any
		changes to the local matrices in joints */
	void update_global_matrices ();

	/*! Returns true if all global matrices are up to date */
	bool global_matrices_uptodate () { return _gmat_uptodate; }

	/*! Set the internal flag that controls global matrices update to false.
		This method is automatically called each time a joint value is changed */
	void invalidate_global_matrices ();

	/*! Compress all internal arrays */
	void compress ();

	/*! To fix geometries that were created in global coordinates */
	void set_geo_local ();

	/*! Create collision geometries as copies (not references) of the existing
		visualization geometry in each joint, and removes colgeos in joints
		not having vizgeos. Returns number of colgeos created. */
	int init_colgeos ();

	/*! Remove the normals of all colgeos. Returns the number of colgeos processed.
		Note that if the colgeo is being shared with the visgeo, both become flat. */
	int flat_colgeos ();

	/*! Compute normals to smooth all visgeos. Returns the number of visgeos processed.
		Note that if the visgeo is being shared with the colgeo, both become smooth. */
	int smooth_visgeos ( float crease_angle=GS_TORAD(35.0f) );

	/*! Returns the number of joints having collision geometries attached */
	int colgeos ();

	/*! Returns the number of joints having visualization geometries attached */
	int visgeos ();

	/*! Returns true if a joint is found with a collision geometry. Otherwise returns false. */
	bool hascolgeos ();

	/*! Returns true if a joint is found with a visualization geometry. Otherwise returns false. */
	bool hasvisgeos ();

	/*! Returns the bounding box of all colgeos if geo=='c' or visgeos if geo=='v'. */
	void bbox ( GsBox &box, char geo );

	/*! Count the number of triangles in collision geometries.
		If the given joint id is <0 (the default), all geometries are considered */
	int coltriangles ( int jid=-1 ) const;

	/*! Count the number of triangles in visualization geometries.
		If the given joint id is <0 (the default), all geometries are considered */
	int vistriangles ( int jid=-1 ) const;

	/*! loads a .s, .sd, or .bvh file from given filename.
		Note 1: if lookforsd is set to true, then if a .sd file exists in the same folder of the 
		skeleton file, the .sd file is automatically subsequently loaded and applied to the skeleton.
		Note 2: if lookforw is true, then if a .w file of same name exists, it is then automatically loaded. */
	bool load ( const char* filename, const char* basedir=0, bool lookforsd=true, bool lookforw=true );

	/*! Loads a skeleton hierarchy from the given GsInput in .s or .bvh format (also merges .sd files).
		Returns false if some error is encountered, otherwise true is returned.
		Parameter basedir can specify the base directory used for searching for geometry
		(.m files), which are searched in the paths declared in the .s file, in basedir,
		and in the current directory. Also, relative paths in .s become relative to basedir.
		If basedir is not given (i.e. given as null), it will be extracted from in.filename(), if available.
		Method compress() is called after the file is loaded.
		The skeleton channels will be computed at first load (not from .sd if called for .s) */
	bool load ( GsInput& in, const char* basedir=0 );

	/*! Save in .s format the current skeleton.
		Parameters: If exportgeo is true all associated geometries are also exported to
		the current folder or to the folder extracted from out.filename().
		If singlegeo is true all geometries will be saved in a single (special) .m format. */
	bool save ( GsOutput& out, bool exportgeo=true, bool singlegeo=true, bool extmtls=false );

	/*! Same as the other save() method, but receiving a filename instead of an output */
	bool save ( const char* filename, bool exportgeo=true, bool singlegeo=true, bool extmtls=false );

	/*! Output the name of the joints in the coldet_free_pairs() array */
	void output_coldet_free_pairs ( GsOutput& out);

	/*! Save joints definitions that can be merged into a skeleton */
	bool export_joints ( GsOutput& out );

   private :
	int _loadjdata ( GsInput& in, KnJoint* j, GsDirs& paths, GsInput* igeo );
	KnJoint* _loadj ( GsInput& in, KnJoint* p, GsDirs& paths, int type, GsInput* igeo );

   protected : // special interface to help customize derived classes
	enum InternalEvent { EvMakeChannels, EvGMatsInvalidated, EvGMatsUpdated };
	virtual bool _skeleton_event ( InternalEvent /*ev*/ ) { return true; };
};

/* inline utility to remind how to convert global to local coordinates */
inline void global2local ( const GsMat& gm, GsMat& lm, const GsMat& frame ) { lm.mult ( gm, frame.inverse() ); }

/* inline utility to remind how to convert local to global coordinates */
inline void local2global ( const GsMat& lm, GsMat& gm, const GsMat& frame ) { gm.mult ( lm, frame ); }

//================================ End of File =================================================

# endif  // KN_SKELETON_H
