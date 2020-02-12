/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_TIMER_H
# define GS_TIMER_H

/** \file gs_timer.h 
 * Measure time and execution per second rates.*/

# include <sig/gs.h>

/*! \class GsTimer gs_timer.h
	\brief Measure time and execution per second rates.

	GsTimer is intended to help counting the time taken by any function or
	portion of code. It can also be used to measure the frames per second 
	rate of an application. After instantiation, the user should call start()
	just before the function to measure the time, and stop() right after. Then,
	the method dt() will return the elapsed time between both function calls.
	Many other time measurements are possible. It is considered that each time
	the start/end pair is called, one measurement is done. In this way the user
	can perform several measurements (like in a loop) and then retrieve the 
	meandt(), calculated according to the specified interval(). Times between
	measurements can also be retrieved with loopdt(). */
class GsTimer
 { private :
	double _interval; // the sampling interval
	double _last_start;
	double _last_stop;
	double _measurement_number;
	double _last_mps_number;
	double _last_mps_time;
	double _mps;
	double _loopdt; 
	double _loopdt_sum;
	double _meandt;
	double _meandt_sum;
	double _dt;

   public : 
	/*! Constructs a counter giving the sampling interval to consider for
		calculating average times. The default interval value is 15 measurements.
		In this case, each time the pair start()/stop() is called "interval" 
		times, a new mps() value is calculated. If interval is initialized
		with 0, average times are not computed. */ 
	GsTimer ( gsuint interval=15 );

	/*! Changes the sampling interval being used. */
	void interval ( gsuint i ) { _interval=(double)i; }

	/*! Returns the sampling interval being used. */
	gsuint interval () const { return (gsuint)_interval; }

	/*! To be called just before your time measurement starts.
		This method simply calls the base class method start(). */
	void start () { _last_start=gs_time(); }

	/*! To be called just after your time measurement ends.
		This method calculates the ellapsed time since the last start()
		call (retrievable with dt()), and updates the mean times if the
		number of measurements performed reaches the specified interval. */
	void stop ();

	/*! Returns the elapsed time since the last call to start(). */
	double t () const { return gs_time()-_last_start; }

	/*! Returns the time taken by the last start/stop measurement. */
	double dt () const { return _dt; } 

	/*! Returns the number of measurements already performed. The returned
		double type contains an integer value. */
	double measurements () const { return _measurement_number; }

	/*! Returns the mean elapsed time taken by the last interval measurements,
		according to the specified sampling interval. */
	double meandt () const { return _meandt; } 

	/*! Returns the current measurements per second rate being achieved,
		measured according to the current sampling interval. Note: if the 
		sampling interval is zero, the mps is simply not computed and the
		last value is returned. */
	double mps () const { return _mps; }

	/*! Returns the mean time taken between measurements, according to the
		current sampling interval. One "loop delta t" is considered to be
		the ellapsed time of two consecutive stop() calls. As such measurements
		makes only sense to perform inside loops, loopdt() returns always the
		mean value according to the sampling interval. */
	double loopdt () const { return _loopdt; }

	/*! Returns the time of the last call to stop(). */
	double last_stop () const { return _last_stop; } 

	/*! Returns the time of the last call to start(). */
	double last_start () const { return _last_start; } 
};

//============================= end of file ==========================

# endif // GS_TIMER_H
