/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include "skelviewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sigogl/ui_dialogs.h>

# include <sigogl/ws_run.h>

# include <sigkin/kn_scene.h>

void ManipPCB ( KnIkManipulator* ikm, void* udata )
{
	((MySkelViewer*)udata)->iksolved(ikm);
}

MySkelViewer::MySkelViewer ( KnSkeleton* sk, int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{ 
	// create my scene and viewer:
	
	rootg()->add ( _sksc=new KnScene(sk) );

	// set visualization:
	bool viewskin=false, viewskel=false, viewvisg=false, viewcolg=false, viewaxis=false;
	if ( sk->skin() )
	{	viewskin=true;
	}
	else if ( sk->visgeos()==0 ) // if no geometries show the skeleton:
	{	viewskel=true; viewvisg=false;
	}
	else viewvisg=true;
	_sksc->set_visibility ( viewskin, viewskel/*skel*/, viewvisg/*visgeo*/, viewcolg/*colgeo*/, viewaxis/*axis*/ );

	// Extra settings:
	//sk->enforce_rot_limits ( false );

	_sk=sk;
	_seljoint=0;

	UiPanel *p, *sp;
	p = uim()->add_panel ( 0, UiPanel::HorizLeft );

	_effs.sizecap(0,4);
	int s=sk->joints().size();
	p->add ( new UiButton ( "joints", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_jointsp=sp;
		for ( int i=0; i<s; i++ )
		{	p->add ( new UiRadioButton ( sk->joints()[i]->name(), EvJoint+i, i==0? true:false ) );
			if ( sk->joints()[i]->ik() ) _effs.push()=sk->joints()[i];
		}
	}

	p->add ( new UiButton ( "values", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_valuesp=sp;
		for ( int i=0; i<6; i++ )
		{	_sl[i] = (UiSlider*)p->add ( new UiSlider ( "V:", EvP1+i ) );
			_sl[i]->all_events(true);
		}
		//if ( sk->joints().size() ) build_value_panel(0);
	}

	s=sk->postures().size();
	if ( s>0 )
	{	p->add ( new UiButton ( "postures", sp=new UiPanel(0,UiPanel::Vertical) ) );
		UiPanel* p=_posturesp=sp;
		for ( int i=0; i<s; i++ )
		{	p->add ( new UiRadioButton ( sk->postures()[i]->name(), EvPosture+i, i==0? true:false ) );
		}
	}

	s=_effs.size();
	if ( s>0 )
	{	rootg()->add( _mg=new SnGroup );
		p->add ( new UiButton ( "effectors", sp=new UiPanel(0,UiPanel::Vertical) ) );
		UiPanel* p=_endeffs=sp;
		for ( int i=0; i<s; i++ )
		{	p->add ( new UiButton ( _effs[i]->name(), EvEff+i ) );
			KnIkManipulator* ikm = new KnIkManipulator;
			ikm->init ( _effs[i]->ik() );
			ikm->solve_method ( KnIkManipulator::SearchOrbit, _sksc );
			ikm->post_callback ( ManipPCB, this );
			ikm->lines ( true );
			_mg->add ( ikm );
		}
	}

	p->add ( new UiButton ( "view", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_valuesp=sp;
		_vbut[0] = (UiCheckButton*)p->add ( new UiCheckButton ( "skin", EvView, viewskin ) );
		_vbut[1] = (UiCheckButton*)p->add ( new UiCheckButton ( "skeleton", EvView, viewskel ) );
		_vbut[2] = (UiCheckButton*)p->add ( new UiCheckButton ( "visgeo", EvView, viewvisg ) );
		_vbut[3] = (UiCheckButton*)p->add ( new UiCheckButton ( "colgeo", EvView, viewcolg ) );
		_vbut[4] = (UiCheckButton*)p->add ( new UiCheckButton ( "frames", EvView, viewaxis ) );
	}

	p->add ( new UiButton ( "info", EvInfo ) );

	p->add ( new UiButton ( "motions", sp=new UiPanel(0,UiPanel::Vertical) ) );
	{	UiPanel* p=_motionsp=sp;
		p->add ( new UiButton ( "load", EvLoadMotion ) );
	}

	p->add ( new UiButton ( "exit", EvExit ) );
}

void MySkelViewer::joint_info ( int jid )
{
	if ( jid<0 || jid>=_sk->joints().size() ) return; // protection
	KnJoint* j = _sk->joints()[jid];

	message().len(0); GsOutput o; o.init(message());
	o << "Name:[" << j->name() << "] ";
	o << "Parent:[" << (j->parent()?j->parent()->name():"null") << "] ";
	o << "Offset:[" << j->offset() << "] ";
	o << "PosDOFs:[" << (3-j->pos()->nfrozen()) << "] ";
	o << "Prepost:[" << (j->rot()->hasprepost()) << "] ";

	o << " Channels:[";
	const KnChannels& ch = _sk->channels();
	for ( int i=0, s=ch.size(); i<s; i++ )
	{	if ( j->name()==ch.cget(i).jname() )
		{	o << ch.cget(i).type_name();
			if ( i+1<s ) o<<gspc;
		}
	}
	o << "]";
}

void MySkelViewer::iksolved ( KnIkManipulator* ikm )
{
	message().set ( KnIk::message(ikm->result()) );
}

void MySkelViewer::build_value_panel ( int jid )
{
	KnJoint* j = _sk->joints()[jid];
	for ( int i=0; i<6; i++ ) _sl[i]->hide();

	if ( !j->pos()->frozen(0) )
	{	_sl[0]->activate();
	}
	if ( !j->pos()->frozen(1) )
	{	_sl[1]->activate();
	}
	if ( !j->pos()->frozen(2) )
	{	_sl[2]->activate();
	}

	if ( j->rot_type()==KnJoint::TypeQuat )
	{
	}
	else if ( j->rot_type()==KnJoint::TypeEuler )
	{	KnJointEuler* e = j->euler();
		const char* l[] = { "Rx", "Ry", "Rz" };
		for ( int d=0; d<3; d++ )
		{	if ( !e->frozen(d) )
			{	_sl[d+3]->label().set(l[d]);
				_sl[d+3]->activate();
				_sl[d+3]->range(e->lolim(d),e->uplim(d));
				_sl[d+3]->value(e->value(d));
			}
		}
	}
	else if ( j->rot_type()==KnJoint::TypeST )
	{	KnJointST* st = j->st();
		_sl[3]->label().set("Sx:");	_sl[3]->activate();	_sl[3]->range(-gspi,gspi); _sl[3]->value(st->swingx());
		_sl[4]->label().set("Sy:");	_sl[4]->activate();	_sl[4]->range(-gspi,gspi); _sl[4]->value(st->swingy());
		if ( !st->twist_frozen() )
		{	_sl[5]->label().set("Tw:");
			_sl[5]->activate();
			_sl[5]->range(st->twist_lolim(),st->twist_uplim());
			_sl[5]->value(st->twist());
		}
	}

	UiPanel* p=_valuesp;
	p->changed ( UiElement::NeedsRebuild );
//	p->rebuild();
}

void MySkelViewer::update_joint ( int si, int jid )
{
	KnJoint* j = _sk->joints()[_seljoint];
	if ( si<3 ) // position DOFs
	{ // to be completed
	}
	else // rotation DOFs
	{	if ( j->rot_type()==KnJoint::TypeEuler )
		{	if ( si==3 ) j->euler()->value ( 0, _sl[3]->value() );
			else if ( si==4 ) j->euler()->value ( 1, _sl[4]->value() );
			else if ( si==5 ) j->euler()->value ( 2, _sl[5]->value() );
		}
		if ( j->rot_type()==KnJoint::TypeST )
		{	if ( si<5 )
				j->st()->swing ( _sl[3]->value(), _sl[4]->value() );
			else
				j->st()->twist ( _sl[5]->value() );
		}
		else
		{ // to be completed with xyz -> quat
		}
	}
}

void MySkelViewer::apply_motion ( KnMotion* m, int n, float t )
{
	m->apply ( t );
	message().setf ( "%s:  n=%d  t=%4.2f  fps=%4.2f", m->name(), n, t, (t>0?float(n)/t:0.0f) );
	_sksc->update ();
	render ();
	ws_check ();
}

int MySkelViewer::uievent ( int e )
{
	const int DEV=200;

	if ( e>=EvP1 && e<=EvR3 )
	{	update_joint(e-EvP1,_seljoint);
		_sksc->update();
		render();
	}
	else if ( e>=EvPosture && e<EvPosture+DEV )
	{	KnPosture* p = _sk->postures()[e-EvPosture];
		p->apply();
		_sksc->update();
		message()=""; GsOutput o; o.init(message());
		p->output ( o, false, true );
		return 1;
	}
	else if ( e>=EvEff && e<EvEff+DEV )
	{	KnJoint* j = _effs[e-EvEff];
		_mg->get<KnIkManipulator>(e-EvEff)->lines( j->ik()->lines()? false:true );
		//_sksc->update();
		render ();
		return 1;
	}
	else if ( e>=EvJoint && e<EvJoint+DEV )
	{	_seljoint = e-EvJoint;
		joint_info ( _seljoint );
		build_value_panel ( _seljoint );
		return 1;
	}
	else if ( e>=EvMotion && e<EvMotion+DEV )
	{	KnMotion* m = _motions[e-EvMotion];
		double d = (double)m->duration();
		double t=0, t0=gs_time();
		int n=1;
		while ( t<d )
		{	apply_motion ( m, n++, (float)t );
			t = gs_time()-t0;
		}
		apply_motion ( m, n, (float)d );
		return 1;
	}
	else switch ( e )
	{	case EvLoadMotion:
		{	const char* mfile = ui_select_file ( "Selection motion file", 0, "*.sm;*.bvh" );
			if ( !mfile ) return 1;
			KnMotion* m = new KnMotion;
			if ( !m->load ( mfile ) ) { delete m; return 1; }
			_motions.push ( m );
			m->connect ( _sk );
			m->ref ();
			_motionsp->add ( new UiButton ( m->name(), EvMotion+_motions.size()-1 ) );
			_motionsp->build();
			if ( _motions.size()==1 ) _motionsp->top()->separate();
		} return 1;

		case EvView:
		{	_sksc->set_visibility ( _vbut[0]->value(), _vbut[1]->value(), _vbut[2]->value(), _vbut[3]->value(), _vbut[4]->value() );
			_sksc->update();
		} return 1;

		case EvInfo: 
		{	message().setf ( "Skeleton:%s  Joints:%d  Channels:%d  Channels in Motions:", 
					_sk->name(), _sk->joints().size(), _sk->channels().size() );
			for ( int i=0; i<_motions.size(); i++ ) message()<<" "<<_motions[i]->channels()->size();
		} return 1;

		case EvExit: ws_exit(); return 1;
	}

	return WsViewer::uievent(e);
}
