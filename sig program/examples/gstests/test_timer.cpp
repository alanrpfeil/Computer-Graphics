/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_output.h>
# include <sig/gs_timer.h>

void test_timer ()
{
	GsTimer timer(0);

	for ( int c=0; c<5; c++ )
	{	gsout << "Time Now: " << timer.t() << gsnl;
	}

	gsout << "\n5 one second ticks using GsTimer:\n";
	for ( int i=1; i<=5; i++ )
	{	int c=0;
		timer.start();
		do { c++; timer.stop(); } while ( timer.dt()<1.0 );
		gsout << i <<gspc<< c << " loops, dt=" << timer.dt() <<gsnl;
	}

	gsout << "\n5 one second ticks using gs_time():\n";
	for ( int i=1; i<=5; i++ )
	{	int c=0;
		double t0 = gs_time();
		do { c++; } while ( gs_time()-t0<1.0 );
		gsout << i <<gspc<< c << " loops, dt=" << (gs_time()-t0) <<gsnl;
	}
}
