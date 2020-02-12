/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_RANDOM_H
# define GS_RANDOM_H

/** \file gs_random.h 
 * Random number generation */

# include <sig/gs.h>

//================================= GsRandom ====================================

/*! GsRandom facilitates generating bounded random numbers.
	All methods use the gs_random() functions. */
template <typename X>
class GsRandom
 { public:
	X min, max;

   public :

	/*! Default constructor sets the random generator to have limits [0,1] */
	GsRandom () { limits((X)0,(X)1); }

	/*! Constructor with given limits */
	GsRandom ( X minval, X maxval ) { limits(minval,maxval); }
 
	/*! Set limits */
	void limits ( X minval, X maxval ) { min=minval; max=maxval; }

	/*! Returns a random number in [min,max]. */
	X get () const { return gs_random(X(min),X(max)); }

	/*! Returns a random int number in [min,max]. */
	int geti () const { return gs_random(int(min),int(max)); }
	
	/*! Returns a random float number in [min,max], with 32-bit resolution. */
	float getf () const { return gs_random(float(min),float(max)); }

	/*! Returns a random double number in [min,max], with 53-bit resolution. */
	double getd () const { return gs_random(double(min),double(max)); }
};

//============================== end of file ======================================

# endif  // GS_RANDOM_H

