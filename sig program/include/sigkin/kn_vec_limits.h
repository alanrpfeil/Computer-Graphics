/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_VEC_LIMITS_H
# define KN_VEC_LIMITS_H

# include <sig/gs_vec.h>

/*! KnVecLimits maintains a vector and min/max limits for its three (x,y,z) coordinates. */
class KnVecLimits
{  protected :
	float _value[3]; // the values for X, Y, Z
	float _lolim[3]; // lower limits for X, Y, Z
	float _uplim[3]; // upper limits for X, Y, Z
	gscbool _limits[3]; // if limits are active or not
	
   public :
	/*! All parameters 'int d' in the methods below are supposed to
		receive only values 0, 1 or 2. We keep here the original used
		enumerator however we use 'int d' in the methods to save
		lengthy typing when accessing these methods. */
	enum Dof { X=0, Y=1, Z=2 };

   public :
   
	/* Constructor calls init(), which set all values frozen in 0. */
	KnVecLimits () { init(); }

	/* Sets all values and limits to 0. The limits are activated.
	   Therefore all values are initialized as frozen in 0. */
	void init ();

	/*! Init with the same values as in the given v object */
	void init ( const KnVecLimits& v );

	/*! Set values per degree of freedom. In case joint limits are activated,
		values are bounded to the limits before inserted. Returns true if
		a new value was entered, and false otherwise. */
	bool value ( int d, float val );

	/*! Set the three coordinates at once, nothing is returned */
	void value ( const GsVec& v );

	/*! Get the value of the X, Y or Z component */
	float value ( int d ) const { return _value[d]; }
	
	/*! Get a const pointer to the array containing the three X, Y and Z values */
	const float* valuept () const { return _value; }
	
	/*! Returns value as a GsVec */
	GsVec value () const { return GsVec(_value); }

	/*! Returns a random value inside the joint limits.
		If limits are not activated, returns a random value in [-pi,pi] */
	float random ( int d ) const;
	
	/*! Set upper and lower limits and ensures val is inside if limits are active */
	void limits ( int d, float l, float u ) { _lolim[d]=l; _uplim[d]=u; value(d,_value[d]); }
	void lolim ( int d, float l ) { _lolim[d]=l; value(d,_value[d]); }
	void uplim ( int d, float u ) { _uplim[d]=u; value(d,_value[d]); }

	/*! Get lower and upper limits */
	void getlimits ( int d, float& l, float& u ) const { l=_lolim[d]; u=_uplim[d]; }
	float lolim ( int d ) const { return _lolim[d]; }
	float uplim ( int d ) const { return _uplim[d]; }

	/*! Activate/deactivate joint limits control */
	void limits ( int d, bool b )  { _limits[d]=b?1:0; }
	bool limits ( int d ) const { return _limits[d]==1; }

	/*! Returns true if the given value respects current limits and false otherwise */
	bool inlimits ( int d, float val ) const { return _lolim[d]<=val && val<=_uplim[d]; }

	/*! Freezes all values by activating joint limits and setting
		both lower and upper limits to the current value. */
	void freeze ( int d ) { limits(d,true); limits(d,value(d),value(d)); }
	
	/*! Freezes all DOFs of the joint. */
	void freeze () { freeze(X); freeze(Y); freeze(Z); }

	/*! Deactivate joint limits, same as limits(d,false) */
	void thaw ( int d ) { limits(d,false); }

	/*! Deactivate the joint limits of all DOFs */
	void thaw () { limits(X,false); limits(Y,false); limits(Z,false); }
   
	/*! Returns true if the value d is frozen, i.e if it cannot receive new values.
		If the upper limit is equal to the lower limit, and the limits(d)
		is activated, the joint is considered frozen. */
	bool frozen ( int d ) const { if (!_limits[d]) return false; else return _lolim[d]==_uplim[d]; }

	/*! Returns the number of DOFs which are frozen. */
	int nfrozen () const;
};

//==================================== End of File ===========================================

# endif  // KN_VEC_LIMITS_H
