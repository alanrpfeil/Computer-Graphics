# ifndef KN_CT_POSTURE_H
# define KN_CT_POSTURE_H

//=================================== KnCtPosture =====================================

class KnMotion;

# include <sigkin/kn_controller.h>
# include <sigkin/kn_posture.h>

/*! This controller always evaluates a same posture. */
class KnCtPosture : public KnController
 { private :
	KnPosture* _pt;  // the posture
	float _duration; // specified duration, or -1

   public :
	static const char* type_name;

   public :
	/*! Constructor */
	KnCtPosture ();

	/*! Destructor is public but pay attention to the use of ref()/unref() */
   ~KnCtPosture ();

	/*! Initialize from a given posture, which will be simply referenced. */
	void init ( KnPosture* pt );

	/*! Initialize from a copy of the given posture */
	void init ( const KnPosture& pt ) { init ( new KnPosture(pt) ); }

	/*! Initialize with a new posture captured from the current pose of the skeleton.
		After this call the controller will be connected to the skeleton. */
	void init ( KnSkeleton* sk );

	/*! Creates a new posture and initializes it from motion m evaluated at time t.
		This method will leave the motion connection state unchanged. */
	void init ( KnMotion* m, float t );

	/*! Defines the desired duration of this rest controller. Default is 1 second.
		For undetermined duration, -1 can be specified. */
	void duration ( float d ) { _duration = d; }
	
	/*! Retrieves the duration, which can be -1. */
	float duration () { return _duration; }

   private : // callbacks for the base class
	virtual void controller_start ();
	virtual void controller_stop ();
	virtual bool controller_evaluate ( double t );
	virtual KnPosture& controller_buffer ();
	virtual double controller_duration ();
	virtual const char* controller_type ();
};

//======================================= EOF =====================================

# endif // KN_CT_POSTURE_H
