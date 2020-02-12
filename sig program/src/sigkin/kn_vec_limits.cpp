/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigkin/kn_vec_limits.h>
# include <sig/gs_vec.h>

//============================= KnVecLimits ============================

void KnVecLimits::init ()
{ 
	for ( int i=0; i<3; i++ )
	{	_value[i] = 0;
		_uplim[i] = 0;
		_lolim[i] = 0;
		_limits[i] = 1;
	}
}

void KnVecLimits::init ( const KnVecLimits& v )
{
	for ( int i=0; i<3; i++ )
	{	_value[i] = v._value[i];
		_uplim[i] = v._uplim[i];
		_lolim[i] = v._lolim[i];
		_limits[i] = v._limits[i];
	}
}

bool KnVecLimits::value ( int d, float val )
{
	if ( _limits[d] ) { val = GS_BOUND(val,_lolim[d],_uplim[d]); }
	if ( val==_value[d] ) return false;
	_value[d] = val;
	return true;
}

void KnVecLimits::value ( const GsVec& v )
{
	_value[0] = _limits[0]? GS_BOUND(v.x,_lolim[0],_uplim[0]) : v.x;
	_value[1] = _limits[1]? GS_BOUND(v.y,_lolim[1],_uplim[1]) : v.y;
	_value[2] = _limits[2]? GS_BOUND(v.z,_lolim[2],_uplim[2]) : v.z;
}

float KnVecLimits::random ( int d ) const
{
	if ( _limits[d] )
		return gs_random ( _lolim[d], _uplim[d] );
	else
		return gs_random ( -gspi, gspi );
}
 
int KnVecLimits::nfrozen () const
{
	return frozen(0) + frozen(1) + frozen(2);
}

//============================ End of File ============================
