/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_JOINT_NAME_H
# define KN_JOINT_NAME_H

# include <sig/gs_table.h>

/*! KnJointName contains only one integer id, which is the id of
	a name stored in a globally defined hash table.
	The hash table management is transparent and joint names 
	comparison is thus performed simply by an integer comparison
	(name comparison is always case-insensitive). */
class KnJointName
 { private :
	gsuint16 _id; // the id of this joint name (max is 65535, see gs.h)
	static gsuint16 _undefid; // to mark undefined ids (mark is max gsuint16 value)
	static GsTable<gsintp> _htable;

   public :

	/*! Constructor initializing this name as undefined */
	KnJointName () { _id=_undefid; }

	/*! Constructor initializing this name with the same id as in jn */
	KnJointName ( const KnJointName& jn ) { _id=jn._id; }

	/*! Constructor initializing this name with the given string, which is inserted
		in the global hash table (if not already there) */
	KnJointName ( const char* st ) { this->operator=(st); }

	/*! Returns true if the name is undefined, ie, its index is equal to 65535 */
	bool undefined () const { return _id==_undefid; }

	/*! Inserts the given string in the global hash table (if not already there)
		and stores its index as identifier of the KnJointName.
		If the given st is null, the string is set as undefined. */
	void operator= ( const char* st );
	
	/*! Assigns to this name the same id as in jn */
	void operator= ( const KnJointName& jn ) { _id=jn._id; }
	
	/*! Comparison operator simply compares ids */
	bool operator== ( const KnJointName& jn ) { return _id==jn._id; }

	/*! Comparison operator with a string without inserting the string in the 
		hash table. Case-insensitive. Always returns false if name is undefined. */
	bool operator== ( const char* st );

	/*! Again case-insensitive comparison operator without inserting st in the hash table. */
	bool operator!= ( const char* st ) { return !operator==(st); }

	/*! Type cast to a const char pointer; "" is returned in case the name is undefined.
		Note: this type cast must be explicitly used when sending a KnJointName to printf() */
	operator const char* () const { return undefined()? "":_htable.key(_id); }
	
	/*! return the associated string; "" is returned in case the name is undefined. */
	const char* st () const { return undefined()? "":_htable.key(_id); }

	/*! Returns the unique id of this name; usefull for debug purposes only */
	gsuint16 id () const { return _id; }
	
	/*! Returns true if the string exists among all joint names */
	static bool exists ( const char* name );

	/*! return the associated string; "" is returned in case the name is undefined. */
	static const char* st ( gsuint16 id ) { return id==_undefid? "":_htable.key(id); }

   private :
	void _check () { if ( _htable.hashsize()==0 ) _htable.init(256); }
};

//==================================== End of File ===========================================

# endif  // KN_JOINT_NAME_H
