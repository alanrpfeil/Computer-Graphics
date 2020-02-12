/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution.
  =======================================================================*/

# ifndef KN_MOTION_H
# define KN_MOTION_H

# include <sig/gs_array.h>
# include <sig/gs_input.h>
# include <sig/gs_shareable.h>
# include <sigkin/kn_channels.h>
# include <sigkin/kn_posture.h>

class GsVars;
class KnSkeleton;

/*! Maintains a motion defined as a sequence of keyframes, each
	keyframe represented by a time stamp and a posture.
	All postures share the same KnChannels, which describes the channels
	used by the motion. */
class KnMotion : public GsShareable
 { protected :
	struct Frame { float keytime; KnPosture* posture; };
	GsArray<Frame> _frames;   // frame data
	GsCharPt _name;			  // motion name
	GsCharPt _filename;		  // file name
	int   _last_apply_frame;  // used to speed up playing with monotone time
	float _freq;			  // sampling rate
	GsVars* _userdata;		  // to store user data

   public :
	/*! Constructor */
	KnMotion();

	/*! Destructor is public but pay attention to the use of ref()/unref() */
	virtual ~KnMotion();

	/*! Set a name to be associated with the motion */
	void name ( const char* n ) { _name.set(n); }

	/*! Get the name associated with the motion */
	const char* name () const { return _name.pt? _name.pt:""; }

	/*! Set a file name to be associated with the motion. The name is not saved
		in the motion file and is not used by KnMotion. It is here just as a
		convenient place to store the information */
	void filename ( const char* n ) { _filename.set(n); }

	/*! Get the file name associated with the motion */
	const char* filename () const { return _filename.pt? _filename.pt:""; }

	/*! Clears frame data, creating an empty motion. Name and filename are not changed. */
	void init ();

	/*! Compress internal arrays */
	void compress ();

	/*! Loads a motion file and returns true if no errors.
		Both .sm and .bvh formats are read here. The filename is updated. */
	bool load ( const char* filename );

	/*! Loads a motion file and returns true if no errors.
		Both .sm and .bvh formats are read here.
		The filename is updated if there is one in the input. */
	bool load ( GsInput& in );

	/*! Loads a bvh fil and return true if no errors. */
	bool load_bvh ( GsInput& in );

	/*! Save the motion to a file and returns true if no errors. */
	bool save ( const char* filename );

	/*! Save the motion to a file and returns true if no errors */
	bool save ( GsOutput& out );

	/*! Save the motion to a file in BVH format and returns true if no errors.
		Channels to be saved are get from the skeleton channel definitioin.
		A skeleton has to be attached to the motion as bvh contains skeleton definition.
		False is returned if the file could not be written or if there is no skeleton attached.
		note: bvh frames are interpolated at each key time defined by fps.
		MotionBuider friendly level: 0: not friendly;
									 1: omit channels for child-less joint;
									 2: also rename child-less joints to "End Site". */
	bool save_bvh ( const char* filename, int fps=30, unsigned int MB_friendly=0 );

	/*! Clears frame data, creating a motion with the given keytimes and postures.
		All given KnPostures in keypost will be simply referenced by KnMotion.
		The postures must share a same KnChannels and both given arrays must be of
		same size. True is returned if the motion could be created, false otherwise */
	bool makeasref ( const GsArray<KnPosture*>& keypost, const GsArray<float>& keytime );

	/*! Makes a motion by defining the frames with given keypostures and keytimes.
		A new KnChannels is created to be used by all frames, and only the channels
		with a variation in value in relation to the first keyposture are created.
		Arrays keypostures and keytimes must have the same size.
		True is returned if the motion could be created, false otherwise. */
	bool make ( const GsArray<KnPosture*>& keypostures, const GsArray<float>& keytimes );

	/*! Inserts one channel at channel position i, with the given joint name,
		type and value. This implies inserting new position(s) in all frames.
		Returns true if all parameters are ok.
		Note that the channel will only take effect after reconnection */
	bool insert_channel ( int pos, KnJointName jname, KnChannel::Type type, float* fvalues=0 );

	/*! Inserts frame p at position pos, with keytime kt. Posture p is simply
		referenced. True returned if success, false if pos is invalid.
		To insert a frame at the end of the motion, use pos as the number of frames. */
	bool insert_frame ( int pos, float kt, KnPosture* p );
	
	/*! Calls insert_frame() to add the posture at the last frame position. */
	bool add_frame ( float kt, KnPosture* p ) { return insert_frame(frames(),kt,p); }

	/*! Removes frame at position index pos. */
	bool remove_frame ( int pos );

	/*! Returns the KnChannels shared by the key postures, or null if the motion is empty */
	KnChannels* channels () const { return _frames.size()? posture(0)->channels():0; }

	/*! Number of frames in the motion */
	unsigned int frames () const { return _frames.size(); }

	/*! The size of the float array representing one posture. */
	int postfloats () const { return _frames.size()<=0? 0 : posture(0)->values.size(); }

	/*! Returns the KnPosture of frame f. It is the user
		responsibility to ensure that 0<=f and f<frames()  */
	KnPosture* posture ( int f ) const { return _frames[f].posture; }

	/*! Set the joints to be considered by the distance function in all posture frames,
		ie, simply calls dfjoints() for all list of key postures */
	void dfjoints ( KnPostureDfJoints* dfjoints );

	/*! Returns the keytime of frame f. It is the user
		responsibility to ensure that 0<=f and f<frames() */
	float keytime ( int f ) const { return _frames[f].keytime; }

	/*! Set a new keytime for frame f. It is the user
		responsibility to ensure that 0<=f and f<frames() */
	void keytime ( int f, float kt ) { _frames[f].keytime = kt; }

	/*! Returns the keytime of the last (final) frame, ie, of index frames()-1 */
	float last_keytime () const { return _frames.top().keytime; }

	/*! Returns the final keytime of the motion minus the first one.
		This method requires the motion to not be empty. */
	float duration () { return _frames.top().keytime-_frames[0].keytime; }

	/*! Returns the sampling rate in case the motion was imported from a bvh file.
		For instance if the motion was sampled at 30Hz, 0.3333 is returned. */
	float freq() const { return _freq; }

	/*! Set sampling rate. */
	void set_freq ( float freq ) { _freq = freq; }

	/*! Returns the last applied frame */
	int last_applied_frame() const { return _last_apply_frame; }

	/*! Connects the keypostures' shared channels to the given skeleton,
		establishing a direct link between each channel in the motion
		to the corresponding channels in the skeleton's joints.
		The number of channels matched is returned.
		A null parameter will disconnect the motion. */
	int connect ( const KnSkeleton* s );

	/*! Connects the keypostures' shared channels to the given posture,
		establishing a direct link between each channel in the motion
		to the corresponding float values in the posture.
		The number of channels matched is returned.
		A null parameter will disconnect the motion. */
	int connect ( const KnPosture* p );

	/*! disconnect the motion */
	void disconnect ();

	/*! Returns the skeleton connected with the first channel
		or null if there is no skeleton connected. Same as channels()->skeleton() */
	KnSkeleton* skeleton () const { return channels()->skeleton(); }

	/*! Apply frame f to the attached skeleton or posture.
		Only sucessfully matched channels are affected.
		Parameter f can be out of range for specifying extreme postures. */
	void apply_frame ( int f );

	/*! Type of interpolation used by method apply(t) */
	enum InterpType { Linear, CubicSpline };

	/*! static-version of apply function for expert use only (use member version below of apply() instead) */
	static void apply ( KnMotion* m, KnChannels* c, int& lastf, float t, KnMotion::InterpType itype, int* lastframe );

	/*! Evaluates and applies the motion at time t to the connected skeleton or posture.
		The search for the 2 keyframes to be interpolated (adjacent to t) is done linearly,
		however as most of the cases the evaluation is monotone, the previous frame number
		used serves as a starting point for the search, resulting in maximum efficiency.
		To optimize evaluations from several controllers sharing a same motion file,
		parameter lastframe can be used to maintain the starting frame to be considered.
		Note: make sure joint limits are properly set in the skeleton, for instance, joints
		with Euler angles will by default be frozen with value 0 */
	void apply ( float t, InterpType itype=Linear, int* lastframe=0 )
		{ apply(this,channels(),_last_apply_frame,t,itype,lastframe); }

	/*! Returns a string describing the interpolation type */
	static const char* interp_type_name ( InterpType type );

	/*! Returns the type relative to the string description */
	static InterpType interp_type_name ( const char* type );

	/*! Copy operator. Copies all motion data from m.
		Note that here the postures are copied, not referenced; and even a new
		copy of the channels is created and the same for KnPostureDfJoints if applicable.  */
	void operator = ( const KnMotion& m );

	/*! Move all keytimes so that the first keytime starts at the given time */
	void move_keytimes ( float startkt );

	/*! Change the angle values of all channels in euler angles type by
		adding +-2PI, in order to have the smallest distance between frames,
		e.g., to obtain interpolation from 0 to -60, instead of 0 to 300.
		Note that this might not work depending on the joint limits defined.
		Returns the number of Euler channels found in the channel array. */
	int correct_euler_angles ();

	/*! Change the values of the given channel index, from all frames from f1 to f2, according to
		the given multiplication factor and offset. First the actual channel value is multiplied
		by factor, and then added with offset. If mfactor is 0, offset becomes the new value of
		the channel. For the Swing and Quat types of channels, mfactor acts as a multiplication
		of the current rotational angle encoded in the rotation. Offset will contain up to
		4 valid positions, according to the channel type. This method will update every frame 
		value, and thus it assumes that postures are not shared among frames (the normal case). */
	void change_values ( int f1, int f2, int channel, float mfactor, const float* offset );

	/*! Calls KnPosture::mirror() for each posture in the motion. */
	void mirror ( const char* left, const char* right, bool printerrors=false );

	/*! Append the given motion to the current one. The appended postures are shared (not copied),
		and their channels will be sharing the one of the first frame. After calling this method,
		motion m will have all its postures shared and with shared channels. */
	void append ( KnMotion* m, float deltakt );

	/*! Load a motion from its file and then call append. Returns true if the file could
		be loaded, otherwise returns false */
	bool append ( const char* filename, float deltakt );

	/*! Access to the user data pointer. The pointer will only be allocated the first time it
		is accessed. The pointer can be shared, and KnMotion will call unref() in its destructor. */
	GsVars* userdata ();
};

//================================ End of File =================================================

# endif  // KN_MOTION_H
