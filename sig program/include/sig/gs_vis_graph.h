/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_VIS_GRAPH_H
# define GS_VIS_GRAPH_H

/** \file gs_polygons.h
 * maintains an array of polygons
 */

# include <sig/gs_vec.h>
# include <sig/gs_graph.h>
# include <sig/gs_buffer.h>
# include <sig/gs_polygons.h>

class GsVisGraphNode;
class GsVisGraphLink : public GsGraphLink
{  public :
	GS_GRAPH_LINK_CASTED_METHODS(GsVisGraphNode,GsVisGraphLink);
	GsVisGraphLink () {}
	GsVisGraphLink ( const GsVisGraphLink& l ) : GsGraphLink() {}
   ~GsVisGraphLink () {}
	friend GsOutput& operator<< ( GsOutput& out, const GsVisGraphLink& l ) { return out; }
	friend GsInput& operator>> ( GsInput& inp, GsVisGraphLink& l ) { return inp; }
	static inline int compare ( const GsVisGraphLink* l1, const GsVisGraphLink* l2 ) { return 0; }
};

class GsVisGraphNode : public GsGraphNode
{  public :
	GsPnt2 p;
   public :
	GS_GRAPH_NODE_CASTED_METHODS(GsVisGraphNode,GsVisGraphLink);
	GsVisGraphNode () : GsGraphNode() {}
	GsVisGraphNode ( const GsVisGraphNode& n ) : GsGraphNode(), p(n.p) {}
	GsVisGraphNode ( const GsPnt& q ) : GsGraphNode(), p(q) {}
   ~GsVisGraphNode () {}
	friend GsOutput& operator<< ( GsOutput& out, const GsVisGraphNode& n ) { return out<<n.p; }
	friend GsInput& operator>> ( GsInput& inp, GsVisGraphNode& n ) { return inp>>n.p; }
	static inline int compare ( const GsVisGraphNode* n1, const GsVisGraphNode* n2 ) { return GsVec::compare(n1->p,n2->p); }
};

/*! \class GsVisGraph gs_vis_graph.h
	\brief a simple visibility graph for 2D path planning
*/   
class GsVisGraph : public GsShareable
{  protected :
	float _radius, _dang;
	GsVisGraphNode *_vi, *_vg;
	GsPolygons* _polygons;  // sharable polygons
	GsArray<GsVec> _bdisks; // bounding disks per polygon
	GsArrayPt<GsBuffer<GsVisGraphNode*>> _nodes;
	GsGraph<GsVisGraphNode,GsVisGraphLink> _graph;
	GsArray<GsVisGraphNode*> _path;

   public :
	/*! Default constructor */
	GsVisGraph ();

	/*! Virtual destructor */
	virtual ~GsVisGraph () { init(); }

	/*! Returns true if the array has no polygons, and false otherwise. */
	bool empty () const { return _bdisks.empty(); }

	float radius () const { return _radius; }
	float dang () const { return _dang; }

	/*! Deletes all internal structures and initializes as an empty graph */
	void init ();

	/*! Builds the visibility graph. GsVisGraph will keep a reference to parameter polys,
		and will convert all non-ccw polygons in it to ccw orientation. */
	void build ( GsPolygons* polys, float r=-1, float dang=-1 );

	GsVisGraphNode* node ( int pol, int vtx ) const { return _nodes[pol]->cget(vtx); }
	int psize ( int pol ) const { return _nodes[pol]->size(); }

	const GsGraph<GsVisGraphNode,GsVisGraphLink>& graph () const { return _graph; }
	int num_nodes () const { return _graph.num_nodes(); }
	int num_links () const { return _graph.num_links(); }
	void get_edges ( GsArray<GsVisGraphNode*>& edges ) { _graph.get_undirected_edges(edges); }

	bool shortest_path ( const GsPnt2& pi, const GsPnt2& pg, GsPolygon& path, float* cost=0 );

	const GsVisGraphNode* vi () const { return _vi; }
	const GsVisGraphNode* vg () const { return _vg; }

   protected :
	void _connect_to_visible ( GsVisGraphNode* n, int pk=-1, int vk=-1, const GsPnt2* sm=0, const GsPnt2* sp=0 );
	void _add_if_free ( GsVisGraphNode* n1, GsVisGraphNode* n2, int p1=-1, int v1=-1, int p2=-1, int v2=-1 );
};

//================================ End of File =================================================

# endif // GS_VIS_GRAPH_H
