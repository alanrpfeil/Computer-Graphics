/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdio.h>
# include <stdlib.h>

# include <sig/gs_matn.h>
# include <sig/gs_output.h>

static void test_lu ( int n )
 {
   GsMatn a(n,n), l, u, lu;

   a.random ( 0, 99.0 );

   ludcmp ( a, l, u );
   gsout << "a :\n" << a;
   gsout << "l :\n" << l;
   gsout << "u :\n" << u;
  
   lu.mult ( l, u );

   gsout << "error : " << dist(a,lu) << gsnl;
 }

static void test_lusolve ( int n )
 {
   GsMatn a(n,n);
   GsMatn b(n,1);

   a.random ( 0, 99.0 );
   b.random ( 0, 99.0 );

   gsout << "a :\n" << a;
   gsout << "b :\n" << b;
  
   GsMatn x(b);
   lusolve ( a, b, x );

   gsout << "x :\n" << x;

   GsMatn b2;
   b2.mult ( a, x );
   gsout << "b2 :\n" << b2;

   gsout << "error : " << dist(b2,b) << gsnl;
 }

static void test_gauss ( int n )
 {
   GsMatn a(n,n);
   GsMatn b(n,1);

   a.random ( 0, 99.0 );
   b.random ( 0, 99.0 );

   gsout << "a :\n" << a;
   gsout << "b :\n" << b;
  
   GsMatn x, b2;
   gauss ( a, b, x );

   gsout << "x :\n" << x;

   b2.mult ( a, x );
   gsout << "b2 :\n" << b2;

   gsout << "error : " << dist(b2,b) << gsnl;
 }

static void test_inverse ( int n )
 {
   GsMatn a(n,n);

   a.random ( 0, 99.0 );

   gsout << "a :\n" << a;

   GsMatn inva(a), id;
   invert ( inva );
   gsout << "inva :\n" << inva;

   id.mult ( a, inva );
   gsout << "a * inva :\n" << id;

   GsMatn real_id(n,n);
   real_id.identity();

   gsout << "error : " << dist(id,real_id) << gsnl;
 }


void test_matn ()
 {
   gsout<<"\nLU:\n"; test_lu (5);
   gsout<<"\nLUSOLVE:\n"; test_lusolve (5);
   gsout<<"\nGAUSS:\n"; test_gauss (5);
   gsout<<"\nINVERSE:\n"; test_inverse (5);
 }

