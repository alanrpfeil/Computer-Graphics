/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_heap.h>
# include <sig/gs_random.h>

typedef GsHeap<int,int> MyHeap;

void print ( MyHeap& h )
 {
   int i, s=h.size();

   for ( i=0; i<s; i++ )
	{
	  gsout << h.elem(i) << " ";
	}
   gsout << gsnl;
 }

void test_heap ()
 {
   int i, s=25;
   GsRandom<int> r ( 0, 99 );
   MyHeap h;

   gsout<<"Heap elements during removal:"<<gsnl;
   while ( s-->0 ) { i=r.get(); h.insert(i,i); }
   while ( h.size()>0 )
	{ print ( h );
	  h.remove();
	}

   gsout<<"Elements in order during removal:"<<gsnl;
   s=10;
   while ( s-->0 ) { i=r.get(); h.insert(i,i); }
   while ( h.size()>0 )
	{ gsout<<h<<gsnl;
	  h.remove();
	}

   s=10;
   while ( s-->0 ) { i=r.get(); h.insert(i,i); }
   gsout<<"Initial heap:"<<gsnl;
   print(h);
   gsout<<"Initial elements in order:"<<gsnl;
   gsout<<h<<gsnl;
   h.size(5);
   gsout<<"After resize:"<<gsnl;
   print(h);
   gsout<<"Elements in order:"<<gsnl;
   gsout<<h<<gsnl;
 }

