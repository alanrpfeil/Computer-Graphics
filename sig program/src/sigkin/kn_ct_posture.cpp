
# include <sigkin/kn_ct_posture.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_motion.h>

//=================================== KnCtPosture =====================================

const char* KnCtPosture::type_name = "Posture";

KnCtPosture::KnCtPosture ()
 {
   _duration = 1.0f;
   _pt = 0;
 }

KnCtPosture::~KnCtPosture ()
 {
   if ( _pt ) _pt->unref();
 }

void KnCtPosture::init ( KnPosture* pt )
 {
   if ( _pt ) _pt->unref();
   _pt = pt;
   _pt->ref ();
 }

void KnCtPosture::init ( KnSkeleton* sk )
 {
   init ( new KnPosture(sk) ); //  Marcelo: new KnPosture );
   //_pt->connect ( sk );  // Marcelo: this seems to be not safe -> the method connect tries to access _channels.size() but _channels is not created yet
   _pt->get ();
 }

void KnCtPosture::init ( KnMotion* m, float t )
 {
   // create a posture with a copy of the motion channels:
   init ( new KnPosture );
   _pt->init ( new KnChannels(*m->channels()) );

   // connect the motion to the posture and apply it:
   m->connect ( _pt );
   m->apply ( t );
   
   // restore original channels:
   m->channels()->copyfrom ( *_pt->channels() );
 }

//----- virtuals -----
 
void KnCtPosture::controller_start ()
 {
 }

void KnCtPosture::controller_stop ()
 {
 }

bool KnCtPosture::controller_evaluate ( double t )
 {
   return t>=_duration? false:true; // returns the activation state
 }

KnPosture& KnCtPosture::controller_buffer ()
 {
   return *_pt;
 }

double KnCtPosture::controller_duration ()
 {
   return _duration;
 }

const char* KnCtPosture::controller_type ()
 {
   return type_name;
 }

//======================================= EOF =====================================

