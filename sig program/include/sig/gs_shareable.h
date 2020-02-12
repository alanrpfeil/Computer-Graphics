/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_SHAREABLE_H
# define GS_SHAREABLE_H

/*! \file gs_shared.h 
	Reference counter for smart pointer behavior.
	Note: attention is required to avoid circular references */
class GsShareable
{ private :
	unsigned int _ref;

  protected :
	/*! Constructor initializes the reference counter as 0 */
	GsShareable () { _ref=0; };

	/*! Destructor will generate a fatal error in case the class is deleted with ref>0.
		Derived classes should have destructors declared as protected if the intent is 
		to have users always calling unref() instead of delete. */
	virtual ~GsShareable();

  public :

	/*! Returns true if the reference counter has 0 or 1, and false otherwise. */
	bool singleref () const { return _ref<=1; }

	/*! Returns the current reference counter value. */
	unsigned getref () const { return _ref; }

	/*! Increments the reference counter. */
	void ref () { _ref++; }

	/*! Decrements the reference counter, and if the counter becomes 0,
		the object is automatically self deleted. A fatal error is generated
		in case unref is called with the number of references being zero. */
	void unref();
};

/*! Unreferences obj1 (if not null) and references obj2 (if not null).
	Object obj2 is referenced first such that no destruction can happen if obj1 is equal to obj2. */
void unrefref ( GsShareable* obj1, GsShareable* obj2 );

/*! References newobj, then unreferences curobj and makes curobj point to newobj.
	Objects curobj and newobj are only unrefd/refd if not null. 
	A call to updref(curobj,0) will make curobj return with 0 value. */
template <class S>
inline void updrefs ( S*& curobj, S* newobj )
{	::unrefref((GsShareable*)curobj,(GsShareable*)newobj); curobj=newobj; }

//============================== end of file ===============================

# endif // GS_SHAREABLE_H
