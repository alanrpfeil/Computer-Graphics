/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

//# include <math.h>
//# include <stdlib.h>
//# include <string.h>

# include <sigkin/kn_mconnection.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_posture.h>

//============================= KnMConnection ============================

KnMConnection::KnMConnection ( KnMotion* m )
{
	_motion = m;
	_motion->ref();

	_channels = new KnChannels;
	_channels->copyfrom ( *_motion->channels() );
	_channels->ref();

	_last_apply_frame = 0;
}

KnMConnection::~KnMConnection()
{
	_channels->unref();
	_motion->unref();
}

void KnMConnection::apply_frame ( int f )
{
	int fs = _motion->frames();
	if ( f>=fs ) f=fs-1;
	if ( f<0 ) f=0;
	_channels->apply ( _motion->posture(f)->values.pt() );
}
