
# ifndef KN_CT_MOTION_H
# define KN_CT_MOTION_H

//=================================== KnCtMotion =====================================

# include <sig/gs_table.h>
# include <sigkin/kn_motion.h>
# include <sigkin/kn_controller.h>

/*! This motion controller provides a controller interface to evaluate an attached
	KnMotion. Besides few extra functionality such as time warping, looping, etc; 
	it also efficiently supports shared KnMotions across many controllers by
	storing the last apply frame index for each controller instance. */
class KnCtMotion : public KnController
 { private :
	KnPosture _buffer; // internal buffer for storing the evaluations of the motion
	KnMotion* _motion;			   // the motion
	KnMotion::InterpType _play_mode; // its play mode
	double _duration;				// the time-warped duration
	float _maxtwarp;  // max time warping factor allowed to increase the motion speed
	float _mintwarp;  // min time warping factor allowed to reduce the motion speed
	float _twarp;	 // current used warping time
	bool  _loop;	  // if the motion is to be played in loop
	int   _last_apply_frame; // to optimize shared motion evaluation

   public :
	static const char* type_name;

   public :
	/*! Constructor */
	KnCtMotion ();

	/*! Destructor is public but pay attention to the use of ref()/unref() */
   ~KnCtMotion ();

	/*! Set the motion to be used. A valid motion must be set using
		this method before calling any other method, and init(0) can be called 
		to make this controller to completely unreference the internal motion.
		The old motion is unreferenced, and the new one is referenced.
		(KnMotion derives GsShareable and has ref/unref methods)
		Important: the keytimes of m are translated to ensure start from zero. */
	void init ( KnMotion* m );

	/*! Returns a pointer to the current motion of this controller */
	KnMotion* motion () { return _motion; }

	/*! Set the play mode, default is linear */
	void play_mode ( KnMotion::InterpType it ) { _play_mode=it; }

	/*! Returns the current play mode */
	KnMotion::InterpType play_mode () const { return _play_mode; }

	/*! Defines the maximum and minimum time scales acceptable for this motion. 
		These parameters are used to determine the feasibility of warping the motion
		in order to reach imposed timing constraints. */
	void warp_limits ( float wmin, float wmax );
	
	/*! Returns the maximum time warping factor */
	float maxtwarp () const { return _maxtwarp; }
	
	/*! Returns the minimum time warping factor */
	float mintwarp () const { return _mintwarp; }
	
	/*! Set a desired time warping ratio, that will be clamped to the min/max values */
	void twarp ( float tw );

	/*! Returns the current time warping ratio, default is 1 (not warping) */
	float twarp () const { return _twarp; }

	/*! Returns true if to be played in loop, and false otherwise */
	bool loop () const { return _loop; }

	/*! Change the loop state. If in loop, the duration of the controller will 
		be undetermined, ie -1 */
	void loop ( bool b ) { _loop=b; }

	/*! Returns the duration of the warped motion, which corresponds to the phase in
		a loop controller with undetermined duration */
	double phase_duration () { return _duration; }

	/*! Output data */
	void output ( GsOutput& out );

	/*! Reads the data. Returns false if the read motion name is not in the
		given hash table of motions */
	bool input ( GsInput& inp, const GsTable<KnMotion*>& motions );

   private : // callbacks for the base class

	virtual void controller_start ();
	virtual void controller_stop ();
	virtual bool controller_evaluate ( double t );
	virtual KnPosture& controller_buffer ();
	virtual double controller_duration ();
	virtual const char* controller_type ();
};

//======================================= EOF =====================================

# endif // KN_CT_MOTION_H

