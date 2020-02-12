
# include <sigkin/kn_controller.h>
# include <sig/gs_string.h>

//============================= KnController ============================

KnController::KnController ()
 {
   _active = false;
   _indt = _outdt = 0.0f;
   _name = 0;
   _emphasist = -1.0f;
   _skcon = 0;
   _ptcon = 0;
   _conch = 0;
 }

KnController::~KnController ()
 {
   delete _conch;
 }

void KnController::emphasist ( float t )
 { 
   if ( t<0 )
	{ t=-1.0f; }
   else
	{ double d = controller_duration();
	  if ( d>=0 && double(t)>d ) t=float(d);
	}
   _emphasist=t;
 }

void KnController::inoutdt ( float indt, float outdt )
 {
   float d = (float)controller_duration();

   if ( d>=0 && indt+outdt>d )
	{ float factor = d/(indt+outdt);
	  indt *= factor;
	  outdt *= factor;
	}

   _indt = indt;
   _outdt = outdt;
 }

void KnController::temporarily_connect ( KnSkeleton* sk )
 {
   if ( _skcon ) return; // already connected
   if ( !_conch ) _conch = new ConCh; // connecting for the first time to a skeleton

   KnChannels& skc = _conch->skc;
   if ( skc.size()==0 ) // buffer not initialized
	{ _conch->ptc.copyfrom ( *channels() ); // store pt connection buffer
	  channels()->connect ( sk ); // connect
	  skc.copyfrom ( *channels() ); // store sk connection buffer
	}
   else // use connected sk buffer
	{ channels()->copyfrom ( skc ); // restore previously connected channels
	}

   _skcon = sk;
 }

void KnController::reconnect_to_buffer ()
 {
   if ( !_skcon ) return; // already connected to buffer
   channels()->copyfrom ( _conch->ptc ); // restore previously connected channels
   _skcon = 0;
 }

void KnController::connect ( KnPosture* pt )
 {
   if ( pt==_ptcon ) return; // already connected
   channels()->connect ( pt ); // connect
   _ptcon = pt;
 }

void KnController::disconnect ()
 {
   if ( _conch ) { _conch->skc.init(); _conch->ptc.init(); }
   _skcon = 0;
   _ptcon = 0;
 }

void KnController::output ( GsOutput& o )
 {
   o << GsSafeWrite(name()) << " inout " << _indt << gspc << _outdt << gsnl;
   if ( _emphasist>=0 )
	 o << "emphasist " << _emphasist << gsnl;
 }

void KnController::input ( GsInput& i )
 {
   i.get();
   name ( i.ltoken() );
   i.get(); // inout
   i >> _indt;
   i >> _outdt;
   i.get();
   if ( i.ltoken()=="emphasist" )
	{ i >> _emphasist; }
   else
	{ _emphasist=-1.0; i.unget(); }
 }

//============================ End of File ============================
