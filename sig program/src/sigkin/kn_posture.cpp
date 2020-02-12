/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
 
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_posture.h>

//# define GS_USE_TRACE1  // trace
//# include <sig/gs_trace.h>

//=========================== KnPostureDfJoints ===============================

void KnPostureDfJoints::input ( GsInput& in, const KnSkeleton* sk )
{
	int i=0, n=99999;
   
	if ( in.check()==GsInput::Number )
	{	in >> n; joints.reserve(n); }
	
	joints.size(0);
	for ( i=0; i<n; i++ )
	{	in.get();
		if ( in.ltype()!=GsInput::String ) break; // ';' found
		joints.push() = sk->joint(in.ltoken());
		if ( !joints.top() ) joints.pop();
	}
	joints.compress();
}

void KnPostureDfJoints::output ( GsOutput& out ) const
{
	out << joints.size() << gspc;
   
	for ( int i=0, s=joints.size(); true; i++ )
	{	out << GsSafeWrite(joints[i]->name());
		if ( i+1<s ) out << gspc; else break;
	}
	out << ";\n";
}

int KnPostureDfJoints::lsearch ( const char* jname )
{
	for ( int i=0, s=joints.size(); i<s; i++ )
	{
		if ( joints[i]->name()==jname ) return i;
	}
	return -1;
}

//============================= KnPosture ============================

KnPosture::KnPosture() : GsShareable()
{
	_channels = 0;
	_dfjoints = 0;
	_syncpoints = false;
	_name = 0;
}

KnPosture::KnPosture ( const KnPosture& p, KnChannels* chstoshare )
		  : GsShareable(), values ( p.values ), points ( p.points )
{
	_channels = 0;
	_dfjoints = 0;

	if ( chstoshare )
	{	_channels=chstoshare; _channels->ref(); }
	else
	{	if ( p._channels ) { _channels=p._channels; _channels->ref(); } }

	if ( p._dfjoints ) { _dfjoints=p._dfjoints; _dfjoints->ref(); }
	  
	_syncpoints = p._syncpoints;
	_name = 0;
	name ( p.name() );
}

KnPosture::KnPosture ( KnChannels* ca, KnPostureDfJoints* dfj )
{
	_dfjoints = 0;
	_syncpoints = false;
	_name = 0;

	_channels = ca;
	_channels->ref();
	values.size ( _channels->floats() );
   
	dfjoints ( dfj );
}

KnPosture::KnPosture ( KnSkeleton* s )
{
	_dfjoints = 0;
	_syncpoints = false;
	_name = 0;

	_channels = &s->channels();
	_channels->ref();
	values.size ( _channels->floats() );

	dfjoints ( s->dfjoints() );
}

KnPosture::~KnPosture ()
{
	init ();
	delete[] _name;
}

void KnPosture::init ()
{
	if ( _channels ) { _channels->unref(); _channels=0; }
	if ( _dfjoints ) { _dfjoints->unref(); _dfjoints=0; }
	_syncpoints = false;
	values.size(0);
	points.size(0);
}

void KnPosture::init ( KnChannels* ca )
{
	init ();
	_channels = ca;
	_channels->ref();
	values.size ( _channels->floats() );
}

void KnPosture::init ( KnChannels* ca, KnPostureDfJoints* ja )
{
	init ( ca );
	dfjoints ( ja );
}

void KnPosture::init ( KnSkeleton* s )
{
	init ( &s->channels() );
	dfjoints ( s->dfjoints() );
}

void KnPosture::channels ( KnChannels* c )
{
	if ( _channels==c ) return;
	if ( _channels ) _channels->unref();
	_channels = c;
	if ( _channels ) _channels->ref();
}

int KnPosture::make_quat_channels ()
{
	if ( !_channels ) return -1;
	KnChannels* c = new KnChannels ( *_channels );
	int n = c->force_quat_channels ();
	values.size ( c->size()*4 );
	channels ( c ); // replace with new one
	return n;
}

void KnPosture::name ( const char* n )
{
	gs_string_set ( _name, n );
}

void KnPosture::get ()
{
	_syncpoints = false;
	_channels->get ( &values[0] );
}

void KnPosture::get_random ()
{
	_syncpoints = false;
	_channels->get_random ( &values[0] );
}

bool KnPosture::insert ( int pos, KnChannel::Type type, float* fvalues )
{
	if ( !_channels ) return false;

	// Insert channel:
	if ( !_channels->insert(pos,type) ) return false;
   
	// Insert space for values:
	int chsize = KnChannel::size(type);
	int fpos = _channels->floatpos(pos);
	values.insert ( fpos, chsize );

   // Set new values to zero or to fvalues:
	for ( int i=0; i<chsize; i++ )
	{	values[fpos+i] = fvalues? fvalues[i]:0.0f;
	}

	return true;
}

void KnPosture::dfjoints ( KnPostureDfJoints* dfjoints )
{
	if ( _dfjoints==dfjoints ) return;

	if ( _dfjoints ) _dfjoints->unref();
	_dfjoints = dfjoints;

	if ( _dfjoints )
	{	_dfjoints->ref();
		points.size ( _dfjoints->joints.size() );
	}
	_syncpoints = false;
}

void KnPosture::make_points ( KnJoint* j )
{
	if ( !_channels ) return;
	if ( !_channels->skeleton() ) return;

	// apply this configuration to skeleton :
	apply ();

	// update matrices:
	if ( !j )
		skeleton()->update_global_matrices();
	else
		j->update_gmat();

	// get points:
	int i;
	if ( _dfjoints )
	{	int size = _dfjoints->joints.size();
		if ( points.size()!=size ) points.size ( size );
		for ( i=0; i<size; i++ ) points[i] = _dfjoints->joints[i]->gcenter();
	}
	else
	{	int size = _channels->size();
		if ( points.size()!=size ) points.size ( size );
		for ( i=0; i<size; i++ ) points[i] = _channels->get(i).joint()->gcenter();
	}

	_syncpoints = true;
}

void KnPosture::mirror ( const char* left, const char* right, bool printerrors )
 {
   int i;
   GsQuat q;

   KnSkeleton *sk = skeleton();
   if ( !sk ) return;

   // first mirror the rotations in all the joints:
   for ( i=0; i<sk->joints().size(); i++ )
	{
	  q = sk->joints()[i]->quat()->value();
	  sk->joints()[i]->quat()->value( GsQuat(-q.w,-q.x,q.y,q.z) );
	}

   // now swap left values with right values
   GsString jname;
   for ( i=0; i<sk->joints().size(); i++ )
	{ KnJoint *ljoint = sk->joints()[i];
	  jname = ljoint->name();
	  if ( jname.replace(left,right)!=-1 )
	   { KnJoint *rjoint = sk->joint(jname);
   		 if ( rjoint==0 )
		  {	if ( printerrors ) gsout << sk->filename() << " does not have joint " << jname << gsnl;
		  }
		 else
		  {	q = ljoint->quat()->value();
			ljoint->quat()->value ( rjoint->quat()->value() );
			rjoint->quat()->value (q);
		  }	
	   }
	}

   GsVec pos = sk->root()->pos()->value();
   sk->root()->pos()->value ( -pos.x, pos.y, pos.z ); // this depending on the original heading
   get (); // retrieve the values from the skeleton to the posture
 }

void KnPosture::operator= ( const KnPosture& p )
{
	if ( &p==this ) return;

	_syncpoints = p._syncpoints;

	if ( _channels!=p._channels )
	{	if ( _channels ) _channels->unref();
		_channels = p._channels;
		if ( _channels ) _channels->ref();
	}

	if ( _dfjoints!=p._dfjoints )
	{	if ( _dfjoints ) _dfjoints->unref();
		_dfjoints = p._dfjoints;
		if ( _dfjoints ) _dfjoints->ref();
	}

	values = p.values;
	points = p.points;
}

//============================= friends ============================

void interp ( const KnPosture& p1, const KnPosture& p2, float t, KnPosture& p )
{
	//gsout<<"SIZES: "<< p1.values.size()<<gspc<<p2.values.size()<<gspc<<p.values.size()<<gsnl;

	KnChannels& ch = *p._channels;
	const float* fp1 = &p1.values[0];
	const float* fp2 = &p2.values[0];
	float* fp = &p.values[0];
	int chsize=ch.size();
	int i, dp;

	for ( i=0; i<chsize; i++ )
	{	dp = ch[i].interp ( fp1, fp2, t, fp );
		fp1 += dp;
		fp2 += dp;
		fp += dp;
	}

	p._syncpoints = false;
}

void interp ( const KnPosture& p, float t )
{
	KnChannels& ch = *p._channels;
	const float* fp1 = p.values.pt();
	int i, chsize=ch.size();
	for ( i=0; i<chsize; i++ )
	{	float* fp2 = ch[i].buffer(); // get buffer pointer of the channel connection
		if ( !fp2 ) { fp1+=ch[i].size(); continue; } // not connected
		fp1 += ch[i].interp ( fp1, fp2, t, fp2 );
	}
}

void interp ( const GsArray<KnPosture*>& postures, const GsArray<float>& w, 
			  KnPosture& p, GsArray<float*>& buffer )
 {
   int i, j, dp, psize=postures.size();

   buffer.size ( psize );
   KnChannels& ch = *p._channels;
   for ( i=0; i<psize; i++ ) buffer[i]= &(postures[i]->values[0]);

   float* fp = &p.values[0];
   int chsize=ch.size();

   for ( i=0; i<chsize; i++ )
	{ //gsout<<ch[i].jname()<<gsnl;
	  dp = ch[i].interp ( buffer, w, fp );
	  for ( j=0; j<psize; j++ ) buffer[j]+=dp;
	  fp += dp;
	}

   p._syncpoints = false;
 }

float dist ( KnPosture& p1, KnPosture& p2 )
{
	int i, size;

	if ( !p1._syncpoints ) p1.make_points();
	if ( !p2._syncpoints ) p2.make_points();

	size = p1.points.size();
	if ( size>p2.points.size() ) size=p2.points.size();

	//gsout<<"SIZES: "<< p1.values.size()<<gspc<<p2.values.size()<<gsnl;
   
	// Max norm :
	float d2, d=0;
	for ( i=0; i<size; i++ )
	{	d2 = dist2 ( (const GsVec&)(p1.points[i]), (const GsVec&)(p2.points[i]) );
		if ( d2>d ) d=d2;
	}
	return sqrtf(d);
}

void KnPosture::output ( GsOutput& out, bool channels, bool values, bool onelineval ) const
{
	int i;

	if ( channels && _channels )
	{	out << (*_channels) << gsnl;
	}
	
	if ( _name && _name[0] )
	{	out << "name " << GsSafeWrite(_name) << gspc;
	}

	char delim = gspc;
	if ( !onelineval ) delim=gsnl;

	if ( values && _channels )
	{	const float* fp = &KnPosture::values[0];
		int chsize = _channels->size();
		for ( i=0; i<chsize; i++ )
		{	fp += _channels->cget(i).save(out,fp);
			if ( i<chsize-1 ) out<<delim;
		}
	}

	out << gsnl;
}

GsOutput& operator<< ( GsOutput& out, const KnPosture& p )
{
	p.output ( out, true, true );
	return out;
}

GsInput& operator>> ( GsInput& inp, KnPosture& p )
 {
   p.name ( 0 );

   while ( inp.check()==GsInput::String )
	{ inp.get();
	  if ( inp.ltoken()=="channels" )
	   { if ( p._channels ) p._channels->unref();
		 p._channels = new KnChannels;
		 p._channels->ref();
		 inp.unget();
		 inp >> *p._channels;
	   }
	  else if ( inp.ltoken()=="name" )
	   { p.name ( inp.gets() );
	   }
	  else 
	   { p.name ( inp.ltoken() );
	   }
	}

   if ( !p._channels ) return inp;

   KnChannels& ch = *p._channels;
   p.values.size ( ch.floats() );
   float* fp = &(p.values[0]);
   int i, chsize = ch.size();

   for ( i=0; i<chsize; i++ )
	{ fp += ch[i].load(inp,fp);
	}

   p._syncpoints = false; // because we do not save/read the points

   return inp;
 }

//================================ End of File =================================================

