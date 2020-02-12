/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef CD_MANAGER
# define CD_MANAGER

/** \file cd_manager.h 
 * main collision detection class */

# include <sig/cd_implementation.h>

/* CdManager serves as a class that can be connected to different collision
   detection algorithms, by connecting it to a CdImplementation class */
class CdManager
{  private:
	CdImplementation* _cdi;

   public:
	/*! Set the default implementation to be automatically used by CdManager when it starts */
	static void set_implementation_allocator ( CdImplementation* (*f)() );

	/*! Constructor with optional CdImplementation to be referenced by CdManager */
	CdManager ( CdImplementation* cdi=0 );

	/*! Destructor calls init(). */
   ~CdManager ();
  
	/*! Set new implementation to be used, the old one will get unref(), and new one ref()  */
	void set_implementation ( CdImplementation* cdi );

	/*! Clears everything */
	void init () { _cdi->init(); }

	/*! Create a new object and returns its id. */
	int insert_object ( const GsModel& m ) { return _cdi->insert_object(m); }

	/*! Removes (and deletes) the object from the database. */
	void remove_object ( int id ) { _cdi->remove_object(id); }

	/*! Returns true if id represents a valid id in the database, and false otherwise */
	bool id_valid ( int id ) { return _cdi->id_valid(id); }

	/*! Update the transformation applied to an object.
		Only rotations and translations are supported. */
	void update_transformation ( int id, const GsMat& m ) { _cdi->update_transformation(id,m); }

	/*! Turn on collision detection for an object. */
	void activate_object ( int id ) { _cdi->activate_object(id); }

	/*! Turn off collision detection for an object. */
	void deactivate_object ( int id ) { _cdi->deactivate_object(id); }

	/*! Turn on collision detection between a specific pair of objects. */
	void activate_pair ( int id1, int id2 ) { _cdi->activate_pair(id1,id2); }
	
	/*! Turn off collision detection between a specific pair of objects. */
	void deactivate_pair ( int id1, int id2 ) { _cdi->deactivate_pair(id1,id2); }

	/*! Returns true if the given pair of objects is deactivated and
		false otherwise (-1 ids return false). */
	bool pair_deactivated ( int id1, int id2 ) { return _cdi->pair_deactivated(id1,id2); }

	/*! Counts and returns the number of deactivated pairs */
	int count_deactivated_pairs () { return _cdi->count_deactivated_pairs(); }

	/*! Returns the array with the ids of the colliding objects in
		the last collide_all() or has_collisions() query. The id
		pairs are sequentially stored in the array. Therefore, the number
		of collisions = colliding_pairs.size()/2. */
	const GsArray<int>& colliding_pairs () const { return _cdi->colliding_pairs(); }

	/*! Perform collision detection only untill finding a first collision.
		True is returned if a collision was found; false is returned otherwise. */
	bool collide () { return _cdi->collide(); }

	/*! Perform collision detection among all pairs of objects. The results can
		be retrieved with colliding_pairs() */
	bool collide_all () { return _cdi->collide_all(); }
	
	/*! Returns true as soon as the distance between one pair of models
		is found to be smaller than the given tolerance.
		False is returned when all pairs respects the minimum clearance. */
	bool collide_tolerance ( float toler ) { return _cdi->collide_tolerance(toler); }
};

#endif // CD_MANAGER
