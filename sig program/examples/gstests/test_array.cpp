/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <string.h>

# include <sig/gs_string.h>
# include <sig/gs_random.h>
# include <sig/gs_array.h>
# include <sig/gs_time.h>

/* Attention: we use this Buf class because GsArray cannot be used with classes having
   internal dynamic memory management. For arrays of strings, use GsStrings. */
struct Buf
{	char st[80];
	Buf ( const char *s ) { strcpy(st,s); }
	void operator = ( const Buf &b ) { strcpy(st,b.st); }
	friend GsOutput& operator<< ( GsOutput& o, const Buf &b ) { return o<<(char*)b.st; }
	friend GsInput& operator>> ( GsInput& i, Buf &b ) { return i>>b.st; }
	static inline int compare  ( const Buf *b1, const Buf *b2 ) { return ::gs_compare((char*)b1->st,(char*)b2->st); }
};

static void run_example1 ()
{
	GsArray<Buf> a;
	gsout << "Empty:  " << a << '\n';

	a.push() = Buf("The");
	a.push() = Buf("book");
	a.push() = Buf("IS");
	a.push() = Buf("on");
	a.push() = Buf("the");
	a.push() = Buf("table");
	gsout << "Push:   " << a << '\n';

	a.sort (Buf::compare);
	gsout << "Sorted: " << a << '\n';

	a.insort ( Buf("inserted1"), Buf::compare );
	a.insort ( Buf("inserted2"), Buf::compare );
	a.insort ( Buf("aaa"), Buf::compare );
	a.insort ( Buf("uuu"), Buf::compare );
	gsout << "Insort: " << a << '\n';

	GsString buf;
	GsOutput o; o.init(buf);
	o << a;
	gsout << "StrOut: " << buf << '\n'; 
  
	GsArray<Buf> b;
	GsInput in; in.init(buf);
	in >> b;
	gsout << "StrInp: " << b << '\n'; 

	GsArray<Buf> c=b;
	gsout << "Copy:   " << c << '\n';
}

// example2 shows an example of sorting an array of pointers:
struct Node
{	int i;
	Node ( int a ) { i=a; }
	friend GsOutput& operator<< ( GsOutput& o, const Node *n ) { return o<<n->i; }
	friend int cmpf ( Node *const* a, Node *const* b );
};

int cmpf ( Node *const* a, Node *const* b )
{
	Node* n1 = *a;
	Node* n2 = *b;
	return n1->i - n2->i;
}

static void run_example2 ()
{
	GsArray<Node*> ptarray;
	GsRandom<int> r (0,100);

	int tot=20;

	gsout<<"Example with sort():\n";
	int n=tot;
	while ( n-- ) ptarray.push() = new Node(r.get());
	ptarray.sort ( cmpf );
	gsout<<ptarray<<gsnl;
	while ( ptarray.size()>0 ) delete ptarray.pop();

	gsout<<"Example with insort():\n";
	n=tot;
	while ( n-- ) ptarray.insort( new Node(r.get()), cmpf );
	gsout<<ptarray<<gsnl;
	while ( ptarray.size()>0 ) delete ptarray.pop();
}

# include <vector>
# include <sig/gs_mat.h>
static void run_example3 ()
{
	GsArray<float> af; //af.size(10);
	std::vector<float> vf; //vf.resize(10);

	gsout<<(gsuint)sizeof(af)<<gsnl;
	gsout<<(gsuint)sizeof(vf)<<gsnl;
}

static void run_example4 ()
{
	int n=50000, di=10;

	double t1 = gs_time();
	GsArray<GsMat> a; a.size(n);
	for ( int i=0; i<n; i+=di ) a.insert(i)=GsMat::id;
	double t2 = gs_time();

	double t3 = gs_time();
	std::vector<GsMat> v; v.resize(n);
	for ( int i=0; i<n; i+=di ) v.insert(v.begin()+i,GsMat::id);
	double t4 = gs_time();

	gsout<<"GsArray time: "<<(t2-t1)<<" secs"<<gsnl;
	gsout<<"std::vector : "<<(t4-t3)<<" secs"<<gsnl;
}

void test_array ()
{
	run_example1 ();
	run_example2 ();
	if (0) run_example3 ();
	run_example4 ();
}
