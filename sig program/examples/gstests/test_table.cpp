/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_table.h>

void print ( GsTableBase& T )
{
	int i, j;
	const char* st;

	gsout <<"\nHash Table:\n";

	for ( i=0; i<T.hashsize(); i++ )
	{
		gsout << i << ":[";
		st = T.key(i);
		if ( !st ) st = "";
		gsout << st << "]";
		j = i;
		while ( T.collidingid(j)!=-1 )
		{	j = T.collidingid(j);
			gsout << ", " << j << ":[" << T.key(j) << "]";
		}
		gsout << gsnl;
	}

	gsout << "HashSize:"<<T.hashsize()
		  << " Elements:"<<T.elements()
		  << " LongestEntry:"<< T.longest_entry()
		  << " Collisions:"<<T.collisions()<<"\n\n";
}

struct MyData 
{   int x;
	MyData(int i=0):x(i) {}
};

void test_table ()
{
	GsTable<gsintp> TB(40);
   
	TB.insert ( "", 0 );
	TB.insert ( "atest", 0 );
	TB.insert ( "btest", 0 );
	TB.insert ( "ctest", 0 );
	TB.insert ( "abc", 0 );
	TB.insert ( "go now", 0 );
	TB.insert ( "lshoulder", 0 );
	TB.insert ( "lelbow", 0 );
	TB.insert ( "LWrist", 0 );
	TB.insert ( "lwrist", 0 );
	TB.insert ( "rwrist", 0 );
	TB.insert ( "rshoulder", 0 );
	TB.insert ( "relbow", 0 );
	TB.insert ( "l_shoulder", 0 );
	TB.insert ( "l_elbow", 0 );
	TB.insert ( "l_wrist", 0 );
	TB.insert ( "r_wrist", 0 );
	TB.insert ( "r_shoulder", 0 );
	TB.insert ( "r_elbow", 0 );

	print ( TB );

	TB.rehash ( 20 );
	print ( TB );
   
	gsout<<"index of [relbow]: "<<TB.lookup_index("relbow")<<gsnl;
	gsout<<"index of [relbowX]: "<<TB.lookup_index("relbowX")<<gsnl;
   
	TB.remove("ctest");
	TB.remove("abc");
	TB.remove("rshoulder");
	TB.remove("l_shoulder");
	gsout<<"\nremoved [ctest,abc,rshoulder,l_shoulder]: "<<gsnl;
	print ( TB );

	gsout<<"\nInserting back [ctest,abc,rshoulder,l_shoulder]: "<<gsnl;
	TB.insert ( "ctest", 0 );
	TB.insert ( "abc", 0 );
	TB.insert ( "rshoulder", 0 );
	TB.insert ( "l_shoulder", 0 );
	print ( TB );

	gsout<<"\nTemplate version:\n\n";
   
	GsTablePt<MyData> T(5,GsTableBase::ReferencedKeys);
	T.insert ( "d1", new MyData(1) );
	T.insert ( "d2", new MyData(2) );
	T.insert ( "d3", new MyData(3) );

	if ( !T.insert("d3") ) gsout<<"duplication detected!\n";
   
	print ( T );
	gsout << T.lookup("d1")->x << gsnl;
	gsout << T.lookup("d2")->x << gsnl;
	gsout << T.lookup("d3")->x << gsnl;
}

