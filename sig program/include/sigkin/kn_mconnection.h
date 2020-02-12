/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution.
  =======================================================================*/

# ifndef KN_MCONNECTION_H
# define KN_MCONNECTION_H

# include <sigkin/kn_motion.h>

class KnMotion;
class KnSkeleton;

/*! Creates a new connection from an existing KnMotion to a new
	KnChannels without changing the original connection state of the KnMotion. */
class KnMConnection : public GsShareable
 { protected :
	KnMotion* _motion;		// source motion
	KnChannels* _channels;	// target channels
	int _last_apply_frame;  // used to speed up playing with monotone time

   public :
	/*! Constructor will connect m to c without changing the connection state of m */
	KnMConnection ( KnMotion* m );

	/*! Constructor will connect m to c without changing the connection state of m */
	KnMConnection ( KnMotion* m, KnSkeleton* sk ) : KnMConnection(m) { connect(sk); }

	/*! Destructor is public but pay attention to the use of ref()/unref() */
	virtual ~KnMConnection();

	/*! Connects the internal channels to the given skeleton */
	void connect ( KnSkeleton* sk ) { _channels->connect(sk); }

	/*! Returns the KnChannels of this connection */
	KnChannels* channels () const { return _channels; }

	/*! Returns the skeleton connected to the channels
		or null if there is no skeleton connected. Same as channels()->skeleton() */
	KnSkeleton* skeleton () const { return channels()->skeleton(); }

	/*! Apply frame f to the connected channels. Equivalent to KnMotion::apply_frame() */
	void apply_frame ( int f );

	/*! Evaluates and applies the motion at time t to the connected channels.
		Equivalent to KnMotion::apply_frame() */
	void apply ( float t, KnMotion::InterpType itype=KnMotion::Linear, int* lastframe=0 )
		{ KnMotion::apply(_motion,_channels,_last_apply_frame,t,itype,lastframe); }
};

//================================ End of File =================================================

# endif  // KN_MCONNECTION_H
