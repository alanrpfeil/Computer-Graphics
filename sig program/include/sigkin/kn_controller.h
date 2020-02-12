
# ifndef KN_CONTROLLER_H
# define KN_CONTROLLER_H

# include <sig/gs_shareable.h>
# include <sig/gs_buffer.h>
# include <sigkin/kn_posture.h>

//=================================== KnController =====================================

/*! The Base class of generic kinematic controllers.  */
class KnController : public GsShareable
{  private :
	GsCharPt _name;		// name of this controller
	float _indt;		// initial period for blending, eg static or prep phase
	float _outdt;		// final period for blending, eg static or retract phase
	float _emphasist;	// time point of "main importance" in the controller
	bool _active;		// if the controller is still active
	KnSkeleton* _skcon; // skeleton connection of this controller, or null
	KnPosture* _ptcon;	// posture connection of this controller, or null
	struct ConCh
	 { KnChannels skc;
	   KnChannels ptc;
	 } *_conch;			// connected buffer channels for fast reconnections

   protected :
	/*! Constructor */
	KnController ();

   public :
	/*! Destructor is public but pay attention to the use of ref()/unref() */
	virtual ~KnController ();

	const char* name () const { return _name.pt? _name.pt:""; }
	void name ( const char* n ) { _name.set(n); }

	/*! returns the specified emphasis time, the default value is <0,
		meaning that the emphasis time point is unknown */
	float emphasist () const { return _emphasist; }
	
	/*! Specifies the desired emphasis time, a <0 value
		means that the emphasis time point is unknown */
	void emphasist ( float t );
	
	/*! Temporary connection will use internal buffers to speedup multiple reconnections.
		Nothing is done if the controller is already connected to sk.
		Note: disconnect() has to be called first when connecting to a skeleton 
		different than a previously connected one. */
	void temporarily_connect ( KnSkeleton* sk );
	 
	/*! Normally each controller is connected to its buffer, but if the 
		controller was temporarily connected to a skeleton this method will restore
		its original buffer connection. Multiple alternate calls to temporarily_connect(sk)
		and reconnect_to_buffer() are efficient as long as sk points to the
		same skeleton. */
	void reconnect_to_buffer ();

	/*! Connects to a given posture buffer and disconnects any previous connections.
		Nothing is done if the controller was already connected to the same posture. */
	void connect ( KnPosture* pt );

	/*! This method will ensure that a new connection is 
		performed (with buffers updated) the next time a connection to a skeleton 
		is made. The channels of the derived controller are not disconnected. */
	void disconnect ();

	/*! Returns the channels of the derived controlled */
	KnChannels* channels () { return controller_buffer().channels(); }

	/*! Returns the channels of the derived controlled */
	KnPosture& buffer () { return controller_buffer(); }

	/*! Returns the activation state */
	bool active () const { return _active; }

	/*! Defines the duration of the initial and final periods of the controller, 
		which can be used for blending and other adjustment purposes.
		Given times are corrected to ensure not exceeding the duration. */
	void inoutdt ( float indt, float outdt );
	
	/*! Get the indt time */
	float indt () const { return _indt; }

	/*! Get the outdt time */
	float outdt () const { return _outdt; }
	
	/*! This method is to be called before starting to evaluate the controller. */
	void start () { _active=true; controller_start (); }

	/*! This method will simply set the active flag of the controller to false and
		notify the derived class by calling the virtual method controller_stop(). */
	void stop () { _active=false; controller_stop (); }

	/*! Evaluates the controller at a local time t. The result of the evaluation
		will be sent to the connected skeleton or posture buffer. */ 
	void evaluate ( double t ) { _active = controller_evaluate ( t ); }

	/*! Simply makes a call to controller_duration(). */
	double duration () { return controller_duration (); }

   protected :
   
	/*! Output of the generic controller data.
		This is a protected method: derived classes will be responsible to save 
		their generic controller data. */
	void output ( GsOutput& o );

	/*! Input of the generic controller data.
		This is a protected method: derived classes will be responsible to load 
		their generic controller data. */
	void input ( GsInput& i );

   protected :
   
	/*! This method will be called to notify that the controller will become active
		and therefore a new evaluation starting at time 0 will soon begin. */
	virtual void controller_start ()=0;

	/*! This method will be called to notify that the controller was requested to stop.
		At the moment this method is called, the active flag is already set to false. */
	virtual void controller_stop ()=0;

	/*! This method is called at each time step (t starts from 0) and is where
		the derived controller will send the evaluation result to the controller
		posture buffer (retrieved with method buffer()). The buffer will have the exact 
		number of positions required by the used channels. The activation state
		must be returned: false if the controller has finished, and true otherwise. */
	virtual bool controller_evaluate ( double t )=0;

	/*! This method will return a KnPosture that must be used by the derived controller
		to maintain the description of the used channels and to store the evaluation
		values when the controller is connected to a buffer for blending operations. */
	virtual KnPosture& controller_buffer ()=0;

	/*! This method will return the fixed total duration time that the controller
		will take, or will return -1 if the duration is undetermined */
	virtual double controller_duration ()=0;
	
	/*! Returns a string describing the type of the controller. The convention is that
		the string corresponds with the derived class name without the 'KnCt' prefix */
	virtual const char* controller_type ()=0;
};

//======================================= EOF =====================================

# endif // KN_CONTROLLER_H
