/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_CHANNEL_H
# define KN_CHANNEL_H

# include <sigkin/kn_joint.h>

/*! KnChannel specifies a channel type, a joint name, and its connection
	status. A channel can be seen as an extension of a DOF
	to more complex parameterizations of a group of DOFs. 
	For example, a channel can be a (Euler) rotation around one axis,
	a swing rotation, a full quaternion, etc. Possible channels
	are enumerated in the enum Type.
	Note that KnChannel only specifies a channel and does not maintain
	any joint values. */
class KnChannel
{  public :
	/*! Possible types for a channel. Quat requires 4 floats, Swing 2 floats,
		IKPos 3 floats, IKGoal 7 floats (pos+quat), and all the others require 1 float to
		be fully represented (enum order must not be modified). */
	enum Type { XPos, YPos, ZPos, XRot, YRot, ZRot, Quat, Swing, Twist, IKPos, IKGoal, Invalid };

	/*! The connection status of the channel: two connections are possible,
		direct connection to a skeleton joint, or to a buffer of float values as
		the one maintained in KnPosture. Order in enumerator cannot be modified. */
	enum Status { Disconnected, JointConnection, BufferConnection, IndexConnection };

   private :
	KnJointName _jname; //<! The name of this channel's joint
	char _type;			//<! The channel type
	char _status;		//<! The channel connection status
	union { KnJoint* j; float* f; int i; } _con; //<! The connected joint, buffer pointer or buffer index, if connected

   public :

	/*! Default constructor creates an Disconnected channel */
	KnChannel ( Type t=XPos ) { init(t); }

	/*! Copy constructor */
	KnChannel ( const KnChannel& c ) { *this=c; }

	/*! Constructor connecting the channel of given type to the given joint,
		retrieving from the joint its name */
	KnChannel ( KnJoint* j, Type t=XPos ) { _type=t; connect(j); }

	/*! Constructor connecting the channel of given type to the given float buffer */
	KnChannel ( float* f, Type t=XPos ) { _type=t; connect(f); }

	/*! Connects the channel to the given joint,
		and also updates the channel joint name from j->name().
		If a null joint is sent, the joint is disconnected.
		Note that the channel type is not set (it is left unchanged).
		As a joint may have several channels, the automatic detection of channels
		only makes sense when building a list of channels, for ex with KnChannels::make(). */
	void connect ( KnJoint* j );

	/*! Conects the channel to the given float buffer */
	void connect ( float* f );

	/*! Conects to a channel array index. */
	void connect ( int i );

	/*! Disconnects the channel (the joint name and type is not changed) */
	void disconnect () { _status=Disconnected; _con.j=0; }

	/*! Set the channel to be Disconnected with optionally with given type.
		The associated joint name is not changed. */
	void init ( Type t=XPos );

	/*! Returns the type of this channel */
	Type type () const { return (Type)_type; }

	/*! Set the type of this channel */
	void type ( Type t ) { _type=t; }

	/*! Returns the connection status of this channel */
	Status status () const { return (Status)_status; }

	/*! Returns the name of the connected joint, KnJointName::undefined() will
		be true if the name has not been defined */
	KnJointName jname () const { return _jname; }

	/*! Set the joint name associated to this channel */
	void jname ( KnJointName jname ) { _jname=jname; }

	/*! Returns the connected joint of the channel. This method will only return a
		valid pointer if the channel status is JointConnection. */
	KnJoint* joint () const { return _con.j; }

	/*! Returns the connected float buffer of the channel. This method will only return a
		valid pointer if the channel status is BufferConnection. */
	float* buffer () const { return _con.f; }

	/*! Returns the connected index of the channel. This method will only return a
		valid pointer if the channel status is IndexConnection. */
	int index () const { return _con.i; }

	/*! Returns the skeleton of the connected joint of the channel.
		If the channel is not connected to a joint, 0 is returned. */
	KnSkeleton* skeleton () const;

	/*! Put values in the connection associated to the channel (a joint or buffer).
		The number of actual floats inserted in the connection is returned (can be 1, 2 or 4).
		If not connected, nothing is done and the returned value corresponds to the
		channel type size (1, 2, or 4).
		(Note: apply will not work in IndexConnection mode) */
	int apply ( const float* src ) const;

	/*! Version for IndexConnection mode */
	int apply ( const float* src, float* destbase ) const;

	/*! Get the channel values from the associated connection and put them in v.
		The number of actual floats inserted in v is returned (can be 1, 2 or 4).
		If not connected, 0 is inserted in v and the returned value corresponds to the
		channel type size (1, 2, or 4).
		(Note: apply will not work in IndexConnection mode) */
	int get ( float* v ) const;

	/*! If the channel is connected to a joint, then this method will put in v
		random values according to the connected joint limits and channel type.
		The number of actual floats inserted in v is returned (1, 2 or 4).
		If not connected or connected to a float buffer, this method simply calls get() */
	int get_random ( float* v ) const;

	/*! Returns in v the interpolation between values v1 and v2,
		according to the channel type. Static method.
		Returns the size of the channel being interpolated */
	static int interp ( Type type, const float* v1, const float* v2, float t, float* v );

	/*! Member function version of interp(t,v1,v2,t,v) */
	int interp ( const float* v1, const float* v2, float t, float* v ) const
		{ return interp ( (Type)_type, v1, v2, t, v ); }

	/*! Returns in v the weighted interpolation of the several values given in array v,
		with weights given in array w. The intrpolation is done according to the channel
		type. Static method. Returns the size of the channel being interpolated. */
	static int interp ( Type type, const GsArray<float*>& values, const GsArray<float>& w, float* v );

	/*! Member function version of interp(type,values,w,v) */
	int interp ( const GsArray<float*>& values, const GsArray<float>& w, float* v ) const
		{ return interp ( (Type)_type, values, w, v ); }

	/*! The given values are multiplied by mfactor and then 'added' with offset.
		If mfactor is 0, offset becomes the new value. For the Swing and Quat 
		types of channels, mfactor acts as a multiplication of the current 
		rotational angle encoded in the rotation. Offset and values need to
		contain up to 4 valid positions, according to the channel type. 
		Returns the size of the channel type being manipulated. */
	static int change_values ( Type type, float* values, float mfactor, const float* offset );

	/*! Member function version of change_values() */
	int change_values ( float* values, float mfactor, const float* offset )
		{ return change_values ( (Type)_type, values, mfactor, offset ); }

	/*! The size of a channel can be 1, 2(Swing), or 4 (Quat) */
	static int size ( Type type ) { return type==Quat? 4 : type==IKPos? 3 : type==Swing? 2 : type==IKGoal? 7:1; }

	/*! Member function version of size() */
	int size () const { return size((Type)_type); }
	
	/*! Returns a static string with the name of the channel type, 
		eg, "XPos", etc. If c is 'out of range', string "Undef" is returned */
	static const char* type_name ( Type type );

	/*! Member function version of type_name() */
	const char* type_name () const { return type_name((Type)_type); }

	/*! Returns the type of the channel according to the name in string s */
	static Type type ( const char* s );

	/*! Outputs the needed number of floats from v, according to type.
		Returns the processed channel type size, which can be different
		than the number of floats written in the Quat channel type,
		as a Quat is written in axis-angle 3 floats format. */
	static int save ( Type type, GsOutput& o, const float* v );
	
	/*! Member function version of save() */
	int save ( GsOutput& o, const float* v ) const { return save((Type)_type,o,v); }
	
	/*! Inputs the channel values to v according to type and returns
		the number of floats put in v */
	static int load ( Type type, GsInput& in, float* v ); // static

	/*! Member function version of load() */
	int load ( GsInput& in, float* v ) { return load((Type)_type,in,v); }

	/*! Comparison operator only checks if the channel names and types are the same. */
	bool operator == ( const KnChannel& c )
		 { return _type==c._type && _jname==c._jname; }

	/*! Comparison operator only considers the channel names and types. */
	bool operator != ( const KnChannel& c )
		 { return _type==c._type && _jname==c._jname? false:true; }
};

//==================================== End of File ===========================================

# endif  // KN_CHANNEL_H
