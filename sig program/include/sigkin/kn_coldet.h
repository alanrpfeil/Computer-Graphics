/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_COLDET_H
# define KN_COLDET_H

//================================ KnColdet =================================================

# include <sig/gs_slot_map.h>
# include <sig/gs_shareable.h>

class KnJoint;
class KnSkeleton;
class CdManager;

/*! Manages collision detection between several articulated objects,
	including self-collisions. Based on the CdManager class.
	Note: this class does not allow inserting GsModels alone; instead,
	a single-joint KnSkeleton must be created to reference the model. */
class KnColdet : public GsShareable
{  private :
	class SkelData;
	GsSlotMap<SkelData> _skels;
	CdManager* _coldet;
	GsArray<KnJoint*> _joints; // contains all joints

   public :
	/*! Constructor */
	KnColdet ();

	/*! Destructor is public but be sure to respect ref()/unref() */
   ~KnColdet ();

	/*! Clears everything, ie, disconnects all skeletons */
	void init ();

	/*! Returns the total number of collision geometries being considered */
	int num_objects () { return _joints.size(); }

	/*! Connects and constructs internal structures for each collision
		geometry attached to the skeleton.
		If parameter deact is given as null (the default) three automatic deactivations
		are performed; otherwise, deact will be a string with letters D, A, C, in that
		order, specifying which deactivations to perform:
		D: declared non-colliding-pairs in the skeleton are automatically deactivated;
		A: Adjacent joints in the skeleton are automatically deactivated;
		C: all initially colliding pairs are deactivated.
		The id of the inserted joints can be retrieved from the KnSkeleton methods.
		Several calls to connect() will correctly add several skeletons.
		If the skeleton is already connected to a KnColdet object, it is re-inserted.
		If pointer pairs is not null, all deactivated pairs are pushed in the array.
		The number of encountered colliding pairs is returned. */
	int connect ( KnSkeleton* s, const char* deact=0, GsArray<KnJoint*>* pairs=0 );

	/*! Updates the global transformation matrices of each collision geometry
		in s, which has to be connected. The number of updated geometries is returned. */
	int update ( KnSkeleton* s );

	/*! Updates the global transformation matrix of the given joint. This
		is a low level method that assumes j valid and connected to coldet */
	void update ( KnJoint* j );

	/*! Recursivelly updates the global transformation matrix of the given joint
		and all its children. Assumes j valid and connected to coldet */
	void update_subtree ( KnJoint* j );

	/*! Check for all collisions returning true if any collisions were found.
		All colliding pairs are returned in the giving array */
	bool collide ( GsArray<KnJoint*>& pairs );

	/*! Check for collisions returning true if any collisions were found. */
	bool collide ();

	/*! Output a report of all colliding pairs */
	void collide_report ( GsOutput& o );

	/*! Returns true if a pair is closer than toler. */
	bool collide_tolerance ( float toler );

	/*! Deactivate all pairs of adjacent joints in the skeleton hierarchy.
		This operation is (by default) performed by method connect(). */
	int deactivate_adjacent_joints ( KnSkeleton* kn );

	/*! Deactivate a pair of joints */
	bool deactivate ( KnJoint* j1, KnJoint* j2 );

	/*! Deactivate all pairs of joints in this skeleton */
	int deactivate_all_pairs ( KnSkeleton* sk );

	/*! Activate a pair of joints */
	bool activate ( KnJoint* j1, KnJoint* j2 );

	/*! Deactivate a joint */
	void deactivate ( KnJoint* j );

	/*! Activate a joint */
	void activate ( KnJoint* j );

	/*! Deactivate all joints in a skeleton */
	void deactivate ( KnSkeleton* s );

	/*! Activate all joints in a skeleton */
	void activate ( KnSkeleton* s );
	
	/*! Returns true if the given pair is deactivated and false otherwise */
	bool deactivated ( KnJoint* j1, KnJoint* j2 );

	/*! Counts and returns the number of deactivated pairs of joints */
	int count_deactivated_pairs ();

   private :
	void _addjoint ( int id, KnJoint* j );
};


//================================ End of File =================================================

# endif  // KN_COLDET_H
