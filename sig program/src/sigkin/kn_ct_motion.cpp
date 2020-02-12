
# include <sig/gs_string.h>
# include <sigkin/kn_ct_motion.h>

//=================================== KnCtMotion =====================================

const char* KnCtMotion::type_name = "Motion";

KnCtMotion::KnCtMotion ()
 {
   _motion = 0;
   _play_mode = KnMotion::Linear;
   _duration = 0;
   _twarp = _maxtwarp = _mintwarp = 1.0f;
   _loop = false;
   _last_apply_frame = 0;
 }

KnCtMotion::~KnCtMotion ()
 {
   init(0);
 }

void KnCtMotion::init ( KnMotion* m )
 {
   if ( _motion ) _motion->unref();
   if ( !m ) { _motion=0; _duration=0; _buffer.init(); return; }

   _motion = m;
   _motion->ref();
   _motion->move_keytimes ( 0 ); // make sure motion starts at 0
   _duration = _motion->duration() / _twarp;
   _last_apply_frame = 0;

   disconnect (); // clear flags with respect to previous connections
   _buffer.init ( new KnChannels(*m->channels()) );
   _motion->connect ( &_buffer );
 }

void KnCtMotion::warp_limits ( float wmin, float wmax )
 {
   if ( wmin<0.0001f ) wmin=0.0001f;
   if ( wmax>9999.0f ) wmax=9999.0f;
   if ( wmin>wmax ) return;
   _mintwarp = wmin;
   _maxtwarp = wmax;
   twarp ( _twarp );
 }

void KnCtMotion::twarp ( float tw )
 {
   if ( !_motion ) return;

   // put in/out back to their original values:
   float in = indt()*_twarp;
   float out = outdt()*_twarp;
   float emph = emphasist()*_twarp;

   // check limits:
   if ( tw<_mintwarp ) tw = _mintwarp;
   if ( tw>_maxtwarp ) tw = _maxtwarp;
   _twarp = tw;

   // update duration:
   _duration = _motion->duration() / _twarp;

   // update in/out:
   inoutdt ( in/_twarp, out/_twarp );

   // update emphasist:
   if ( emph>=0 ) emphasist ( emph/_twarp );
	else emphasist ( -1.0f ); // set -1 again to ensure that all <0 values are -1
 }

void KnCtMotion::output ( GsOutput& out )
 {
   KnController::output ( out );

   if ( !_motion ) return;

   // name
   if ( gs_compare(name(),_motion->name())!=0 )
	{ out << "motion " << GsSafeWrite(_motion->name()) << gsnl;
	}

   // play mode
   if ( _play_mode!=KnMotion::Linear )
	{ out << "mode " << KnMotion::interp_type_name(_play_mode) << gsnl;
	}

   // time warp
   if ( _twarp!=1.0f || _mintwarp!=1.0f || _maxtwarp!=1.0f )
	{ out << "twarp " << _twarp;
	  out << " minmax " << _mintwarp << gspc << _maxtwarp << gsnl;
	}

   // loop
   if ( _loop==true )
	{ out << "loop true\n";
	}

   // end
   out << "end\n";
 }

bool KnCtMotion::input ( GsInput& inp, const GsTable<KnMotion*>& motions )
 {
   KnController::input ( inp );

   // init with defaults:
   GsString mname ( name() );
   _play_mode = KnMotion::Linear;
   _twarp = _mintwarp = _maxtwarp = 1.0f;
   _loop = false;

   // read:
   while ( !inp.end() )
	{ inp.get();
	  const GsString& s = inp.ltoken();
	  if ( s=="motion" )
	   { inp.get();
		 mname = inp.ltoken();
	   }
	  else if ( s=="mode" )
	   { inp.get();
		 _play_mode = KnMotion::interp_type_name(inp.ltoken());
	   }
	  else if ( s=="twarp" )
	   { inp >> _twarp;
		 inp.get();
		 inp >> _mintwarp >> _maxtwarp;
	   }
	  else if ( s=="loop" )
	   { inp.get();
		 _loop = inp.ltoken()=="true";
	   }
	  else if ( s=="end" )
	   { break;
	   }
	}
   
   KnMotion* m = motions.lookup ( mname );
   if ( !m ) return false;
   init ( m );
   return true;
 }

//----- virtuals -----
 
void KnCtMotion::controller_start ()
 {
 }

void KnCtMotion::controller_stop ()
 {
 }

bool KnCtMotion::controller_evaluate ( double t )
 {
   if ( !_motion ) return false;
   if ( _loop )
	{ double x = t/_duration;
	  if ( x>1.0 ) t = _duration *( x-int(x) );
	  _motion->apply ( float(t)*_twarp, _play_mode, &_last_apply_frame );
	  return true;
	}
   else
	{ _motion->apply ( float(t)*_twarp, _play_mode, &_last_apply_frame );
	  return t>=_duration? false:true; // returns the activation state
	}
 }

KnPosture& KnCtMotion::controller_buffer ()
 {
   return _buffer;
 }

double KnCtMotion::controller_duration ()
 {
   return _loop? -1.0:_duration;
 }

const char* KnCtMotion::controller_type ()
 {
   return type_name;
 }
 
//======================================= EOF =====================================

