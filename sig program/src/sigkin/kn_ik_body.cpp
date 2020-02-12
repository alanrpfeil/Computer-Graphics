/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigkin/kn_ik_body.h>
# include <sig/sn_lines.h>
# include <sig/sn_group.h>
# include <sigkin/kn_skeleton.h>

//========================= KnIkBody ================================

KnIkBody::KnIkBody ()
{
	_skeleton = 0;
	_coldet = 0;
	_result = KnIk::Ok;
	_posroot = 0;
	_solvetime = -1;
   
	for ( int i=0; i<4; i++ )
	{	_ik[i] = 0;
		_osearch_active[i] = 0;
		_lines[i] = 0;
	}
}

KnIkBody::~KnIkBody ()
{
	_unrefik ();
	if ( _skeleton ) _skeleton->unref();
	if ( _coldet ) _coldet->unref();
}   

void KnIkBody::_unrefik ()
{
	for ( int i=0; i<4; i++ ) if ( _ik[i] ) { _ik[i]->unref(); _ik[i]=0; }
}

void KnIkBody::_initskel ()
{
	if ( _skeleton ) { _skeleton->unref(); _skeleton=0; }

	for ( int i=0; i<4; i++ )
	{	if ( _ik[i] )
		{	_skeleton = _ik[i]->base()->skeleton();
			if ( _skeleton )
			{	_skeleton->ref();
				_posture.init ( _skeleton );
				break;
			}
		}
	}
}

int KnIkBody::init ( KnJoint* endef[4] )
 {
   int i, count=0;
   bool ok;
   _unrefik ();

   for ( i=0; i<4; i++ )
	{ // ensure endef is valid:
	  if ( endef[i]==0 ) continue;

	  // create new one:
	  _ik[i] = new KnIk;
	  _ik[i]->ref();

	  // initialize it:
	  ok = _ik[i]->init ( (KnIk::Type)i, endef[i] );
	  if ( ok )
	   { count++; }
	  else
	   { _ik[i]->unref(); _ik[i]=0; }
	}

   _initskel();

   return count;
 }

int KnIkBody::init ( KnSkeleton* sk, const char* endefs )
 {
   KnJoint* jts[4];
   _unrefik ();
   int i;
   GsInput inp;
   inp.init ( endefs );
   for ( i=0; i<4; i++ )
	{ jts[i] = 0;
	  inp.get();
	  if ( inp.ltype()==GsInput::String ) 
	   { jts[i] = sk->joint ( inp.ltoken() );
	   }
	}

   return init ( jts );
 }

int KnIkBody::init ( KnIk* ik[4] )
 {
   int i, count=0;
   _unrefik ();

   for ( i=0; i<4; i++ )
	{ // ensure ik[i] is valid:
	  if ( ik[i]==0 ) continue;
	  if ( ik[i]->base()==0 ) continue;
	  if ( ik[i]->type()!=i ) continue;

	  // ref new one:
	  _ik[i] = ik[i];
	  _ik[i]->ref();
	  count++;
	}

   _initskel();
   return count;
 }

void KnIkBody::solve_closest ( bool b, int i )
{ 
	if ( i<0 )
	{	_ik[0]->solve_closest(b);
		_ik[1]->solve_closest(b);
		_ik[2]->solve_closest(b);
		_ik[3]->solve_closest(b);
	}
	else
	{	_ik[i]->solve_closest(b);
	}
}

void KnIkBody::coldet ( KnColdet* cd )
{
	if ( _coldet ) { _coldet->unref(); _coldet=0; }
	_coldet = cd;
	if ( _coldet ) _coldet->ref();
}

void KnIkBody::remove_joint_limits ()
{
	if ( _ik[0] ) _ik[0]->remove_joint_limits();
	if ( _ik[1] ) _ik[1]->remove_joint_limits();
	if ( _ik[2] ) _ik[2]->remove_joint_limits();
	if ( _ik[3] ) _ik[3]->remove_joint_limits();
}

void KnIkBody::capture_init_posture ()
{
	if ( !_skeleton ) return;
	_skeleton->update_global_matrices();
   
	// get feet global positions:
	if ( _ik[2] ) _lfoot = _ik[2]->end()->gmat();
	if ( _ik[3] ) _rfoot = _ik[3]->end()->gmat();
   
	// get legs orbit angles:
	if ( _ik[2] ) _osearch[2].init = _ik[2]->orbit_angle(); // lleg
	if ( _ik[3] ) _osearch[3].init = _ik[3]->orbit_angle(); // rleg

	// select best root:
	_posroot = _skeleton->root();
	if ( _posroot->children()==1 ) // try "second root":
	{	if ( !_posroot->pos()->frozen(0) &&
			 !_posroot->pos()->frozen(1) &&
			 !_posroot->pos()->frozen(2) )
		{	_posroot=_posroot->child(0); // 2nd root is preferred, 1st one may have other purposes
		}
	}

	// get root position:
	_initpos.x = _posroot->pos()->valuex();
	_initpos.y = _posroot->pos()->valuey();
	_initpos.z = _posroot->pos()->valuez();
}

KnIk::Result KnIkBody::solve ( int i, const GsMat& globgoal )
{
	// 1. Be sure IK exists:
	if ( !_ik[i] ) return KnIk::Ok;

	// 2. Set goal matrix to local:
	_posture.get(); // save entire body posture, looking ahead to full body ik
	_skeleton->update_global_matrices();
	_locgoal = globgoal;

	_ik[i]->set_local(_locgoal);

	// 3. Solve linkage i:
	if ( _solvetime>=0 ) _solvetime=gs_time();
	if ( _osearch_active[i] )
	{	_result = _ik[i]->solve ( _locgoal, _osearch[i], _coldet );
	}
	else
	{	_result = _ik[i]->solve ( _locgoal, _osearch[i].init, _coldet );
	}
	if ( _solvetime>=0 ) _solvetime=gs_time()-_solvetime;

	if ( _result==KnIk::Ok )
		_ik[i]->apply_last_result();
	else
		_posture.apply();

	return _result;
}

int KnIkBody::add_lines ( SnGroup* g )
{
	int i, count=0;
	SnLines* lines;

	for ( i=0; i<4; i++ )
	{	if ( ik(i) )
		{	lines = new SnLines;
			_lines[i] = lines;
			g->add ( lines );
			count++;
		}
	}

	return count;
}

void KnIkBody::lines ( int i, bool attach )
{
	if ( ik(i) ) ik(i)->lines ( attach? lines(i):0 );

	if ( _lines[i] && !attach ) _lines[i]->init();
}

//============================== EOF =====================================
