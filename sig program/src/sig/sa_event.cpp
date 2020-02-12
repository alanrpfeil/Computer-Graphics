/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_editor.h>
# include <sig/sa_event.h>

//# define GS_USE_TRACE1 // constructor / destructor
//# define GS_USE_TRACE2 // closest editor selection
# include <sig/gs_trace.h>

//================================== SaEvent ====================================

void SaEvent::apply ( SnNode* n )
{
	// 1. Initialize
	_hits.size ( 0 );
	_result=0;

	// 2. Traverse scene
	SaAction::apply ( n );

	// 3. Get hit closest to camera eye and send the event to the corresponding editor
	if ( _hits.size()>0 )
	{	GS_TRACE2 ( "Hits: "<<_hits.size() );
		int i, imin=0;
		float tmin=_hits[0].t;
		for ( i=1; i<_hits.size(); i++ )
		{	if ( _hits[i].t<tmin ) { tmin=_hits[i].t; imin=i; } }
		GS_TRACE2 ( "min index: "<<imin );
		_hits[imin].ed->handle_event ( _hits[imin].ev, _hits[imin].t );
	}
}

bool SaEvent::editor_apply ( SnEditor* ed )
{
	push_matrix ();
	mult_matrix ( ed->mat() );

	GsMat mat = _matstack.top();
	mat.invert();

	GsEvent ev = _ev;
	ev.ray.p1 = mat*ev.ray.p1;
	ev.ray.p2 = mat*ev.ray.p2;
	ev.lray.p1 = mat*ev.lray.p1;
	ev.lray.p2 = mat*ev.lray.p2;
	ev.mousep = mat*ev.mousep;
	ev.lmousep = mat*ev.lmousep;

	float t;
	int res=0;
	if ( ed->visible() )
	{	res = ed->check_event ( ev, t );
		if ( res>_result ) _result=res;
	}

	if ( res==2 ) // event must be handled now
	{	GS_TRACE2 ( "Priority Hit t="<<t );
		ed->handle_event ( ev, t );
		_hits.size(0); // ensure no closest determination is done
		return false; // interrupt traversal
	}
	else if ( res==1 ) // event can be handled: save it
	{	GS_TRACE2 ( "Hit t="<<t );
		_hits.push();
		_hits.top().ev = ev;
		_hits.top().ed = ed;
		_hits.top().t = t;
	}
	else if ( res==0 )
	{	SaAction::apply ( ed->child() ); // propagate event to child
	}

	pop_matrix ();
	return true;
}

//======================================= EOF ====================================
