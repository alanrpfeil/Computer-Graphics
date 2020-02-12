
# include <sigkin/kn_ct_scheduler.h>
# include <sigkin/kn_skeleton.h>

//# define GS_USE_TRACE1
# include <sig/gs_trace.h>

//=================================== KnCtScheduler =====================================

const char* KnCtScheduler::type_name = "Scheduler";

KnCtScheduler::KnCtScheduler ()
 {
   _lastt = 0;
   _sk = 0;
 }

KnCtScheduler::~KnCtScheduler ()
 {
   clear ();
 }

void KnCtScheduler::init ( const KnChannels& ch )
 {
   clear ();
   _sk = 0;
   _lastt = 0;
   _buffer.init ( new KnChannels(ch) );
 }

void KnCtScheduler::init ( KnSkeleton* sk )
 {
   init ( sk->channels() ); // set channels
   _sk = sk;
   _buffer.init ( sk );
   _buffer.get ();
 }

void KnCtScheduler::schedule ( KnController* c, double tin, float indt, float outdt, Mode mode )
 {
   c->connect ( &_buffer ); // will do nothing if already connected to this scheduler

   c->ref ();

   double dur = c->duration();

   Track& tr = _tracks.push();
   tr.controller = c;
   tr.dominant = false;

   tr.tin = tin;
   tr.tstart = tin;
   tr.waitdt = 0;

   tr.tout = dur<0? -1.0 : tin + dur;
   tr.tend = tr.tout;
   tr.extdt = 0;

   tr.outdt = outdt;
   tr.indt = indt;
   tr.mode = mode;
 }

void KnCtScheduler::clear ( int n )
 {
   stop();
   while ( _tracks.size()>n )
	{ _tracks.top().controller->unref();
	  _tracks.pop();
	}
   if ( _domtr >= n ) _domtr = -1;
 }

bool KnCtScheduler::remove ( int n )
{
	if ( _tracks.size()>n )
		return false;

	_tracks[n].controller->unref();
	_tracks.remove(n);
	
	if ( _domtr >= n ) 
	 { if ( _domtr == n ) _domtr = -1;
	   else _domtr -= 1; // will keep the dominant adjusting the index
	 } 

	return true;
}

int KnCtScheduler::track ( KnController* c )
 {
   int i;
   for ( i=0; i<_tracks.size(); i++ )
	if ( _tracks[i].controller==c ) return i;
   return -1;
 }

void KnCtScheduler::change_mode ( int track, Mode mode )
 {
   _tracks[track].mode = mode;
 }

void KnCtScheduler::change_mode ( Mode oldmode, Mode newmode )
 {
   int i;
   for ( i=0; i<_tracks.size(); i++ )
	{ if ( _tracks[i].mode==oldmode )
	   _tracks[i].mode = newmode;
	}
 }
 
//----- virtuals -----

void KnCtScheduler::controller_start ()
 {
   int k, trsize = _tracks.size();
   for ( k=0; k<trsize; k++ ) _tracks[k].controller->start();
 }

void KnCtScheduler::controller_stop ()
 {
   int k, trsize = _tracks.size();
   for ( k=0; k<trsize; k++ ) _tracks[k].controller->stop();
 }

bool KnCtScheduler::controller_evaluate ( double t )
 {
   int trsize = _tracks.size();
   if ( trsize==0 ) return false; // no tracks, not active anymore
   KnController* ct;
   Track* tr;
   double tloc, tstart, tout;
   int k; // current active track

   // if there is a dominant motion being evaluated, just process that one:
   // possibly optimize: use fast reconnection mechanism in KnController to send values
   // of the dominant motion directly to the skeleton, otherwise two buffer transfers happen.
   if ( _domtr>=0 )
	{ tr = &_tracks[_domtr];
	  ct = tr->controller;
	  if ( t>tr->tend-tr->outdt || t<_lastt )
	   { _domtr=-1; }
	  else
	   { tloc = t-tr->tstart;
		 GS_TRACE1 ( _domtr<<": applying dominant controller at t="<<tloc );
		 ct->evaluate ( tloc ); // evaluate to the controller buffer with local time
		 interp ( ct->buffer(), 0 ); // result is placed in the connected posture buffer
		 _lastt = t;
		 return ct->active(); // returns the activation state of the dominant controller
	   }
	}

   bool stateret = false;
   for ( k=0; k<trsize; k++ )
	{ tr = &_tracks[k];
	  ct = tr->controller;
	  tstart = tr->tstart;
	  tout = tr->tout;

	  // 1. Check if controller is to be played:
	  if ( t<tstart ) continue; // not yet started
	  if ( tout>0 && t>tr->tend ) // know duration and finished
	   { if ( ct->active() ) ct->stop();
		 if ( tr->mode==Once ) { ct->unref(); _tracks.remove(k); trsize--; k--; } // remove track
		 continue;
	   }

	  // 2. Evaluate controller if needed:
	  if ( tout<0 || t<tout ) // tout not known or valid and before extension period
	   { if ( !ct->active() ) ct->start();
		 tloc = t-tstart;
		 ct->evaluate ( tloc ); // evaluate to the controller buffer with local time
	   }

	  // 3. Blend or copy values to the scheduler's buffer:
	  if ( tloc<tr->indt ) // check if in ease-in phase
	   { tloc = 1.0-(tloc/tr->indt);
		 GS_TRACE1 ( k<<": blending in with t="<<tloc );
		 interp ( ct->buffer(), GS_CUBIC((float)tloc) ); // result is placed in the connected posture buffer
	   }
	  else if ( tout>0 && t>tr->tend-tr->outdt ) // check if in ease-out phase
	   { tloc = 1.0-( (tr->tend-t)/tr->outdt );
		 GS_TRACE1 ( k<<": blending out with t="<<tloc );
		 interp ( ct->buffer(), GS_CUBIC((float)tloc) ); // result is placed in the connected posture buffer
	   }
	  else // just copy values
	   { GS_TRACE1 ( k<<": copying values at t="<<tloc );
		 interp ( ct->buffer(), 0 ); // result is placed in the connected posture buffer
		 if ( tr->dominant ) _domtr=k;
	   }

	  if ( ct->active() ) // if at least one controller is active the scheduler is active
		  stateret = true;
	}

   _lastt = t;
   return stateret; // returns the activation state
 }

KnPosture& KnCtScheduler::controller_buffer ()
 {
   return _buffer;
 }

double KnCtScheduler::controller_duration ()
 {
   int i;
   double tout, dur=0;
   for ( i=0; i<_tracks.size(); i++ )
	{ tout = _tracks[i].tout;
	  if ( tout<0 ) return -1;
	  if ( tout>dur ) dur=tout;
	}
   return dur;
 }
 
const char* KnCtScheduler::controller_type ()
 {
   return type_name;
 }

//======================================= EOF =====================================
