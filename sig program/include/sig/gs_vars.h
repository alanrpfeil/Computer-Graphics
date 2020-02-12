/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_VARS_H
# define GS_VARS_H

/** \file gs_vars.h 
 * an array of GsVars
 */

# include <sig/gs.h>
# include <sig/gs_vec.h>
# include <sig/gs_var.h>
# include <sig/gs_shareable.h>

/*! \class GsVars gs_vars.h
	\brief a table of generic type variables

	GsVars has methods to manage a table of variables,
	which is always kept sorted by name. */
class GsVars : public GsShareable
{  public :
	enum OutputFormat { NormalFormat, ReverseFormat };
   private :
	GsArray<GsVar*> _table;
	OutputFormat _outfmt;
	char* _name;

   public :	
	/*! Constructor */
	GsVars () { _name=0; _outfmt=NormalFormat; }

	/*! Copy constructor */
	GsVars ( const GsVars& v );

	/*! Destructor */
	~GsVars ();

	/*! Change the desired output format. */
	void output_format ( OutputFormat of ) { _outfmt=of; }

	/*! Sets a name for the var table. Null can be passed to clear the name. */
	void name ( const char* n );

	/*! Returns the associated name. "" is returned if no name was set. */
	const char* name () const;

	/*! Clear the table */
	void init ();

	/*! Compress free spaces */
	void compress () { _table.compress(); }

	/*! Returns size, ie, number of vars entries */
	int size () const { return _table.size(); }

	/* Looks for the name (as in get()) and if not there prints an error and exit.
	   Otherwise the found var is returned. */
	GsVar* test ( const char* name ) const;

	/*! Get a const reference to a var from an index, which must be in valid range */
	const GsVar& cget ( int i ) const { return *_table[i]; }

	/*! Get a reference to a var from an index, which must be in valid range */
	GsVar& get ( int i ) const { return *_table[i]; }

	/* Does a binary search for the name, and returns a pointer to the found var.
	   If not found, null is returned. */
	GsVar* get ( const char* name ) const;

	/* Does a binary search for the name and returns the value if the var is found,
	   in which case true is returned. If the name is not found, the value
	   receives value 'defvalue' and false is returned. */
	bool get ( const char* name, int& value, int defvalue=0 );
	bool get ( const char* name, bool& value, bool defvalue=0 );
	bool get ( const char* name, float& value, float defvalue=0 );
	bool get ( const char* name, double& value, double defvalue=0 );

	/*! Same as the other get() methods, but here value is a pointer to
		a string that will be re-allocated with operator new to contain the new value */
	bool get ( const char* name, char*& value, const char* defvalue="" );

	/*! Version of get() based on GsString */
	bool get ( const char* name, GsString& value, const char* defvalue="" );

	/* Does a binary search for the name, and returns the value if the name was
	   found. If not found, 0 or "" is returned. */
	int geti ( const char* name, int index=0 ) const { return test(name)->geti(index); }
	bool getb ( const char* name, int index=0 ) const { return test(name)->getb(index); }
	float getf ( const char* name, int index=0 ) const { return test(name)->getf(index); }
	const char* gets ( const char* name, int index=0 ) const { return test(name)->gets(index); }
	GsVec getv ( const char* name ) const;
	void getv ( const char* name, float* fp, int size ) const;

	/* Does a binary search for the name and set the value if the var is found.
	   Returns a pointer to the found var or null if no var was found.
	   Automatic type casts are performed if required. */
	GsVar* set ( const char* name, int value, int index=0 );
	GsVar* set ( const char* name, bool value, int index=0 );
	GsVar* set ( const char* name, float value, int index=0 );
	GsVar* set ( const char* name, const char* value, int index=0 );
	GsVar* set ( const char* name, double value, int index=0 )
		   { return set(name,float(value),index); }

	/*! Access operator. The index must be in a valid range */
	GsVar& operator [] (int i ) const { return get(i); }

	/* Does a binary search and returns the index of the
	   found variable. Returns -1 if not found. */
	int search ( const char* name ) const;

	/*! Add v to the table. Variable v should be created with operator
		new, and will be managed by the table after insertion.
		Returns the allocated position of v.
		If a duplicated entry is already in the table, v is deleted and
		not inserted, but the value in v is passed to the old entry and
		the old entry id is returned. */
	int add ( GsVar* v );

	/* Allocate a new GsVar and insert it to the table with method insert(). */
	int add ( const char* name, int value ) { return add(new GsVar(name,value)); }
	int add ( const char* name, bool value ) { return add(new GsVar(name,value)); }
	int add ( const char* name, float value ) { return add(new GsVar(name,value)); }
	int add ( const char* name, double value ) { return add(new GsVar(name,float(value))); }
	int add ( const char* name, const char* value ) { return add(new GsVar(name,value)); }

	/*! Remove variable with index i. If i is out of range nothing is done */
	void remove ( int i );

	/*! Take the vars of vt. If duplicated names are found, the values found
		in vt takes place. Var table vt becomes empty. */
	void merge ( GsVars& vt ); 

	/*! Extract the variable with index i from the table. The user becomes
		responsible of managing the deletion of the returned pointer.
		If i is out of range, 0 is returned. */
	GsVar* extract ( int i );

	/*! Output the name and the variables between curly brackets.
		The format, in summary, is: name { var1=val; var2=val1 val2; } */
	friend GsOutput& operator<< ( GsOutput& o, const GsVars& v );

	/*! Read name and data. Flexible if name or brackets are not present. */
	friend GsInput& operator>> ( GsInput& in, GsVars& v );
};

//================================ End of File =================================================

# endif  // GS_VARS_H

