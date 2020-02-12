/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <math.h>

# include <sig/gs_math.h>
# include <sigkin/kn_joint_rot.h>

# define INSYNC _jq->insync(KnJointRot::ST)
# define SYNC   _jq->syncto(KnJointRot::ST)
# define NEWVAL _jq->setowner(KnJointRot::ST)

//============================= KnJointST ============================

KnJointST::KnJointST ( KnJointRot* jq )
{
	_jq = jq;
	init();
}

void KnJointST::init ()
{
	_sx = _sy = 0;
	_ex = _ey = gspi;
	_twist = _lolim = _uplim = 0;
	_limit_twist = 1;
	NEWVAL;
}

void KnJointST::init ( const KnJointST* st )
{ 
	_sx = st->_sx;
	_sy = st->_sy;
	_ex = st->_ex;
	_ey = st->_ey;
	_twist = st->_twist;
	_lolim = st->_lolim;
	_uplim = st->_uplim;
	_limit_twist = st->_limit_twist;
}

bool KnJointST::swing_inlimits ( float x, float y )
{
	SYNC;
	float cx = x/_ex;
	float cy = y/_ey;
	float f = cx*cx + cy*cy - 1;
	return f<=0;
}

void KnJointST::swing ( float x, float y )
{
	if ( x==_sx && y==_sy && INSYNC ) return; // no change
   
	float cx = x/_ex;
	float cy = y/_ey;
	float f = cx*cx + cy*cy - 1;

	if ( f>0 ) gs_get_closest_on_ellipse ( _ex, _ey, x, y );
	_sx = x;
	_sy = y;

	NEWVAL;
}

float KnJointST::swingx ()
{
	_jq->syncto(KnJointRot::ST);
	return _sx;
}

float KnJointST::swingy ()
{
	_jq->syncto(KnJointRot::ST);
	return _sy;
}

void KnJointST::ellipse ( float rx, float ry )
{
	rx = GS_BOUND ( rx, 0.00001f, gspi );
	ry = GS_BOUND ( ry, 0.00001f, gspi );
	if ( rx==_ex && ry==_ey ) return; // no change
	SYNC;
	_ex = rx;
	_ey = ry;
	_sx = _sx+1; // change only to ensure that the swing values will be updated
	swing ( _sx-1, _sy );
}

void KnJointST::twist ( float t )
{
	if ( _twist==t && INSYNC ) return; // no change
   
	if ( _limit_twist )
		_twist = GS_BOUND(t,_lolim,_uplim);
	else
		_twist = t;
	NEWVAL;
}

float KnJointST::twist ()
{
	SYNC;
	return _twist;
}

void KnJointST::twist_limits ( float min, float max )
{
	if ( _lolim==min && _uplim==max ) return; // no change
	if ( min>max ) min = max = (max-min)/2;
	SYNC;
	_lolim = min;
	_uplim = max;
	float t = GS_BOUND(_twist,_lolim,_uplim);
	twist ( t );
}

void KnJointST::twist_freeze ()
{
	SYNC;
	_lolim = _uplim = _twist;
	_limit_twist = 1;
}

bool KnJointST::twist_frozen () const
{
	if ( !_limit_twist ) return false;
	return _lolim==_uplim;
}

void KnJointST::get_random_swing ( float& x, float& y ) const
{
	float a = gs_random ( 0.0f, gs2pi ); // angle around ellipse
	float t = gs_random (); // linear interp from origin to ellipse border
	x = t * _ex * cosf(a);
	y = t * _ey * sinf(a);
}

float KnJointST::get_random_twist () const
{
	return gs_random(_lolim,_uplim);
}

void KnJointST::get ( GsQuat& q ) const
{
	SYNC;
	GsVec v ( _sx, _sy, 0.0f );
	q.set ( v );
	if ( _twist!=0 )
	{	GsQuat twist ( GsVec::k, _twist );
		q = q * twist;
	}
}

void KnJointST::set ( const GsQuat& q )
{
	float sx, sy, tw;
	quat2st ( q, sx, sy, tw );
	swing ( sx, sy );
	twist ( tw );
}
 
//============================ End of File ============================
