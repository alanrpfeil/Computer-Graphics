/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_timer.h>

//======================= GsTimer =====================================

GsTimer::GsTimer ( gsuint interval )
{
	_interval = (double)interval; 
	_last_start = 0.0;
	_last_stop = 0.0;
	_measurement_number = 0.0;
	_last_mps_number = 0.0;
	_last_mps_time = 0.0;
	_mps = 0.0;

	_loopdt = 0.0;
	_loopdt_sum = 0.0;
	_meandt = 0.0;
	_meandt_sum = 0.0;
	_dt = 0.0;
}

void GsTimer::stop ()
{
	// The first thing to do is to take the actual time:
	double curtime = gs_time();

	// Increment the measurements counter:
	_measurement_number++;

	// Save the dt of this measurement:
	_dt = curtime - _last_start;

	// Accumulate deltas used to get mean values:
	_meandt_sum += _dt;
	_loopdt_sum += curtime - _last_stop;

	// Check if it is time to update mean values:
	if ( _measurement_number-_last_mps_number == _interval )
	{
		// mps calculation :
		_mps = _interval / (curtime-_last_mps_time); // frames/secs
		_last_mps_time = curtime;
		_last_mps_number = _measurement_number;

		// Loop calculation :
		_loopdt = _loopdt_sum / _interval;
		_loopdt_sum = 0.0;

		// Mean measurement calculation :
		_meandt = _meandt_sum / _interval;
		_meandt_sum = 0.0;
	}

	_last_stop = curtime;
}

//============================= end of file ==========================
