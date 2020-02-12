/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_model.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigkin/kn_scene.h>
# include <sigkin/kn_coldet.h>
# include <sigkin/kn_ik_manipulator.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_skin.h>

//=================================== KnIkManipulator =====================================

const char* KnIkManipulator::class_name = "KnIkManipulator";

KnIkManipulator::KnIkManipulator () : SnGroup(class_name)
{
	_ik = 0;
	_res = KnIk::Ok;
	_ikbody = 0;
	_ikbodyid = 0;
	_knscene = 0;
	_coldet = 0;
	_cbfunc = 0;
	_postcbfunc = 0;
	_udata = 0;
	_postudata = 0;
	separator ( true );
}

KnIkManipulator::~KnIkManipulator ()
{
	if ( _ik ) _ik->unref();
	if ( _ikbody ) _ikbody->unref();
	if ( _knscene ) _knscene->unref();
	if ( _coldet ) _coldet->unref();
}

static void _ikcb ( SnManipulator* /*manip*/, const GsEvent& /*ev*/, void* udata )
{
	((KnIkManipulator*)udata)->update();
}

bool KnIkManipulator::init ( KnIk* ik, GsModel* goalmodel )
{
	if ( !ik ) return false;
	if ( !ik->base() ) return false;
   
	// unref old classes:
	if ( _ik ) _ik->unref();
	if ( _ikbody ) { _ikbody->unref(); _ikbody=0; }
   
	// ref new classes:
	_ik = ik;
	_ik->ref();
	_ik->lines(0);
	_iksearch.set_defaults ( _ik->type() );
	_method = UserCallback;
	_res = KnIk::Ok;
   
	// create nodes to display:
	GsModel* model = goalmodel? goalmodel:_ik->end()->visgeo();
	_manip = new SnManipulator;
	_lines = new SnLines;
	_matrix = new SnTransform;
	SnModel* snmodel = new SnModel(model); // ok if model is null
	if ( _ik->end()->skeleton()->skin() ) snmodel->visible(false); //render_mode(gsRenderModeLines);
	SnGroup::remove_all();
	SnGroup::add ( _manip );
	SnGroup::add ( _lines );
	SnGroup::add ( _matrix );
	SnGroup::add ( snmodel ); 

	// Create axis with the same sizes as of the model to manipulate:
	GsBox box;
	if ( model ) model->get_bounding_box ( box );
	else { box.a=GsVec::minusone; box.b=GsVec::one; }
	SnLines* l = new SnLines;
	l->push_axis ( GsPnt::null, 0/*len not used*/, 3, "xyz", true/*unit marks*/, &box );

	// Set the manip to control the axis in the current wrist place:
	_ik->end()->skeleton()->update_global_matrices();
	match ();
	_manip->child ( l );
	_manip->callback ( _ikcb, this );
	return true;
}

bool KnIkManipulator::init ( KnIkBody* ikbody, int i, GsModel* goalmodel )
{
	if ( !ikbody ) return false;
	if ( !ikbody->ik(i) ) return false;
	if ( !init(ikbody->ik(i),goalmodel) ) return false;
	_ikbody = ikbody;
	_ikbody->ref();
	_ikbodyid = i;
	return true;
}

void KnIkManipulator::solve_method ( SolveMethod sm, KnScene* knscene, KnColdet* coldet )
{
	// unref old classes:
	if ( _knscene ) _knscene->unref();
	if ( _coldet ) _coldet->unref();
   
	// ref new classes:
	_knscene = knscene;
	_coldet = coldet;
	if ( _knscene ) _knscene->ref();
	if ( _coldet ) _coldet->ref();
   
	// finally:
	_method = sm;
	_res = KnIk::Ok;
	if ( !_knscene ) _method = UserCallback;
}

void KnIkManipulator::callback ( CallBack cb, void* udata )
{
	_method = UserCallback;
	_cbfunc = cb;
	_udata = udata;
}
 
void KnIkManipulator::post_callback ( CallBack cb, void* udata )
{
	_postcbfunc = cb;
	_postudata = udata;
}

void KnIkManipulator::match ()
{
	GsMat goal;
	_ik->base()->skeleton()->update_global_matrices();
	goal = _ik->end()->gmat();
	_manip->init();
	_manip->initial_mat ( goal );
	_matrix->set ( goal );
}
 
void KnIkManipulator::lines ( bool b )
{
	if ( b )
	{	_ik->lines ( _lines );
		update ();
	}
	else
	{	_ik->lines ( 0 );
		_lines->init();
	}
}

KnIk::Result KnIkManipulator::update ()
{
	if ( !_manip ) return KnIk::Ok;
	_matrix->set ( _manip->mat() ); // update the displayed model position
	return iksolve ();   
}

const GsMat& KnIkManipulator::cmat() const
{
	if ( !_manip ) return GsMat::id;
	return _manip->mat();
}

GsMat& KnIkManipulator::mat()
{
	return _manip->mat();
}

//-------------- virtuals -----------------------

KnIk::Result KnIkManipulator::iksolve ()
{
	_res = KnIk::Ok;

	if ( _method==UserCallback )
	{	if (_cbfunc) (_cbfunc) ( this, _udata );
	}
	else if ( _method==FixedOrbit )
	{	GsMat goal = _manip->mat();
		_ik->set_local ( goal );
		_res = _ik->solve ( goal, _iksearch.oangle, _coldet );
		if ( _res==KnIk::Ok )
		{	_ik->apply_last_result();
			if ( _knscene ) _knscene->update();
			if ( _knscene->skeleton()->skin() ) _knscene->skeleton()->skin()->update();
		}
	}
	else if ( _method==SearchOrbit )
	{	GsMat goal = _manip->mat();
		_ik->set_local ( goal );
		_res = _ik->solve ( goal, _iksearch, _coldet );
		if ( _res==KnIk::Ok )
		{	_ik->apply_last_result();
			if ( _knscene ) _knscene->update();
			if ( _knscene->skeleton()->skin() ) _knscene->skeleton()->skin()->update();
		}
	}
	else if ( _method==IkBody )
	{	_res = _ikbody->solve ( _ikbodyid, _manip->mat() );
		if ( _knscene ) _knscene->update();
		if ( _knscene->skeleton()->skin() ) _knscene->skeleton()->skin()->update();
	}

	if (_postcbfunc) (_postcbfunc) ( this, _postudata );

	return _res;
}

int add_ik_manipulators ( SnGroup* g, KnIkBody* ikb, KnScene* knscene, 
						  KnIkManipulator::CallBack postcb, void* udata, KnColdet* coldet )
{
	KnIkManipulator* ikm;
	int i, count=0;

	for ( i=0; i<4; i++ )
	{
		ikm = new KnIkManipulator;
		ikm->ref();

		if ( ikm->init(ikb,i) )
		{	g->add ( ikm );
			ikm->solve_method ( KnIkManipulator::IkBody, knscene, coldet );
			if ( postcb ) ikm->post_callback(postcb,udata);
			count++;
		}

		ikm->unref();
	}

	return count;
}

int match_ik_manipulators ( SnGroup* g )
{
	KnIkManipulator* ikm;
	int i, count=0;

	for ( i=0; i<g->size(); i++ )
	{
		if ( g->get(i)->instance_name()==KnIkManipulator::class_name )
		{
			ikm = (KnIkManipulator*)g->get(i);
			ikm->match();
			count++;
		}
	}

	return count;
}

int update_ik_manipulators ( SnGroup* g )
{
	KnIkManipulator* ikm;
	int i, count=0;

	for ( i=0; i<g->size(); i++ )
	{
		if ( g->get(i)->instance_name()==KnIkManipulator::class_name )
		{
			ikm = (KnIkManipulator*)g->get(i);
			ikm->update();
			count++;
		}
	}

	return count;
}

KnIkManipulator* get_ik_manipulator ( SnGroup* g, int id )
{
	KnIkManipulator* ikm;

	for ( int i=0, s=g->size(); i<s; i++ )
	{
		if ( g->get(i)->instance_name()==KnIkManipulator::class_name )
		{	ikm = (KnIkManipulator*)g->get(i);
			if ( ikm->ikbodyid()==id ) return ikm;
		}
	}

	return 0;
}

//======================================= EOF =====================================

