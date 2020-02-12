/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_vis_graph.h>
# include <sig/gs_geo2.h>

//# define GS_USE_TRACE1 // build
//# define GS_USE_TRACE2 // search
# include <sig/gs_trace.h>

//=== GsBuffer =======================================================================

GsVisGraph::GsVisGraph ()
{
	_dang = 0;
	_radius = 0;
	_polygons = 0;
	_vi = _vg = 0;
}

void GsVisGraph::init ()
{
	_radius = _dang = 0;
	if ( _polygons ) { _polygons->unref(); _polygons=0; }
	_bdisks.size(0);
	_nodes.init();
	_graph.init();
	_vi = _vg = 0;
}

# define FOR_ALL_POL(p)			for ( int p=0; p<s; p++ ) 
# define FOR_ALL_PVTX(p,P,v,vs)	const GsBuffer<GsVisGraphNode*>& P=*_nodes[p]; for ( int v=0, vs=P.size(); v<vs; v++ )
# define FOR_ALL_VERTICES(p,P,v,vs)	FOR_ALL_POL(p) { FOR_ALL_PVTX(p,P,v,vs) {
# define END_FOR }}

void GsVisGraph::build ( GsPolygons* polys, float r, float dang )
{
	GS_TRACE1 ( "Build started..." );

	init ();

	if ( !polys ) return;
	if ( _polygons ) _polygons->unref();
	_polygons = polys;
	_polygons->ref();
	_radius = r;
	_dang = dang;

	GsPnt2 dc;
	float dr;

	int ps = _polygons->size();
	_bdisks.size(ps);

	// Compute bounding disks and nodes:
	GS_TRACE1 ( "Pre-processing input..." );
	GsPolygon pol, polb;
	for ( int i=0; i<ps; i++ )
	{	// Inflate polygon and check CCW orientation:
		if ( !_polygons->get(i).ccw() ) _polygons->get(i).reverse();
		if ( r>0 && dang>0 )
			pol.inflate ( _polygons->get(i), r, dang );
		else
			pol = _polygons->get(i);

		// Store bounding disk for test optimization:
		pol.get_bounding_disk ( dc, dr );
		_bdisks[i].set ( dc.x, dc.y, dr );

		// Insert vertices to the graph:
		GsBuffer<GsVisGraphNode*>& n = *_nodes.push();
		n.size ( pol.size() );
		for ( int j=0, js=pol.size(); j<js; j++ )
		{	n[j] = new GsVisGraphNode(pol[j]);
			_graph.insert ( n[j] );
		}
	}

	// Compute links:
	GS_TRACE1 ( "Adding edges..." );
	int s = _nodes.size();

	FOR_ALL_VERTICES(pi,Pi,vi,vis)
		int vim = Pi.vid(vi-1);
		int vip = Pi.vidpos(vi+1);
		const GsPnt2& p = Pi.get(vi)->p;
		const GsPnt2& pm = Pi.get(vim)->p;
		const GsPnt2& pp = Pi.get(vip)->p;

		// Only consider connecting CCW corners:
		if ( ccw(pm,p,pp)<=0 ) continue;

		// Connect polygon boundary if it leads to CCW corner:
		const GsPnt2& ppp = Pi.get(Pi.vidpos(vi+2))->p;
		if ( ccw(p,pp,ppp)>0 )
			_add_if_free ( _nodes[pi]->get(vi), _nodes[pi]->get(vip), pi,vi, pi,vip );

		// Connect to other polygons:
		GsVisGraphNode* n = _nodes[pi]->get(vi);
		_connect_to_visible ( n, pi, vi, &pm, &pp );
	END_FOR

	GS_TRACE1 ( "Done." );
}

void GsVisGraph::_connect_to_visible ( GsVisGraphNode* na, int pa, int va, const GsPnt2* sm, const GsPnt2* sp )
{
	GsPnt2 a(na->p);
	double x, y;

	int s = _nodes.size();
	FOR_ALL_VERTICES(pi,Pi,vi,vis)
		// Treat cases between vertices in same polygon:
		int vim = Pi.vid(vi-1);
		int vip = Pi.vidpos(vi+1);
		if ( pi==pa ) { if ( va==vi || va==vim || va==vip ) continue; }

		// Only consider connecting CCW corners:
		const GsPnt2& b = Pi.get(vi)->p;
		const GsPnt2& bm = Pi.get(vim)->p;
		const GsPnt2& bp = Pi.get(vip)->p;
		if ( ccw(bm,b,bp)<=0 ) continue;

		// Tangency tests:
		if ( sm && gs_segment_line_intersect ( sm->x,sm->y,sp->x,sp->y, a.x,a.y,b.x,b.y, x,y ) ) continue;
		if ( gs_segment_line_intersect ( bm.x,bm.y,bp.x,bp.y, a.x,a.y,b.x,b.y, x,y ) ) continue;

		// Add if visible:
		GsVisGraphNode* nb = _nodes[pi]->get(vi);
		_add_if_free ( na, nb, pa, va, pi, vi );
	END_FOR
}

void GsVisGraph::_add_if_free ( GsVisGraphNode* n1, GsVisGraphNode* n2, int p1, int v1, int p2, int v2 )
{
	GsPnt2 a(n1->p);
	GsPnt2 b(n2->p);
	int s = _nodes.size();

	if ( n1->search_link(n2)>=0 ) return; // link already there

	// visibility tests:
	FOR_ALL_POL(pi)
	{	// bounding disk prunning:
		if ( pi!=p1 && pi!=p2 && gs_point_segment_dist(_bdisks[pi].x,_bdisks[pi].y, a.x,a.y,b.x,b.y)>_bdisks[pi].z ) continue;
		FOR_ALL_PVTX(pi,Pi,vi,vis)
		{	// do not test segments with endpoints in n1-n2 line:
			int vip = Pi.vidpos(vi+1);
			if ( pi==p1 && (vi==v1||vip==v1) ) continue;
			if ( pi==p2 && (vi==v2||vip==v2) ) continue;

			// intersection test:
			const GsPnt2& c = Pi.get(vi)->p;
			const GsPnt2& d = Pi.get(vip)->p;
			if ( gs_segments_intersect(a.x,a.y,b.x,b.y, c.x,c.y,d.x,d.y) ) return;
		}
	}
	// ok add link:
	_graph.link ( n1, n2, dist(n1->p,n2->p) );
}

bool GsVisGraph::shortest_path ( const GsPnt2& pi, const GsPnt2& pg, GsPolygon& path, float* cost )
{
	GS_TRACE2 ( "Initializing search..." );
	bool newpi=false, newpg=false;

	// Initialize if needed:
	if ( !_vi )
	{	_vi=new GsVisGraphNode; _graph.insert(_vi); newpi=true;
		_vg=new GsVisGraphNode; _graph.insert(_vg); newpg=true;
	}
	else // if new points given, unlink old ones:
	{	if ( _vi->p!=pi ) { _vi->unlink(); newpi=true; }
		if ( _vg->p!=pg ) { _vg->unlink(); newpg=true; }
	}

	// link new points:
	if ( newpi ) { _vi->p=pi; _connect_to_visible(_vi); }
	if ( newpg ) { _vg->p=pg; _connect_to_visible(_vg); }
	_add_if_free ( _vi, _vg ); // will not add if link already there

	// search path:
	GS_TRACE2 ( "Searching..." );
	float gcost;
	bool found = _graph.shortest_path ( _vi, _vg, _path, 0, &gcost );
	path.open ( true );
	path.size ( _path.size() );
	for ( int i=_path.size()-1; i>=0; i-- ) path[i]=_path[i]->p;

	GS_TRACE2 ( "Path size:"<<_path.size()<<" cost:"<<gcost );
	if ( cost ) *cost = gcost;
	return found;
}

//=== End of File =====================================================================
