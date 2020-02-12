/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_GRAPH_H
# define GS_GRAPH_H

/** \file gs_graph.h 
 * Graph maintenance classes
 */

# include <sig/gs_array.h>
# include <sig/gs_list.h>

//================================ GsGraphLink ============================================

class GsGraphNode;
class GsGraphBase;

/*! GsGraphLink contains the minimal needed information for
	a link. To attach user-related information, GsGraphLink
	and GsManager must be derived and properly implemented. */
class GsGraphLink
{  private :
	GsGraphNode* _node;	// node this link is pointing to
	gsuint _index;		// index for traversing
	float _cost;		// cost for minimum-cost paths
	int _blocked;		// used as boolean or as a ref counter
	friend class GsGraphNode;
	friend class GsGraphBase;
   protected :
	GsGraphLink () { _node=0; _index=0; _cost=0; _blocked=0; }
   ~GsGraphLink () {}
   public :
	void cost ( float c ) { _cost=c; }
	float cost () const { return _cost; }
	GsGraphNode* node () const { return _node; }
	gsuint index () const { return _index; }
	int blocked () const { return _blocked; }
	void blocked ( bool b ) { _blocked = b; }
	void blocked ( int b ) { _blocked = b; }
};

//================================ GsGraphNode =============================================

/*! GsGraphNode contains the minimal needed information for
	a node. To attach user-related information, GsGraphLink
	must be derived and as well a corresponding 
	GsManagerBased, which will manage only the user data. */
class GsGraphNode : public GsListNode
{  private :
	GsArray<GsGraphLink*> _links;
	gsuint _index;
	int _blocked; // used as boolean or as a ref counter
	GsGraphBase* _graph;
	friend class GsGraphBase;
   public :
	float fparam; // generic parameter for algorithms

   protected :

	/*! Constructor simply initializes data memebers with null values */
	GsGraphNode ();

	/*! Destructor deletes the outgoing links of this node (if the node is part of a graph) */
   ~GsGraphNode ();

   public :

	/*! Returns the next node in the global list of nodes */
	GsGraphNode* next() const { return (GsGraphNode*) GsListNode::next(); }

	/*! Returns the prior node in the global list of nodes */
	GsGraphNode* prior() const { return (GsGraphNode*) GsListNode::prior(); }

	/*! Returns the index of this node */
	gsuint index () { return _index; }

	int blocked () const { return _blocked; }
	void blocked ( bool b ) { _blocked = b? 1:0; }
	void blocked ( int b ) { _blocked = b; }

	/*! Compress internal links array */
	void compress () { _links.compress(); }

	/*! Returns the new link created */
	GsGraphLink* linkto ( GsGraphNode* n, float cost=0 );

	/*! Remove all links going out of, and coming to, this node */
	void unlink ();

	/*! Remove link of index li, with a fast remove: the order in the
		links array is not mantained */
	void unlink ( int li );

	/*! Remove the link pointing to n. Should be called ONLY when it is
		guaranteed that the link to n really exists! Uses method unlink(ni) */
	void unlink ( GsGraphNode* n ) { unlink ( search_link(n) ); }

	/*! Returns the index in the links array, or -1 if not linked to n.
		The search starts from the latest link inserted, optimizing many dynamic scenarios. */
	int search_link ( GsGraphNode* n ) const;

	/*! Returns the link pointing to n. Should be called ONLY when it is
		guaranteed that the link to n really exists! */
	GsGraphLink* link ( GsGraphNode* n ) { return _links[search_link(n)]; }

	/*! Returns the link index i, which must be a valid index */
	GsGraphLink* link ( int i ) { return _links[i]; }

	/*! Returns the last link */
	GsGraphLink* last_link () { return _links.top(); }

	/*! Returns the number of links in this node */
	int num_links () const { return _links.size(); }
	int nlinks () const { return _links.size(); }

	/*! Returns the links array */
	const GsArray<GsGraphLink*>& links() const { return _links; }

   private :
	void output ( GsOutput& o ) const;
};

//================================ GsGraphBase ==============================================

/*! The following define is to be used inside a user derived class
	of GsGraphLink, in order to easily redefine public GsGraphLink
	methods with correct type casts. */
# define GS_GRAPH_LINK_CASTED_METHODS(N,L) \
N* node() const { return (N*)GsGraphLink::node(); }

/*! The following define is to be used inside a user derived class
	of GsGraphNode, in order to easily redefine public GsGraphNode
	methods with correct type casts. */
# define GS_GRAPH_NODE_CASTED_METHODS(N,L) \
N* next() const { return (N*)GsListNode::next(); } \
N* prior() const { return (N*)GsListNode::prior(); } \
L* linkto ( N* n, float c ) { return (L*)GsGraphNode::linkto(n,c); } \
L* link ( N* n ) { return (L*)GsGraphNode::link(n); } \
L* link ( int i ) { return (L*)GsGraphNode::link(i); } \
const GsArray<L*>& links() const { return (const GsArray<L*>&) GsGraphNode::links(); }

class GsGraphPathTree;

/*! GsGraphBase maintains a directed graph with nodes and links. Links around
	a node do not have any order meaning.
	The user should avoid redundant links, as no tests to detect redunduncy are included
	(for efficiency) in several methods.
	GsGraphBase is the base class, the user should use GsGraph template instead. */
class GsGraphBase
{  private :
	GsList<GsGraphNode> _nodes;
	GsArray<GsGraphNode*> _buffer;
	mutable gsuint _curmark;
	mutable char _mark_status;
	GsGraphPathTree* _pt;
	GsManagerBase* _lman; // link manager for a class deriving GsGraphLink
	mutable gscenum _leave_indices_after_save;

   public :
	/*! Constructor requires managers for nodes and links */
	GsGraphBase ( GsManagerBase* nm, GsManagerBase* lm );

	/*! Destructor deletes all data in the graph */
   ~GsGraphBase ();

	/*! Returns a pointer to the node manager */
	GsManagerBase* node_class_manager() const { return _nodes.class_manager(); }

	/*! Returns a pointer to the link manager */
	GsManagerBase* link_class_manager() const { return _lman; }

	/*! Make the graph empty */
	void init ();

	/*! Compress all link arrays in the nodes */
	void compress ();

	/*! Returns the number of nodes in the graph */
	int num_nodes () const { return _nodes.elements(); }

	/*! Counts and returns the number of (directional) links in the graph */
	int num_links () const;

	/*! Methods for marking nodes and links */
	void begin_marking () const;
	void end_marking () const;
	bool marked ( GsGraphNode* n ) const;
	void mark ( GsGraphNode* n ) const;
	void unmark ( GsGraphNode* n ) const;
	bool marked ( GsGraphLink* l ) const;
	void mark ( GsGraphLink* l ) const;
	void unmark ( GsGraphLink* l ) const;

	/*! Methods for indexing nodes and links */
	void begin_indexing () const;
	void end_indexing () const;
	gsuint index ( GsGraphNode* n ) const;
	void index ( GsGraphNode* n, gsuint i ) const;
	gsuint index ( GsGraphLink* l ) const;
	void index ( GsGraphLink* l, gsuint i ) const;

	/*! Returns the list of nodes, that should be used with
		care to not invalidate some GsGraph operations. */
	GsList<GsGraphNode>& nodes () { return _nodes; }

	/*! Inserts node n in the list of nodes, as a new
		unconnected component. n is returned. */
	GsGraphNode* insert ( GsGraphNode* n );

	/*! Extract (without deleting) the node from the graph. Nothing
		is done concerning possible links between the graph and
		the node being extracted */
	GsGraphNode* extract ( GsGraphNode* n );

	/*! Removes and deletes the node from the graph. Its the user
		responsibility to ensure that the graph will not have links
		pointing to the node being deleted. The destructor of the
		node being deleted will delete all outgoing links of the node */
	void remove_node ( GsGraphNode* n );

	/*! Searches and removes the edge(s) linking n1 with n2 if any.
		Links are removed with a "fast remove process" so that indices
		in the links arrays of the involved nodes may be modified.
		Returns the number of (directed) links removed. */
	int remove_link ( GsGraphNode* n1, GsGraphNode* n2 );

	/*! Links n1 to n2 and n2 to n1, with cost c in both directions */
	void link ( GsGraphNode* n1, GsGraphNode* n2, float c=0 );

	/*! Returns the first node of the list of nodes. Note: the list is a circular
		list and the prior/next methods of GsListNode can also be used. */
	GsGraphNode* first_node () const { return _nodes.first(); }

	/*! Get all directed edges of the graph. Note that if n1 is linked
		to n2 and n2 is linked to n1, both edges (n1,n2) and (n2,n1) will
		appear in the edges array. Every two consecutive nodes in the returned
		array represents one edge. Array links, if given, returns the links. */
	void get_directed_edges ( GsArray<GsGraphNode*>& edges, GsArray<GsGraphLink*>* links=0 );

	/*! Get all edges of the graph without duplications from the directional
		point of view. Array links, if valid pointer is given, returns the links. */
	void get_undirected_edges ( GsArray<GsGraphNode*>& edges, GsArray<GsGraphLink*>* links=0 );

	/*! Get all nodes which are in the same connected component of source */
	void get_connected_nodes ( GsGraphNode* source, GsArray<GsGraphNode*>& nodes );

	/*! Organize nodes by connected components. The indices in array components give
		pairs of start and end positions in array nodes, for each component. */
	void get_disconnected_components ( GsArray<int>& components, GsArray<GsGraphNode*>& nodes );

	/*! Computes and returns in array path the shortest path between n1 and n2.
		In the case the two nodes are in two different disconnected components
		an empty path is returned. If n1==n2 a path with the single node n1
		is returned. In all cases, if cost is null-non it will receive the path cost.
		If links is non-null, it will contain the links between the path nodes.
		If hdistf is provided, this method performs A*; otherwise Dijkstra.
		In case no path is found and the optional parameter gdistf is given,
		the path to the closest processed node to the goal is returned.
		Parameters hdistf and gdistf may be the same. */
	bool shortest_path ( GsGraphNode* n1, GsGraphNode* n2, GsArray<GsGraphNode*>& path,
						GsArray<GsGraphLink*>* links=0, float* cost=0,
						float (*hdistf) ( const GsGraphNode*, const GsGraphNode*, void* udata )=0,
						float (*gdistf) ( const GsGraphNode*, const GsGraphNode*, void* udata )=0,
						void* udata=0 );

	/*! Performs a breadth-first search from n1 until finding n2.
		Parameter cost, if non-null, will point to the path cost.
		The search stops if either maxdepth or maxcost is reached, but only
		if these parameters are given with positive values.
		True is returned if a path is returned. */
		// method not yet tested
	bool bfs ( GsGraphNode* startn, GsGraphNode* endn, GsArray<GsGraphNode*>& path,
			   GsArray<GsGraphLink*>* links=0, float* cost=0, int maxdepth=-1, float maxcost=-1 );

	/*! When set to true, graph search will consider a link blocked if any of its
		directions is blocked, i.e., blocking only one direction of a link will
		block both the directions. Afftected methods are get_short_path() and
		local_search(). Default is false. */
	void bidirectional_block_test ( bool b );

	/*! If this is set to true, it will be the user responsibility to call
		end_indexing() after the next graph save. It can be used to retrieve the indices
		used during saving in order to reference additional data to be saved in derived classes. */
	void leave_indices_after_save ( bool b ) { _leave_indices_after_save = (gscenum)b; }

	/*! Returns the internal buffer, which might be usefull for some specific needs;
		see for instance the description of the input() method. */
	GsArray<GsGraphNode*>& buffer () { return _buffer; }

	/*! Outputs the graph in the format: [ (l1..lk)e1 (..)e2 (..)en ].
		Nodes are indexed (starting from 0), and after the output
		all indices of the nodes are set to 0. */
	void output ( GsOutput& o ) const;

	/*! Initializes the current graph and load another one from the given input.
		Method buffer() can be used to retrieve an array with all nodes loaded,
		indexed by the indices used in the loaded file. */
	void input ( GsInput& i );

	/*! Output operator */
	friend GsOutput& operator<< ( GsOutput& o, const GsGraphBase& g ) { g.output(o); return o; }

	/*! Input operator */
	friend GsInput& operator>> ( GsInput& i, GsGraphBase& g ) { g.input(i); return i; }

   private :
	void _normalize_mark() const;
};

//================================ GsGraph =================================================

/*! GsGraph is a template that includes type casts for the user
	derived types (of GsGraphNode and GsGraphLink) to correctly
	call GsGraphBase methods. */
template <class N, class L>
class GsGraph : public GsGraphBase
{ public :
	GsGraph () : GsGraphBase ( new GsManager<N>, new GsManager<L> ) {}

	GsGraph ( GsManagerBase* nm, GsManagerBase* lm ) : GsGraphBase(nm,lm) {}

	const GsList<N>& nodes () { return (GsList<N>&)GsGraphBase::nodes(); }

	N* insert ( N* n ) { return (N*) GsGraphBase::insert((GsGraphNode*)n); }
	N* extract ( N* n )  { return (N*) GsGraphBase::extract((GsGraphNode*)n); }
	N* first_node () const { return (N*) GsGraphBase::first_node(); }

	void get_directed_edges ( GsArray<N*>& edges, GsArray<L*>* links=0 )
	{	GsGraphBase::get_directed_edges( (GsArray<GsGraphNode*>&)edges, (GsArray<GsGraphLink*>*)links ); }

	void get_undirected_edges ( GsArray<N*>& edges, GsArray<L*>* links=0 )
	{	GsGraphBase::get_undirected_edges( (GsArray<GsGraphNode*>&)edges, (GsArray<GsGraphLink*>*)links ); }

	void get_disconnected_components ( GsArray<int>& components, GsArray<N*>& nodes )
	{	GsGraphBase::get_disconnected_components( components, (GsArray<GsGraphNode*>&)nodes ); }

	bool shortest_path ( N* n1, N* n2, GsArray<N*>& path, GsArray<L*>* links, float* cost=0,
						float (*hdistf) ( const GsGraphNode*, const GsGraphNode*, void* udata )=0,
						float (*gdistf) ( const GsGraphNode*, const GsGraphNode*, void* udata )=0,
						void* udata=0  )
	{	return GsGraphBase::shortest_path (
				(GsGraphNode*)n1, (GsGraphNode*)n2,
				(GsArray<GsGraphNode*>&)path, (GsArray<GsGraphLink*>*)links,
				cost, hdistf, gdistf, udata );
	}

	GsArray<N*>& buffer () { return (GsArray<N*>&) GsGraphBase::buffer(); }

	friend GsOutput& operator<< ( GsOutput& o, const GsGraph& g ) { return o<<(GsGraphBase&)g; }
	friend GsInput& operator>> ( GsInput& i, GsGraph& g ) { return i>>(GsGraphBase&)g; }
};

//================================ End of File =================================================

# endif  // GS_GRAPH_H
