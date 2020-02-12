
# ifndef KN_CT_SCHEDULER_H
# define KN_CT_SCHEDULER_H

//=================================== KnCtScheduler =====================================

# include <sigkin/kn_controller.h>

/*! The scheduler maps each attached controller to the scheduler's posture buffer, so that
	evaluated values can be copied and blended as evaluated by the scheduler. */
class KnCtScheduler : public KnController
{  public :
	 enum Mode { Once,   // controller automatically removed when finished
				 Static  // controller not removed when finished (not tested)
			   };

	 struct Track
	  { KnController* controller; // controller in this track
		int	   dominant;	// if >0 this controller will connect directly to the output when not being blended
		double	tin;		// when to start in global time
		double	tout;		// when the controller will end, or -1 if undetermined
		double	waitdt;		// duration after tin and before indt to use 0% activation (see waitdt() method)
		double	extdt;		// extension period after controller's end
		double	tstart;		// tin+twaitdt
		double	tend;		// tout+extdt
		double	indt;		// duration to apply ease-in transition
		double	outdt;		// duration to apply ease-out transition
		Mode	mode;		// track mode
	 };

   private :
	GsArray<Track> _tracks;	// active current motions being played
	KnPosture _buffer;		// used to blend tracks, which are mapped to the buffer
	KnSkeleton* _sk;		// connected skeleton, if not null
	double _lastt;			// last evaluated t
	int _domtr;				// index of the dominant track, if>=0

   public :
	static const char* type_name;

   public :
	/*! Constructor */
	KnCtScheduler ();

	/*! Destructor is public but pay attention to the use of ref()/unref() */
   ~KnCtScheduler ();

	/*! Returns the current number of tracks */
	int tracks () const { return _tracks.size(); }

	/*! Returns a reference to the track index i (0 is at the bottom). Use with care. */
	Track& track ( int i ) { return _tracks[i]; }

	/*! Returns a reference to the top track. Use with care. */
	Track& toptrack () { return _tracks.top(); }

	/*! Initialize from a copy of the given channels */
	void init ( const KnChannels& ch );

	/*! Initializes the channels from the ones in the given skeleton
		and initializes the internal buffer values with the current posture
		of the given skeleton. */
	void init ( KnSkeleton* sk );

	/*! Schedule controller c with the given parameters.
		c - the controller to be scheduled.
		tin - the time to activate (start) the controller.
		indt - ease-in duration for blending with the prior controller in the stack.
		outdt - ease-out duration for blending with the next controller in the stack.
		mode - Once will automatically remove c after completion, Static will not remove.
		Parameters waitdt and extension are automatically set to 0. The end time (tout)
		of c is automatically retrieved from c (can be undetermined -1).
		Controller c is automatically mapped to the scheduler if needed.
		See also methods: extension(), waitdt() and tout(). */
	void schedule ( KnController* c, double tin, float indt, float outdt, Mode mode );

	/*! Set a period for extending the controller in the given track. During the extension 
		period, the final posture of the controller remains active after its completion.
		Therefore the extension duration is applied after the time out of the controller.
		The outdt period will start at time (tout+extdt)-outdt. */
	void extension ( Track& tr, float ext ) { tr.extdt=ext; tr.tend=tr.tout+ext; }

	/*! Set the extension parameter for the top track */
	void extension ( float ext ) { extension ( toptrack(), ext ); }

	/*! Sets a "wait time" for the controller in the given track. This is a period of time
		where the controller will have 0% of activation after tin, and before the indt period.
		For example: if a controller has tin=3, waitdt=1, and  indt=2, the controller will first
		be evaluated at time 3+1=4, corresponding to its local time 1, and only after that the
		ease-in blending will start, have indt duration, to then reach 100% activation. */
	void waitdt ( Track& tr, float wdt ) { tr.waitdt=wdt; tr.tstart=tr.tin+wdt; }

	/*! Set the waitdt parameter for the top track */
	void waitdt ( float wdt ) { waitdt ( toptrack(), wdt ); }

	/*! If set to true only the given track will generate motion outside the indt/outdt periods;
		this allows a direct output connection, without evaluating other existing tracks. */
	void dominant ( Track& tr, bool b ) { if ( tr.tout>0 ) tr.dominant=(int)b; }

	/*! Set the dominant parameter for the top track */
	void dominant ( bool b ) { dominant ( toptrack(), b ); }

	/*! Sets the desired tout for the controller in the top track. -1 can be given for
		specifying a controller with undetermined duration, i.e. with "self-termination" */
	void tout ( Track& tr, double to ) { tr.tout=to; }

	/*! Set the tout parameter for the top track */
	void tout ( double to ) { tout ( toptrack(), to ); }

	/*! Removes tracks from the scheduler, starting from the top, until only n tracks remains.
		Calling clear with n equal 0 (the default value) will empty the scheduler. */
	void clear ( int n=0 );

	/*! Removes selected track from the scheduler if present.
		False if element not preset. */
	bool remove ( int n );
	
	/*! Returns the track number of the controller c, or -1 if c is not
		currently scheduled to play */
	int track ( KnController* c );
	
	/*! Changes the mode of the controller in the given track index */
	void change_mode ( int track, Mode mode );
	
	/*! Change the mode of all controllers in the scheduler of type oldmode to newmode */
	void change_mode ( Mode oldtype, Mode newtype );
	
   private : // callbacks for the base class
   
	virtual void controller_start ();
	virtual void controller_stop ();
	virtual bool controller_evaluate ( double t );
	virtual KnPosture& controller_buffer ();
	virtual double controller_duration ();
	virtual const char* controller_type ();
 };

//======================================= EOF =====================================

# endif // KN_CT_SCHEDULER_H
