/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_string.h>
# include <sig/gs_output.h>

static void runex1 ()
 {
   GsString a ( "The book " );
   GsString b; b.setf( "is over the %s", "table." );

   a<<b;
   gsout << a <<gsnl;

   a.replace ( "over", "on" );
   a.replace ( "bookx", "Get" );
   a.replace ( "TABLE", "chair" );
   a.replace ( ".", "!" );
   gsout << a <<gsnl;

   GsString x; x << a << " End.\n";
   gsout << x << gsnl;
 }

void test_string ()
 {
   runex1 ();
 }
