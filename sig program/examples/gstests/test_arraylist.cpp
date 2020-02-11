/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <vector>
using namespace std;

# include <string.h>

# include <sig/gs_timer.h>
# include <sig/gs_array.h>
# include <sig/gs_list.h>
# include <sig/gs_tree.h>
# include <sig/gs_input.h>
# include <sig/gs_output.h>
# include <sig/gs_random.h>

class Buf : public GsListNode
{  public :
	int i;
	Buf ( int x ) { i=x; }
	Buf () { i=0; }
	void operator = ( const Buf &b ) { i=b.i; }
	friend GsOutput& operator<< ( GsOutput& o, const Buf &b ) { return o<<b.i; }
	friend GsInput& operator>> ( GsInput& i, Buf& b ) { return i>>(b.i); }
	static inline int compare  ( const Buf *b1, const Buf *b2 ) { return ::gs_compare(&b1->i,&b2->i); }
};

class Tbuf : public GsTreeNode
{  public :
	int i;
	Tbuf ( int x ) { i=x; }
	Tbuf () { i=0; }
	void operator = ( const Tbuf &b ) { i=b.i; }
	friend GsOutput& operator<< ( GsOutput& o, const Tbuf &b ) { return o<<b.i; }
	friend GsInput& operator>> ( GsInput& i, Tbuf& b ) { return i>>(b.i); }
	static inline int compare  ( const Tbuf *b1, const Tbuf *b2 ) { return ::gs_compare(&b1->i,&b2->i); }
};

# define SIZE 5000

static void run ()
{
	GsArray<Buf> a;
	GsList<Buf>  l;
	GsTree<Tbuf>  tree;
	vector<Buf> v;

	gsout << "Time required for different structure manipulations\n";
	gsout << "Manipulating " << SIZE << " elements\n\n";

	int i;
	double t1, t2, t3;
	GsTimer t;
	GsRandom<int> r(0,999999);

	// create a list of random ints:
	GsArray<int> ints;
	for ( i=0; i<SIZE; i++ ) ints.push()=r.get();
	//gsout<<ints<<gsnl;

	t.start();
	for ( i=0; i<SIZE; i++ ) a.insort(ints[i],Buf::compare);
	t.stop(); t1=t.dt();
	t.start();
	for ( i=0; i<SIZE; i++ ) l.insort(new Buf(ints[i]));
	t.stop(); t2=t.dt();
	t.start();
	for ( i=0; i<SIZE; i++ ) tree.insert(new Tbuf(ints[i]));
	t.stop(); t3=t.dt();
	//gsout<<tree<<gsnl;

	gsout << "1. Insertion: Array: "<<t1<<"  List: "<<t2<< "  Tree: "<<t3<<'\n';

	t.start();
	a.sort (Buf::compare);
	t.stop(); t1=t.dt();
	t.start();
	l.sort ();
	t.stop(); t2=t.dt();
	gsout << "2. Sort:      Array: "<<t1<<"  List: "<<t2<< '\n';

	t.start();
	for ( i=0; i<SIZE; i++ ) a.remove(0);
	t.stop(); t1=t.dt();
	t.start();
	for ( i=0; i<SIZE; i++ ) delete l.pop_back();
	t.stop(); t2=t.dt();
	t.start(); // recall that the tree will not have duplications
	for ( i=0; i<SIZE; i++ ) if ( tree.root()!=GsTreeNode::null) tree.remove(tree.root());
	t.stop(); t3=t.dt();
	gsout << "3. Deletion:  Array: "<<t1<<"  List: "<<t2<< "  Tree: "<<t3<<'\n';

	Buf b(1);
	a.capacity(0);
	t.start();
	for ( i=0; i<SIZE; i++ ) a.push(b);
	t.stop(); t1=t.dt();
	t.start();
	for ( i=0; i<SIZE; i++ ) v.push_back(b);
	t.stop(); t2=t.dt();

	gsout << "4. Insertion: GsArray: "<<t1<<"  stl vector: "<<t2<<'\n';
	gsout << "sizeof: " << (gsuint)sizeof(a) << " " << (gsuint)sizeof(v) << gsnl;
}

void test_arraylist ()
{
	run ();
}

/* SIZE==5000, Impact, with a.remove(0) :
1. Insertion: Array: 0.010340 List: 0.016021 ! even with array reallocs
2. Sort: Array: 0.083514 List: 5.082449
3. Deletion: Array: 2.794826 List: 0.008282
*/

/* SIZE==5000, Impact, with a.remove(i<2000? 0:1900) :
1. Insertion: Array: 0.009994 List: 0.016544 ! even with array reallocs
2. Sort: Array: 0.081935 List: 4.956194
3. Deletion: Array: 1.957149 List: 0.007001
*/
