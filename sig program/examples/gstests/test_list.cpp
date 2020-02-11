/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_string.h>
# include <sig/gs_list.h>

class MyData : public GsListNode
{  public :
	GsString s;
   public :
	MyData () : GsListNode() {}
	MyData ( const char* st ) : GsListNode() { s=st; }
	MyData ( const MyData& d ) : GsListNode() { s=d.s; }
   ~MyData () {}
	friend GsOutput& operator<< ( GsOutput& out, const MyData& c ) { return out<<c.s; }
	friend GsInput& operator>> ( GsInput& inp, MyData& c ) { return inp>>c.s; }
	static inline int compare ( const MyData* c1, const MyData* c2 ) { return ::gs_compare(c1->s,c2->s); }
};

static void run ()
 {
   GsList<MyData> l;

   l.insert_next ( new MyData("E1") );
   l.insert_next ( new MyData("E5") );
   l.insert_next ( new MyData("E3") );
   l.sort();
   gsout<<l<<gsnl;

   { GsList<MyData> nl(l);
	 nl.insort ( new MyData("E2") );
	 nl.insort ( new MyData("E4") );
	 gsout<<nl<<gsnl;
	 l = nl;
   }

   GsListIterator<MyData> it(l);
   for ( it.first(); it.inrange(); it.next() ) gsout<<it->s<< ' ';
   for ( it.last(); it.inrange(); it.prior() ) gsout<<it->s<< ' ';
   gsout<<gsnl;

   MyData key("E1");
   l.search(&key); l.remove();
   it.reset();
   for ( it.first(); it.inrange(); it.next() ) gsout<<it->s<< ' ';
   for ( it.last(); it.inrange(); it.prior() ) gsout<<it->s<< ' ';
   gsout<<gsnl;

   l.push_front ( new MyData("F1") );
   l.push_front ( new MyData("F2") );
   l.push_back ( new MyData("B1") );
   l.push_back ( new MyData("B2") );
   gsout<<l<<gsnl;

   delete l.pop_front ();
   delete l.pop_back ();
   gsout<<l<<gsnl;
 }

void test_list ()
 {
   run ();
 }
