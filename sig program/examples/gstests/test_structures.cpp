/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <string.h>

# include <sig/gs_random.h>
# include <sig/gs_string.h>
# include <sig/gs_strings.h>
# include <sig/gs_array.h>
# include <sig/gs_queue.h>
# include <sig/gs_graph.h>
# include <sig/gs_list.h>
# include <sig/gs_tree.h>
# include <sig/gs_slot_map.h>

class MyListNode : public GsListNode
{  public :
	GsString s;
   public :
	MyListNode () : GsListNode() {};
	MyListNode ( const MyListNode& l ) : GsListNode() { s=l.s; }
	MyListNode ( const GsString& st ) : GsListNode() { s=st; }
   ~MyListNode () {}
	friend GsOutput& operator<< ( GsOutput& o, const MyListNode& l ) { return o<<l.s; }
	friend GsInput& operator>> ( GsInput& i, MyListNode& l ) { return i>>(l.s); }
	static inline int compare ( const MyListNode* a, const MyListNode* b ) { return GsString::compare(&a->s,&b->s); }
};

class MyTreeNode : public GsTreeNode
{  public :
	GsString s;
   public :
	MyTreeNode () : GsTreeNode() {}
	MyTreeNode ( const MyTreeNode& n ) : GsTreeNode() { s=n.s; }
	MyTreeNode ( const GsString& st ) : GsTreeNode() { s=st; }
   ~MyTreeNode () {}
	friend GsOutput& operator<< ( GsOutput& o, const MyTreeNode& n ) { return o<<n.s; }
	friend GsInput& operator>> ( GsInput& i, MyTreeNode& n ) { return i>>(n.s); }
	static inline int compare ( const MyTreeNode* a, const MyTreeNode* b ) { return GsString::compare(&a->s,&b->s); }
};

class MyGraphNode : public GsGraphNode
{  public :
	GsString s;
   public :
	MyGraphNode () : GsGraphNode() {}
	MyGraphNode ( const MyGraphNode& n ) : GsGraphNode() { s=n.s; }
	MyGraphNode ( const GsString& st ) : GsGraphNode() { s=st; }
   ~MyGraphNode () {}
	friend GsOutput& operator<< ( GsOutput& o, const MyGraphNode& n ) { return o<<n.s; }
	friend GsInput& operator>> ( GsInput& i, MyGraphNode& n ) { return i>>(n.s); }
	static inline int compare ( const MyGraphNode* a, const MyGraphNode* b ) { return GsString::compare(&a->s,&b->s); }
};

class MyGraphLink : public GsGraphLink
{  public :
	friend GsOutput& operator<< ( GsOutput& o, const MyGraphLink& l ) { return o; }
	friend GsInput& operator>> ( GsInput& i, MyGraphLink& l ) { return i; }
	static inline int compare ( const MyGraphLink* a, const MyGraphLink* b ) { return 1; } // not used
};

/* We use this MyBuf class as GsArray and GsQueue cannot be used with classes having
   internal dynamic memory managements. Another option would be to use
   GsArray<GsString*> to have an array of pointers and alloc and delete each entry
   in the array manually. */
struct MyBuf 
{  char buf[20]; 
   void operator= ( const MyBuf& b2 ) { strcpy(buf,b2.buf); }
   friend GsOutput& operator<< ( GsOutput& o, const MyBuf& b ) { return o<<b.buf; }
   static inline int compare ( const MyBuf* a, const MyBuf* b ) { return ::gs_compare(a->buf,b->buf); }
};

GsList<MyListNode>	list;
GsTree<MyTreeNode>	tree;
GsGraph<MyGraphNode,MyGraphLink>  graph;
GsSlotMap<GsString>	smap;
GsArray<MyBuf>		array;
GsQueue<MyBuf>		queue;
GsStrings			sarray;

static void out ()
{
   gsout << "TREE:    " << tree << gsnl;
   gsout << "LIST:    " << list << gsnl;
   gsout << "SLOTMAP: " << smap << gsnl;
   gsout << "ARRAY:   " << array << gsnl;
   gsout << "QUEUE:   " << queue << gsnl;
   gsout << "STARRAY: " << sarray << gsnl;
   gsout << "GRAPH:   " << graph << gsnl;
}

void test_structures ()
{
	GsString s;
	for ( int i=0; i<6; i++ ) 
	{	int r = gs_random ( 15, 85 );
		s.setf("A%02d",r); array.push(); strcpy(array.top().buf,s);
		s.setf("Q%02d",r); queue.insert(); strcpy(queue.last().buf,s);
		s.setf("S%02d",r); smap.insert ( new GsString(s) );
		s.setf("L%02d",r); list.insert_next ( new MyListNode(s) );
		s.setf("G%02d",r); graph.insert ( new MyGraphNode(s) );
		s.setf("T%02d",r); tree.insert_or_del ( new MyTreeNode(s) );
		s.setf("S%02d",r); sarray.push ( s );
	}

	out();

	array.sort(MyBuf::compare);
	sarray.sort();
	list.sort();
	queue.flush();
	queue.compress();
   
	gsout << "\nAfter sorting arrays and list:\n"; 
	out ();
}
