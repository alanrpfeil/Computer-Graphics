/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef CD_IMPLEMENTATION
# define CD_IMPLEMENTATION

/** \file cd_implementation.h 
 * contains a collision detector implementation */

# include <sig/gs_model.h>

/* CdImplementation encapsulates a collision
   detection algorithm to be connected to a CdManager class */
class CdImplementation : public GsShareable
 { protected:
	GsArray<int> _pairs; // will contain the pairs of detected collisions
 
   public:
	/*! Constructor */
	CdImplementation ();

	/*! Virtual destructor  */
	virtual ~CdImplementation ();
  
	/*! Clears everything */
	virtual void init ();

	/*! Create a new object and returns its id. */
	virtual int insert_object ( const GsModel& m );

	/*! Removes (and deletes) the object from the database. */
	virtual void remove_object ( int id );

	/*! Returns true if id represents a valid id in the database, and false otherwise */
	virtual bool id_valid ( int id );

	/*! Update the transformation applied to an object.
		Only rotations and translations are supported. */
	virtual void update_transformation ( int id, const GsMat& m );

	/*! Turn on collision detection for an object. */
	virtual void activate_object ( int id );

	/*! Turn off collision detection for an object. */
	virtual void deactivate_object ( int id );

	/*! Turn on collision detection between a specific pair of objects. */
	virtual void activate_pair ( int id1, int id2 );
	
	/*! Turn off collision detection between a specific pair of objects. */
	virtual void deactivate_pair ( int id1, int id2 );

	/*! Returns true if the given pair of objects is deactivated and
		false otherwise (-1 ids return false). */
	virtual bool pair_deactivated ( int id1, int id2 );

	/*! Counts and returns the number of deactivated pairs */
	virtual int count_deactivated_pairs ();

	/*! Returns the array with the ids of the colliding objects in
		the last collide_all() or has_collisions() query. The id
		pairs are sequentially stored in the array. Therefore, the number
		of collisions = colliding_pairs.size()/2. */
	virtual const GsArray<int>& colliding_pairs () const;

	/*! Perform collision detection only untill finding a first collision.
		True is returned if a collision was found; false is returned otherwise. */
	virtual bool collide ();

	/*! Perform collision detection among all pairs of objects. The results can
		be retrieved with colliding_pairs() */
	virtual bool collide_all ();
	
	/*! Returns true as soon as the distance between one pair of models
		is found to be smaller than the given tolerance.
		False is returned when all pairs respects the minimum clearance. */
	virtual bool collide_tolerance ( float toler );
 };

#endif // CD_IMPLEMENTATION
