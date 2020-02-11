/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_vec2.h>
# include <sig/gs_color.h>
# include <sig/gs_graph.h>
# include <sig/gs_string.h>

class MyNode;

class MyLink : public GsGraphLink
{  public :
	GsString s;
   public :
	GS_GRAPH_LINK_CASTED_METHODS(MyNode,MyLink);

	MyLink () { s="link"; }
	MyLink ( const MyLink& l ) : GsGraphLink(), s(l.s) {}
   ~MyLink () {}

	friend GsOutput& operator<< ( GsOutput& out, const MyLink& l )
	{	return out<<'"'<<l.s<<'"'; }

	friend GsInput& operator>> ( GsInput& inp, MyLink& l )
	{	return inp>>l.s; }

	static inline int compare ( const MyLink* l1, const MyLink* l2 )
	{	return 0; }
};

class MyNode : public GsGraphNode
{  public :
	GsString s;
   public :
	GS_GRAPH_NODE_CASTED_METHODS(MyNode,MyLink);

	MyNode () : GsGraphNode() { s="node"; }
	MyNode ( const MyNode& n ) : GsGraphNode(), s(n.s) {}
	MyNode ( const char* st ) : GsGraphNode() { s="node"; s<<" "; s<<st; }
   ~MyNode () {}

	friend GsOutput& operator<< ( GsOutput& out, const MyNode& n )
	{	return out<<'\"'<<n.s<<'\"'; }

	friend GsInput& operator>> ( GsInput& inp, MyNode& n )
	{	return inp>>n.s; }

	static inline int compare ( const MyNode* n1, const MyNode* n2 )
	{	return 0; } // not used
};

typedef GsGraph<MyNode,MyLink> MyGraph;

static void run ()
 {
   MyGraph g;

   /*MyNode* n0 =*/ g.insert ( new MyNode("n0") ); // n0 is not used in this example
   MyNode* n1 = g.insert ( new MyNode("n1") );
   MyNode* n2 = g.insert ( new MyNode("n2") );
   MyNode* n3 = g.insert ( new MyNode("n3") );

   g.link ( n1, n2, 12 );
   n1->link(n2)->s << " 12";
   n1->linkto ( n3, 13 )->s << " 13";

   gsout<<g<<gsnl;

   GsListIterator<MyNode> it ( g.first_node() );
   for ( it.first(); it.inrange(); it.next() ) gsout<<it->s<<gsnl;

   gsout<<"\nReverse order:"<<gsnl;
   for ( it.last(); it.inrange(); it.prior() ) gsout<<it->s<<gsnl;
 }

void test_graph ()
 {
   run ();
 }
