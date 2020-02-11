/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>
# include <sig/gs_mat.h>
# include <sig/gs_timer.h>
# include <sig/gs_random.h>

static GsVec rvec ()
 {
   static GsRandom<float> r (-9.99f,9.99f);
   GsVec v ( r.get(), r.get(), r.get() );
   return v; 
 }

static char rfmt ()
 {
   static const char* fmt = "LC";
   return fmt[gs_random(0,1)];
 }

static void test1 ()
 {
   char fmt;
   int i, times=100000;
   GsMat m, mr, mt;
   GsVec v, v1, v2, v3;
   float l1, l2, l3;

   gsout << times <<" random calls for each test:\n";

   gsout << "Testing lcombtrans... ";
   for ( i=0; i<times; i++ )
	{ v1=rvec(); v2=rvec(); v=rvec();
	  fmt=rfmt();
	  mr.rot(v1,v2);
	  mt.translation(v);
	  m = mt * mr;
	  mr.lcombtrans(v);
	  if ( !next(m,mr,gstiny) ) { gsout<<"lcombtrans ERROR!\n"; break; }
	}
   gsout << "Ok.\n";
   
   gsout << "Testing rcombtrans... ";
   for ( i=0; i<times; i++ )
	{ v1=rvec(); v2=rvec(); v=rvec();
	  fmt=rfmt();
	  mr.rot(v1,v2);
	  mt.translation(v);
	  m = mr * mt;
	  mr.rcombtrans(v);
	  if ( !next(m,mr,gstiny) ) { gsout<<"rcombtrans ERROR!\n"; break; }
	}
   gsout << "Ok.\n";

   gsout << "Testing projxy... ";
   for ( i=0; i<times; i++ )
	{ v1=rvec(); v2=rvec(); v3=rvec();
	  l1 = dist(v1,v2);
	  l2 = dist(v2,v3);
	  l3 = dist(v3,v1);
	  fmt=rfmt();
	  m.projxy(v1,v2,v3);
	  if ( fmt=='C' )
	   { v1=v1*m; v2=v2*m; v3=v3*m; }
	  else
	   { v1=m*v1; v2=m*v2; v3=m*v3; }

	  if ( !next(v1,GsVec::null,gsmall) ||
		   !next(v2,GsVec(v2.x,0.0f,0.0f),gsmall) ||
		   !next(v3,GsVec(v3.x,v3.y,0.0f),gsmall) ) { gsout<<"projxy ERROR!\n"; break; }
	
	  if ( gs_dist(l1,dist(v1,v2))>gsmall ||
		   gs_dist(l2,dist(v2,v3))>gsmall ||
		   gs_dist(l3,dist(v3,v1))>gsmall ) { gsout<<"projxy dist ERROR!\n"; break; }

	}
   gsout << "Ok.\n";
 }

# define MAX 500

static void test2 ()
 {
   GsMat m ( 2, 3, 4, 5, 
			 6, 7, 8, 9,
			 1, 2, 3, 4, 
			 5, 6, 7, 8 );

   int i, j;
   GsTimer t;
   float d, dmax=0;
   GsMat minv, p, pmax(GsMat::id);

   t.start ();
   for ( i=0; i<MAX; i++ )
	{ for ( j=0; j<15; j++ ) m[j] = gs_random(-1.0f,1.0f);
	  m.inverse ( minv );
	  p.mult ( m, minv );
	  d = dist2 ( p, GsMat::id );
	  if ( d>dmax ) { dmax=d; pmax=m; }
	}
   t.stop ();
   gsout << "GsMat time for " << MAX << " inversions: " << t.dt() << gsnl;
   gsout << "Max error: " << float(sqrtf(dmax)) << gsnl;
   
   pmax.inverse(minv);
   gsout << gsnl << pmax << gsnl;
   gsout << minv << gsnl;

   GsMat mid(pmax*minv);
   for ( i=0; i<16; i++ ) mid[i]=gs_round(mid[i],0.0001f);

   gsout << mid << gsnl;
 }

void test_mat ()
 {
   test1();
   test2();
 }

