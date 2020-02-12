/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_TIME_H
# define GS_TIME_H

/** \file gs_time.h 
 * Manipulate time elements.*/

# include <sig/gs_input.h>
# include <sig/gs_output.h>

class GsString;

/*! \class GsTime gs_time.h
	\brief Manipulate a time in the format hh:mm:ss

	GsTime allows to accumulate seconds (eg from a timer) and
	format it in the format hh:mm:ss. */
class GsTime
 { private :
	int _h, _m;
	float _s;

   public :

	/*! Constructs with time 0:0:0 */ 
	GsTime ();

	/*! Reset time to 0:0:0 */ 
	void init () { _h=_m=0; _s=0.0f; }
	
	/*! Returns the current hour */ 
	int h() const { return _h; }

	/*! Returns the current minutes */ 
	int m() const { return _m; }

	/*! Returns the current seconds */ 
	float s() const { return _s; }

	/*! Sets a time. Parameters can have any positive values, which will
		be normalized to ensure m<60 and s<60. */
	void set ( int h, int m, float s );
 
	/*! Sets a time with float parameters. Parameters can have any positive
		values, which will be normalized to ensure m<60 and s<60. */
	void set ( float h, float m, float s );

	/*! Add two times, normalizing the results. */
	void add ( const GsTime& t );

	/*! Add the given amount of hours and normalize the results. */
	void add_hours ( float h );
 
	/*! Add the given amount of minutes and normalize the results. */
	void add_minutes ( float m );

	/*! Add the given amount of seconds and normalize the results. */
	void add_seconds ( float s );

	/*! Accumulates the given time, same as add() method. */
	GsTime& operator += ( const GsTime& t );

	/*! Set the string s to contain the time in format hh:mm:ss,
		and returns a reference to string s. */
	GsString& print ( GsString& s ) const;

	/*! Set the string s to contain the time in format hh:mm:ss.nnnn
		and returns a reference to string s. */
	GsString& printms ( GsString& s ) const;

	/*! Outputs the time in format hh:mm:ss. */
	friend GsOutput& operator<< ( GsOutput& o, const GsTime& t );

	/*! Input the time from format hh:mm:ss or hh:mm:ss.nnnn. */
	friend GsInput& operator>> ( GsInput& in, GsTime& t );
};

//============================= end of file ==========================

# endif // GS_TIME_H
