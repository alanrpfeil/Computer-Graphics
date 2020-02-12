/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_POSTURE_H
# define KN_POSTURE_H

# include <sig/gs_vec.h>
# include <sig/gs_buffer.h>

# include <sigkin/kn_channels.h>

class KnSkeleton;

//============================== KnPostureDfJoints =================================================

/*! Shared class that keeps the array of joints to be
	considered by the distance function in KnPosture */
class KnPostureDfJoints : public GsShareable
{  public :
	/*! Array containing the list of joints */
	GsArray<KnJoint*> joints;

	/*! Will read format "[N] jname1 ... jnameN;" and fill the array
		of joints with the pointers to the joints found in kn. */
	void input ( GsInput& in, const KnSkeleton* sk );

	/*! Will write in format "N jname1 ... jnameN;" */
	void output ( GsOutput& out ) const;

	/*! Returns array position of given joint name if found, or -1 otherwise */
	int lsearch ( const char* jname );
};

//================================ KnPosture =================================================

/*! KnPosture stores joint values specifying a posture and 3D points to be
	used by the distance function in the 'values' and 'joints' buffers.
	Two shared classes are referenced: a channel array describing 'values',
	and a KnPostureDfJoints describing how to update 'points'. */
class KnPosture : public GsShareable
{  public :
	GsBuffer<float> values; // values defining this posture
	GsBuffer<GsPnt> points; // articulation points used by the distance function
	
   private :
	char* _name;
	bool _syncpoints;
	KnChannels* _channels;
	KnPostureDfJoints* _dfjoints;

   public :
	/*! Default constructor initializes an empty, not usable posture.
		An init() method must be properly called at some point. */
	KnPosture ();

	/*! Copy constructor. KnChannels and KnPostureDfJoints are shared from p.
		The values, points and name are copied from p. */
	KnPosture ( const KnPosture& p, KnChannels* chstoshare=0 );

	/*! Constructor from a given channel array and (optionally) a dfjoint list,
		which will be both shared */
	KnPosture ( KnChannels* ca, KnPostureDfJoints* dfj=0 );

	/*! Constructor that will access and share the channel array of the given skeleton */
	KnPosture ( KnSkeleton* s );

	/*! Destructor. Be sure to access it through unref() when needed. */
   ~KnPosture ();

	/*! Set the posture empty and disconnected and thus not usable (name is not changed) */
	void init ();
 
	/*! Set the posture as empty and initializes it by referencing the given channels */
	void init ( KnChannels* ca );

	/*! Set the posture as empty and initializes it from both the given
		channel array and distance function joints */
	void init ( KnChannels* ca, KnPostureDfJoints* ja );

	/*! Set the posture as empty and references the active channels of the given skeleton. */
	void init ( KnSkeleton* s );

	/*! Returns the referenced channel array (which should not be modified).
		Remember that the channels here are shared, using ref()/unref(). */
	KnChannels* channels () { return _channels; }
	const KnChannels* cchannels () const { return _channels; }

	/*! Makes the posture to reference the given channels, and unreference the original one.
		The new channel must be compatible with the old one, no checkings are done. */
	void channels ( KnChannels* c );

	/*! Allocates and connects a new KnChannels class accessing the same joints but with all channels
		set to type KnChannel::Quat. The size of the values array will be adjusted accordingly. 
		Returns the number of channels that had a different type, or -1 if no channels exist. */
	int make_quat_channels ();

	/*! Connects the posture channels to the given skeleton */
	void connect ( const KnSkeleton* kn ) { _channels->connect(kn); }

	/*! Connects the posture channels to another given posture p with direct pointers */
	void connect ( const KnPosture* p ) { _channels->connect(p); }

	/*! Connects the posture channels to another given posture p by indices,
		allowing to reuse indices for transfering values to any posture with
		same channels as p. However, note that in this case motion application
		can only be performed with the special method apply ( KnPosture* ) */
	void connect_indices ( const KnPosture* p ) { _channels->connect_indices(p); }

	/*! Returns the skeleton associated with the channel array (can be null) */
	KnSkeleton* skeleton () const { return _channels->skeleton(); }

	/*! Get the name, can be an empty string, but not a null pointer */
	const char* name () const { return _name? _name:""; }
	
	/*! Set a name */
	void name ( const char* n );

	/*! Get and store in values the current values of the connected skeleton or posture,
		but calling the KnChannels::get() method. */
	void get ();

	/*! Get a random configuration according to the joint types and limits.
		Calls method KnChannels::get(). */
	void get_random ();

	/*! Apply the posture values to the connected joints or posture.
		When applying to a connected skeleton, its global matrices remain not updated. */
	void apply () const { _channels->apply(&values[0]); }

	/*! Apply the posture to the given posture using indices previously mapped
		with connect_indices(). */
	void apply ( KnPosture* p ) const { _channels->apply(&values[0],&p->values[0]); }

	/*! Inserts a new channel at the given position in the associated KnChannels, and
		also inserts the corresponding space in the values array, copying the values
		in fvalues (if not null) to the new space. True is returned in case of success */
	bool insert ( int pos, KnChannel::Type type=KnChannel::XPos, float* fvalues=0 );

	/*! Set specific joints to be considered by the distance function,
		otherwise, all joints of the channel array are used.
		Given dfjoints is a shared class. Null can be passed to disconsider any dfjoints. */
	void dfjoints ( KnPostureDfJoints* dfjoints );

	/*! Returns the referenced array of distance function joints (or null). */
	KnPostureDfJoints* dfjoints () { return _dfjoints; }

	/*! Force to recompute the distance function points next time they are needed */
	void unsyncpoints () { _syncpoints=false; }

	/*! Returns true if the distance function points are in sync with the cur posture */
	bool syncpoints () const { return _syncpoints; }

	/*! To make means to (1) apply the posture to and (2) update the global matrices
		of the associated skeleton, to then (3) get the global coordinates of each
		joint defined to be used in the distance function.
		If no distance function joints are defined, all the joints in the channel
		array are taken (duplicated joints have no effect as we use the max norm).
		Method update_gmat() is called for the given joint j, or in case it is null
		(default), all the skeleton connected to the channel array is updated.
		This function is automatically called when needed, usually the user does
		not need to call it. 
		If the posture is not connected to a skeleton, this method does nothing. */
	void make_points ( KnJoint* j=0 );

	/*! mirror each joint in the posture. This method assumes the posture is connected to
		a skeleton and symmetric joints are consistently labeled using keywords left and right */
	void mirror ( const char* left, const char* right, bool printerrors=false );

	/*! Copy operator copies all internal data, but does not copy the name. */
	void operator= ( const KnPosture& p );

	/*! Put in p the interpolation between p1 and p2 at the parametric position t in [0,1].
		All given postures must have compatible existing referenced channel arrays. 
		Posture p1 does not need to be connected to p2, and all channels are processed. */
	friend void interp ( const KnPosture& p1, const KnPosture& p2, float t, KnPosture& p );

	/*! This interpolation function assumes that the channels of posture p are connected to 
		some other posture p2 and only the succesfully connected channels are processed.
		This function assumes the connection to p2 is by float pointers, and not indices.
		Posture p will be interpolated with posture p2, and the result will be placed in p2.
		Posture p2 is not needed as an argument since only the connection pointers are used. */
	friend void interp ( const KnPosture& p, float t );

	/*! Put in p the weighted interpolation of all the postures in array postures, using
		the weights in array w. Array buffer is needed to optimize computations across several calls.
		All given postures must have compatible existing referenced channel arrays */
	friend void interp ( const GsArray<KnPosture*>& postures, const GsArray<float>& w,
						 KnPosture& p, GsArray<float*>& buffer );

	/*! Returns the maximum Euclidian distance between the corresponding points in
		the 'points' buffer field. If changes were made to the postures p1 and/or p2,
		their 'points' buffer are automatically rebuilt with make_points(). */
	friend float dist ( KnPosture& p1, KnPosture& p2 );

	/*! Custom output of posture data */
	void output ( GsOutput& out, bool channels, bool values, bool onelineval=true ) const;

	/*! Write all data of the posture using output() (with all options to true) */
	friend GsOutput& operator<< ( GsOutput& out, const KnPosture& p );

	/*! Read the posture, converting the angles to radians. As only angle values
		are saved/read, the points array of p are left not up to date */
	friend GsInput& operator>> ( GsInput& inp, KnPosture& p );
};

//================================ End of File =================================================

# endif // KN_POSTURE_H
