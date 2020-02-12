/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_CHANNELS
# define KN_CHANNELS

# include <sig/gs_buffer.h>
# include <sig/gs_shareable.h>
# include <sigkin/kn_channel.h>

class KnPosture;

/*! KnChannels manipulates an array of channels and is used to
	specify postures and motions. */
class KnChannels : public GsShareable
{  protected :
	class HashTable;
	GsArray<KnChannel> _channels;
	mutable HashTable* _htable;
	int _floats;

   public :

	/*! Constructor */
	KnChannels ();

	/*! Copy Constructor */
	KnChannels ( const KnChannels& c );

	/*! Destructor is public but be sure to check if unref() has to be used */
   ~KnChannels ();

	/*! Set the array as empty */
	void init ();

	/*! Compress internal tables */
	void compress () { _channels.compress(); }

	/*! Returns the number of channels in the array */
	int size () const { return _channels.size(); }

	/*! Add in the end of the array a new channel, by giving
		only its joint name and type; the channel is created disconnected */
	void add ( KnJointName jname, KnChannel::Type t ) { _add(0,t); _channels.top().jname(jname); }

	/*! Add in the end of the array a new channel connected to given joint */
	void add ( KnJoint* j, KnChannel::Type t ) { _add(j,t); }

	/*! Same as add(ch.joint,ch.type) */
	void add ( const KnChannel& ch ) { _add(0,ch.type()); _channels.top()=ch; }

	/*! Inserts a channel in the array at given position. The channel is
		inserted as unconnected.
		True is returned in case of success, otherwise the requested position
		is out of range and false is returned. */
	bool insert ( int pos, KnChannel::Type type=KnChannel::XPos );

	/*! Same as the other insert() method for inserting an unconnected channel in the array,
		but here also providing the associated joint name */
	bool insert ( int pos, KnJointName jname, KnChannel::Type type=KnChannel::XPos )
		 { if ( !insert(pos,type) ) return false; _channels[pos].jname(jname); return true; }

	/*! Access operator */
	KnChannel& operator[] ( int i ) { return _channels[i]; }

	/*! Const access operator */
	KnChannel& operator() ( int i ) const { return _channels[i]; }

	/*! Access operator */
	KnChannel& get ( int i ) { return _channels[i]; }

	/*! Const access operator */
	const KnChannel& cget ( int i ) const { return _channels[i]; }

	/*! Returns the last channel added */
	KnChannel& top () { return _channels.top(); }

	/*! Returns the skeleton associated with the joint connected to the first channel
		or null if there is no joint connected. */
	KnSkeleton* skeleton () const;

	/*! Recalculates and stores the number of floats required to store all the
		channels. Note that it is invalidated each time a channel type is
		changed without using the KnChannels methods. */
	int count_floats ();
	
	/*! Returns the number of floats required to store all the channels. */
	int floats () const { return _floats; }
	
	/*! Counts and returns the float position of the 1st value used by channel c */
	int floatpos ( int c ) const;

	/*! Put in the channel connection the floats specified in src */
	void apply ( const float* src );

	/*! Put in the channel indexed connection the floats in src; indices starting at destbase */
	void apply ( const float* src, float *destbase );

	/*! Put in fp all the floats specified by the connection of the channel array */
	void get ( float* fp ) const;
   
	/*! Get random values by calling method KnChannel::get_random() for each channel */
	void get_random ( float* fp ) const;
	
	/*! Builds the channels from the active joints in the given skeleton */
	void make ( KnSkeleton* kn );

	/*! Make a channel array by extracting the used channels in the given list of postures,
		which has to be composed of postures using the same number of channels.
		Only the channels with a different value in relation to the first
		posture are included.
		The array pointer 'indices', if not null, will translate the indices of the original
		float values in relation to the channels used by postures to the created new channels,
		eg, newval[index[i]] will be origval[i], if index[i]>=0.
		True is returned in case of success (parameters are correct), false otherwise. */
	bool make ( const GsArray<KnPosture*>& postures, GsArray<int>* indices=0 );
	
	/*! Returns the channel position of given channel type and joint name, or -1 if not found.
		This method uses a linear search, not relying in the internal hash table option.
		If float pos is given, it will return the float index of that channel. */
	int lsearch ( KnJointName jname, KnChannel::Type type, int* floatpos=0 ) const;

	/*! Returns the channel position of given channel type and joint name, or -1 if not found.
		This method uses an internal hash table to make the search practically O(1); however the
		hash table must be up to date. If the hash table is not built, it will automatically be
		built, but if the channel array is changed after that, it is the user responsibility
		to manually call method rebuild_hash_table(). */
	int search ( KnJointName name, KnChannel::Type type ) const;

	/*! An internal hash table is used to optimize method search() and the methods
		for posture connection. Whenever the channel array is edited, the user must
		call this method to ensure that the internal hash table is up to date. 
		This method can be declared as const because the internal hashtable is mutable. */
	void rebuild_hash_table () const;

	/*! Matches the channel names with the joint names in the given skeleton,
		storing the corresponding joint pointers in the channels. Joints
		not matched will result in null joint pointers in the channel array.
		The number of channels matched is returned. A null
		parameter will result in a call to disconnect().
		This method is efficient since it uses a hash table to match joints */
	int connect ( const KnSkeleton* s );

	/*! Matches the channel joint names with the joint names in the channels of
		the given posture p, connecting the channels to the corresponding float pointers
		for direct access to the values in the posture.
		Joints not matched will result in null float pointers in the channel array.
		The number of channels matched is returned.
		A null parameter will result in a call to disconnect().
		This method is efficient since it uses a hash table to match joints */
	int connect ( const KnPosture* p );
	
	/*! Matches the channel joint names with the joint names in the channels of
		the given posture p, storing indices for mapping to other classes channels
		with the same channel description.
		The number of channels matched is returned.
		A null parameter will result in a call to disconnect().
		This method is efficient since it uses a hash table to match joints */
	int connect_indices ( const KnPosture* p );

	/*! Simply calls connect(0) */
	void disconnect ();

	/*! Returns number of channels connected */
	int count_connected () const;

	/*! Returns number of channels disconnected */
	int count_disconnected () const;

	/*! Adds to the channel array all channels that are in ca,
		but are not in the channel array. The "floats counter" is updated. */
	void merge ( const KnChannels& ca );

	/*! Returns true if all channels are of KnChannel::Quat type, and false otherwise. */
	bool only_quat_channel () const;

	/*! Set all channels to be of KnChannel::Quat type and returns the number of channels
		that had a different type. All channels that had a connection different than 
		KnChannel::JointConnection are disconnected. */
	int force_quat_channels ();

	/*! Method to print the internal hash table */
	void htableout ( GsOutput& o ) const;

	/*! Copy all fields from a. */
	void copyfrom ( const KnChannels& c );

	/*! Copy operator copies all fields using copyfrom(). */
	void operator = ( const KnChannels& c ) { copyfrom(c); }
	
	/*! Comparison operator checks if the channel names and types are the same. */
	bool operator == ( const KnChannels& c );

	/*! Output operator */
	friend GsOutput& operator<< ( GsOutput& o, const KnChannels& c );

	/*! Input operator */
	friend GsInput& operator>> ( GsInput& in, KnChannels& c );
	
   private:
	void _add ( KnJoint* j, KnChannel::Type t );
};

//==================================== End of File ===========================================

# endif  // KN_CHANNELS
