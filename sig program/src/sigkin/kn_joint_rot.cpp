/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_joint_rot.h>
# include <sigkin/kn_joint.h>

//# define GS_USE_TRACE1 // owner/sync tracking
//# define GS_USE_TRACE2 // value methods
//# define GS_USE_TRACE3 // prepost
# include <sig/gs_trace.h>

# define JN "("<<((const char*)_joint->name())<<") "

//============================= KnJointRot ============================

KnJointRot::KnJointRot ( KnJoint* j )
{
	_joint = j;
	_prepost = 0;
	_sync = FQ;  // init as in sync with the initial full rotation value
	_frozen = 1; // init as a non active quaternion
	_mode = FullMode;
	_euler = 0;
	_st = 0;
}

KnJointRot::~KnJointRot()
{
	delete _prepost; // C++ handles null pointer case
	delete _euler;
	delete _st;
}

void KnJointRot::init ( const KnJointRot& jr )
{
	if ( jr.hasprepost() )
	{	if ( !_prepost ) _prepost = new PrePost; 
		_prepost->pre=jr._prepost->pre;
		_prepost->post=jr._prepost->post;
	}
	else if ( _prepost )
	{	delete _prepost;
		_prepost=0;
	}

	_quat = jr._quat; 
	_lquat = jr._lquat;
	_frozen = jr._frozen;
	_mode = jr._mode;
	_sync = jr._sync;
	if ( jr._st ) st()->init ( jr._st ); else { delete _st; _st=0; }
	if ( jr._euler ) euler()->init ( jr._euler ); else { delete _euler; _euler=0; }

	// forcing to recompute the local mat and gmat out of date
	_joint->set_lmat_changed();
	GS_FLAG_OFF(_sync, JT); 
}

void KnJointRot::value ( const float* f )
{
	GS_TRACE2 ( JN<<"set value" );
	if ( _frozen ) return;

	if ( !_prepost || _mode==FullMode )
	{	_quat.set(f);
		setowner(FQ);
	} 
	else // local mode only with prerot/postrot set
	{	_lquat.set(f);
		setowner(LQ);
	}
}

const GsQuat& KnJointRot::value ()
{
	GS_TRACE2 ( JN<<"get value" );
	if ( _mode==FullMode ) 
		return fullvalue();
	else
		return localvalue();
}

const GsQuat& KnJointRot::fullvalue ()
{
	GS_TRACE2 ( JN<<"get full value" );
	syncto(FQ); return _quat;
}

const GsQuat& KnJointRot::localvalue ()
{
	GS_TRACE2 ( JN<<"get local value" );
	syncto(LQ); return _lquat;
}

void KnJointRot::initprepost ()
{
	GS_TRACE2 ( JN<<"init prepost" );
	if ( !_prepost ) return;
	GsQuat q = value();
	delete _prepost;
	_prepost=0;
	setowner ( FQ );
	value (q);
}

void KnJointRot::prerot ( const GsQuat& q )
{
	GS_TRACE2 ( JN<<"prerot" );
	GsQuat fq = fullvalue();
	if ( !_prepost ) _prepost = new PrePost;
	_prepost->pre = q;   
	setowner(FQ);
	_quat = fq;
}
 
void KnJointRot::postrot ( const GsQuat& q )
{
	GS_TRACE2 ( JN<<"posrot" );
	GsQuat fq = fullvalue();
	if ( !_prepost ) _prepost = new PrePost;
	_prepost->post = q;
	setowner(FQ);
	_quat = fq;
}

void KnJointRot::align ( AlignType t, const GsVec& v )
{
	if ( _joint->children()<1 ) return;
	if ( !_prepost ) _prepost = new PrePost;

	GsQuat fq = fullvalue();
	KnJointRot* c = _joint->child(0)->quat();

	if ( t==AlignPostInv )
	{ 
		GsQuat q ( v, _prepost->post.apply(c->_joint->offset()) );
		_prepost->pre = q * _prepost->pre;
		_prepost->post = q.inverse() * _prepost->post;
	}
	else if ( t==AlignPreInv )
	{
		GsQuat q ( _prepost->pre.apply(v), _joint->offset() );
		_prepost->pre = q * _prepost->pre;
		_prepost->post = q.inverse() * _prepost->post;
	}

	if ( t==AlignPre || t==AlignPrePost )
	{
		GsQuat q ( _prepost->pre.apply(v), _joint->offset() );
		_prepost->pre = q * _prepost->pre;
	}

	if ( t==AlignPost || t==AlignPrePost )
	{
		GsQuat q ( v, _prepost->post.apply(c->_joint->offset()) );
		_prepost->post = q.inverse() * _prepost->post;
	}

	setowner(FQ);
	_quat = fq;
}

//============================ private =============================

# define FLAGST(f) "["<<(f&JT?"JT ":"")<<(f&FQ?"FQ ":"")<<(f&LQ?"LQ ":"")<<(f&ST?"ST ":"")<<(f&EU?"EU ":"")<<"]"

void KnJointRot::setowner ( SyncRot sr )
{
	GS_TRACE1 ( JN<<"setowner: "<<FLAGST(sr) );
	_sync=sr; // rewrites current update signal
	_joint->set_lmat_changed();
}

/* Conventions:
- syncto(LQ) is requested when pre/post exists otherwise is mirrored to FQ
- syncto(FQ) will also sync to LQ in the process
- when a new value is set somewhere, it will call setowner() for the respective type
- sync bits will be gradually set to 1 as we sync values from multiple parameterizations */
void KnJointRot::syncto( SyncRot sr, bool force ) // will make sure the given type gets in sync
{
	GS_TRACE1 ( JN<<"syncto starting: _sync=="<<FLAGST(_sync)<<", sr=="<<FLAGST(sr) );
	if ( !force && insync(sr) ) { GS_TRACE1("insync."); return; } // already in sync: nothing to do

	// 1. First make sure LQ is in sync
	if ( _sync&FQ ) // convert from quat to lquat
	{	GS_TRACE1("quat->lquat");
		if (_prepost)
		{	GS_TRACE1("prepost applied");
			_lquat = _prepost->remove(_quat);
			_lquat.normalize();
		}
		else _lquat = _quat;
	} 
	else if (_sync&ST) // convert from swing-twist
	{	GS_TRACE1("st->lquat");
		_st->get(_lquat);
	} 
	else if (_sync&EU) // convert from euler
	{	GS_TRACE1("eu->lquat");
		_euler->get(_lquat);
	}

	GS_FLAG_ON(_sync,LQ); // local quat is now in sync
	if ( insync(sr)!=0 ) // if the request is LQ no need to convert to another format
	{	GS_TRACE1("syncto ended: _sync==" << FLAGST(_sync));
		return;
	}
  
	bool enflim = _joint->skeleton()->enforce_rot_limits();
   
	// 2. Sync _quat to the asked parameterization:
	GsQuat q;
	char newsync = _sync; // newsync is needed since the set methods will override the flag internally
	if ( sr==FQ ) 
	{	GS_TRACE1("lquat->quat");
		_quat = _lquat; // no pre-post quat is the same as local
		if (_prepost)
		{	_quat = _prepost->apply(_quat);
			_quat.normalize();
		}
		GS_FLAG_ON(newsync,FQ); // accumulate flag
	} 
	else if ( sr==ST )
	{	GS_TRACE1("lquat->st");
		_st->set(_lquat); // ST owns flag internally
		if ( enflim )
		{	_st->get(q); 
			if ( q!=_lquat ) // checks if the limits were enforced
			{	_lquat=q;	 // we need to reapply to _lquat
				GS_FLAG_OFF(newsync,FQ); // full is now not up to date
			}
		}
		GS_FLAG_ON(newsync,ST); // accumulate flag
	}
	else if ( sr==EU ) 
	{	GS_TRACE1("lquat->eu");
		_euler->set(_lquat); // EU now owns flag internally
		if ( enflim )
		{	_euler->get(q); 
			if ( q!=_lquat ) // checks if the limits were enforced
			{	_lquat=q;	 // we need to reapply to _lquat
				GS_FLAG_OFF(newsync,FQ); // full is now dirty
			}
		}
		GS_FLAG_ON(newsync, EU); // accumulate flag
	}

	// restore flag with the accumulated conversion
	_sync = newsync; 

	GS_TRACE1 ( "syncto ended: _sync=="<<FLAGST(_sync) );
}

//============================ End of File ============================
