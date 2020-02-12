/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution.
  =======================================================================*/

# ifndef GS_VAR_H
# define GS_VAR_H

/** \file gs_var.h
 * multi type variable
 */

# include <sig/gs.h>
# include <sig/gs_array.h>

/*! \class GsVar gs_var.h
	\brief generic multi-dimensional variable

	GsVar keeps generic type and multi dimension variable values. */
class GsVar
 { private :
	union Var { bool b; int i; float f; char* s; };

	GsArray<Var> _data; // keeps the elements
	char* _name;		// contains name, if used
	char _type;			// type 'b', 'i', 'f', or 's'

   public :

	/*! Constructs an empty GsVar as type 'i' and name "" */
	GsVar ();

	/*! Constructs an empty GsVar with the given type and name "".
		Parameter type can be 'b', 'i', 'f', or 's'.
		If the given type is unknown, type 'i' is set. */
	GsVar ( char type );

	/*! Constructs an empty GsVar as the given type and name.
		If the given type is unknown, type 'i' is set. */
	GsVar ( const char* name, char type );

	/*! Constructs a type 'b' GsVar with the given name and value */
	GsVar ( const char* name, bool value );

	/*! Constructs a type 'i' GsVar with the given name and value */
	GsVar ( const char* name, int value );

	/*! Constructs a type 'f' GsVar with the given name and float value */
	GsVar ( const char* name, float value );

	/*! Constructs a type 'f' GsVar with the given name and double value */
	GsVar ( const char* name, double value ) { GsVar ( name, float(value) ); }

	/*! Constructs a type 's' GsVar with the given name and value */
	GsVar ( const char* name, const char* value );

	/*! Copy constructor */
	GsVar ( const GsVar& v );

	/*! Deletes all used internal data */
	~GsVar ();

	/*! Sets a name to be kept by GsVar. Null can be passed to clear the name. */
	void name ( const char* n );

	/*! Returns the associated name. "" is returned if no name was set. */
	const char* name () const;

	/*! Inits GsVar with the given type. Types are specified by the first
		letter of: bool, int, float, or string.
		This method will not free extra spaces existing in internal arrays.
		For such purpose, compress() should be used.
		GsVar values can then be defined with one of the set methods.
		Note: if a non recognized type is given GsVar is set to type 'i'. */
	void init ( char type );

	/*! Makes GsVar be identical to v, same as operator= */
	void init ( const GsVar& v );

	/*! Frees any extra spaces existing in internal array */
	void compress () { _data.compress(); }

	/*! Returns the actual array size of the variable */
	int size () const { return _data.size(); }

	/*! Set a new array size */
	void size ( int ns );

	/*! Returns the letter corresponding to the type of the variable, which
		is one of: 'b', 'i', 'f' or 's' */
	char type () const { return _type; }

	/*! Methods set() are used to update a value or to push a new value in GsVar.
		Parameter index gives the position to update the value and has
		the default value of 0.
		If the index is out of range, the value is pushed after
		the last current value, increasing the array size of GsVar.
		Automatic type casts are performed in case of type mixing */
	void set ( bool b, int index=0 );
	void set ( int i, int index=0 );
	void set ( float f, int index=0 );
	void set ( double d, int index=0 ) { set(float(d),index); }
	void set ( const char* s, int index=0 );

	/*! Methods push() are used to push a new value in GsVar. */
	void push ( bool b );
	void push ( int i );
	void push ( float f );
	void push ( double d ) { push(float(d)); }
	void push ( const char* s );

	/*! Methods get() are used to get a value from GsVar.
		Parameter index gives the position to retrieve the value and has
		the default value of 0.
		If the index is out of range, zero is returned ("" for gets() ) */
	bool getb ( int index=0 ) const;
	int geti ( int index=0 ) const;
	float getf ( int index=0 ) const;
	const char* gets ( int index=0 ) const;

	/*! Starting at position i (included), this method removes n positions.
		Parameter n has default value of 1 */
	void remove ( int i, int n=1 );

	/*! Insert at position i, n positions.
		Parameter n has default value of 1 */
	void insert ( int i, int n=1 );

	/*! Makes GsVar be identical to v, same as init(v) */
	GsVar& operator= ( const GsVar& v );

	/*! Output GsVar.
		If GsVar has no name, name "var" is used. If GsVar is empty,
		a 0 value ("" for string type) is used. */
	friend GsOutput& operator<< ( GsOutput& o, const GsVar& v );

	/*! Input GsVar. The type of v will change according to the first element
		found in the input after the '=' sign. For example, the input "point=1.0 2;"
		will set v with name 'point', type float, and values 1.0f and 2.0f */
	friend GsInput& operator>> ( GsInput& in, GsVar& v );

	/*! C style comparison function compares the names of the variables,
		returning <0, 0, or >0. */
	static int compare ( const GsVar* v1, const GsVar* v2 );
};

//================================ End of File =================================================

# endif  // GS_VAR_H

