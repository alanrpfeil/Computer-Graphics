/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_VAL_H
# define GS_VAL_H

/** \file gs_val.h 
 * a multi type value
 */

# include <sig/gs.h>
# include <sig/gs_input.h>
# include <sig/gs_output.h>

/*! \class GsVal gs_val.h
	\brief generic type value
	GsVal keeps a int, bool, float or string value */
class GsVal
 { private :
	union Val { bool b; int i; float f; char* s; };
	Val  _val;  // keeps the value
	char _type; // type 'b', 'i', 'f', or 's'

   public :

	/*! Constructs a GsVal of type 'i' and zero value. */
	GsVal () { _type='i'; _val.i=0; }

	/*! Copy constructor */
	GsVal ( const GsVal& v ) { _type='i'; init(v); }

	/*! Constructs a GsVal with the given type and zero value.
		Parameter type can be 'b', 'i', 'f', or 's'.
		If the given type is unknown, type 'i' is set. */
	GsVal ( char type ) { _type='i'; init(type); }

	/*! Constructs a type 'b' GsVal with the given value */
	GsVal ( bool value ) { _type='b'; _val.b=value; }

	/*! Constructs a type 'i' GsVal with the given value */
	GsVal ( int value ) { _type='i'; _val.i=value; }

	/*! Constructs a type 'f' GsVal with the given float value */
	GsVal ( float value ) { _type='f'; _val.f=value; }

	/*! Constructs a type 'f' GsVal with the given double value converted to float */
	GsVal ( double value ) { _type='f'; _val.f=(float)value; }

	/*! Constructs a type 's' GsVal with the given string value */
	GsVal ( const char* value ) { _type='s'; _val.s=gs_string_new(value); }

	/*! Destructor deletes the maintained string if of 's' type */
	~GsVal () { if (_type=='s') gs_string_delete(_val.s); }

	/*! Inits GsVal with the given type. Types are specified by the first
		letter of: bool, int, float, or string.
		if a non recognized type is given GsVal is set to type 'i'. */
	void init ( char type );

	/*! Makes GsVal be identical to v, same as operator= */
	void init ( const GsVal& v );

	/*! Returns the letter corresponding to the type of the value, which
		is one of: 'b', 'i', 'f' or 's' */
	char type () const { return _type; }

	/*! Methods set() are used to set a new value in GsVal.
		Automatic type casts are performed in case of type mixing: the
		new values given are converted to the actual type of GsVal if needed */
	void set ( bool b );
	void set ( int i );
	void set ( float f );
	void set ( double d ) { set(float(d)); }
	void set ( const char* s );

	/*! Methods get() are used to get a value from GsVal.
		The returned value is a converted from the actual GsVal value if needed. */
	bool  getb () const;
	int   geti () const;
	float getf () const;
	const char* gets () const; //!< returns "" if GsVal is not of type 's'

	/*! Makes GsVal be identical to v, same as init(v) */
	GsVal& operator= ( const GsVal& v ) { init(v); return *this; }

	/*! Output the current value. */
	friend GsOutput& operator<< ( GsOutput& o, const GsVal& v );

	/*! Input a value and set the type of GsVal according to the value type */
	friend GsInput& operator>> ( GsInput& in, GsVal& v );
};

//================================ End of File =================================================

# endif  // GS_VAL_H

