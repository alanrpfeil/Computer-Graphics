/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_string.h>
# include <sig/gs_slot_map.h>

class MyData
{  public :
	GsString s;
   public :
	MyData () {}
	MyData ( const MyData& d ) { s=d.s; }
   ~MyData () {}
	friend GsOutput& operator<< ( GsOutput& out, const MyData& d ) { return out<<d.s; }
	friend GsInput& operator>> ( GsInput& inp, MyData& d ) { return inp>>d.s; }
	static inline int compare ( const MyData* d1, const MyData* d2 ) { return GsString::compare(d1->s,d2->s); }
};

static void runex1 ()
{
	GsSlotMap<MyData> sm;
	int i, id;
	for ( i=0; i<10; i++ ) sm[sm.insert()]->s.setf("E%d",i);

	gsout << "Orig: " << sm << gsnl;

	sm.remove ( 2 );
	sm.remove ( 6 );
	sm.remove ( 8 );
	sm.remove ( 7 );
	gsout << "Rem 2,6,8,7: " << sm << gsnl;

	sm.remove ( 9 );
	gsout << "Rem 9   : " << sm << gsnl;

	GsSlotMap<MyData> copysm(sm); // for later on tests

	id=sm.insert(); sm[id]->s="xx";
	gsout << "Ins xx  : " << sm << gsnl;

	GsArray<int> newids;
	sm.remove_gaps(newids);
	gsout << "Rem gaps: " << sm << gsnl;
	gsout << "New ids : " << newids << gsnl;

	// copyset tests
	GsString buf;
	GsOutput out; out.init( buf );
	out << copysm;
	GsInput inp; inp.init(buf);
	inp.lowercase(false);
	inp >> sm;

	gsout << gsnl << "I/O test:" << gsnl;
	gsout << copysm << " maxid=" << copysm.maxid() << gsnl;   
	gsout << sm << " maxid=" << sm.maxid() << gsnl;   

	gsout << gsnl << "Remove 0 5:" << gsnl;
	sm.remove(0);
	sm.remove(5);
	gsout << sm << " maxid=" << sm.maxid() << gsnl;   

	gsout << gsnl << "Traversing elements:" << gsnl;
	for ( i=0; i<=sm.maxid(); i++ )
		if (sm[i]) gsout<< *sm[i] << gspc;
	gsout<<gsnl;
	for ( i=sm.maxid(); i>=0; i-- )
		if (sm[i]) gsout<< *sm[i] << gspc;
	gsout<<gsnl;
}

void test_slotmap ()
{
	runex1 ();
}
