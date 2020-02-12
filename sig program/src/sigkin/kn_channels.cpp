/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigkin/kn_channels.h>
# include <sigkin/kn_skeleton.h>

//========================= KnChannels::HashTable ============================

/* We use an internal specific implementation of a hash table for fast channel search.
   The hash table is created and used whenever methods KnChannels::search() and 
   KnChannels::map() are called (methods implementation in the end of this file)*/
class KnChannels::HashTable
{  public:
	struct Entry
	{	int	  jname_key;   // KnJointName::id()
		char	 ctype_key;   // the KnChannel::Type enumerator [-127,128] (see gs.h)
		gsuint16 pos_data;	// the position of the key in the channel array max is 65,535
		gsint16  next;		// the index of the next colliding item in [-32768,32767]
		void set ( int jk, char ck, gsuint16 pos, gsint16 n )
		{ jname_key=jk; ctype_key=ck; pos_data=pos; next=n; }
	 };
	GsArray<Entry> table;
	int hash_size;
   public:
	HashTable () { hash_size=0; }
	void init ( int hsize );
	int size () const { return table.size(); }
	int collisions () const { return table.size()-hash_size; }
	int longest_entry () const;					   // for inspection purposes only
	int lookup ( int jname, char ctype ) const;	   // returns position or -1 if not there
	int insert ( int jname, char ctype, gsuint16 pos ); // return -1 if already inserted
	void out ( GsOutput& o );
};

//============================= KnChannels ============================

KnChannels::KnChannels ()
 {
   _floats=0;
   _htable=0;
 }

KnChannels::KnChannels ( const KnChannels& c )
 {
   _channels = c._channels;
   _floats = c._floats;
   _htable = 0;
 }

KnChannels::~KnChannels ()
 {
   delete _htable;
 }

void KnChannels::init()
 {
   _channels.size ( 0 );
   _floats = 0;
   if ( _htable ) { delete _htable; _htable=0; }
 }

void KnChannels::_add ( KnJoint* j, KnChannel::Type t )
{
	_channels.push();

	if ( j==0 )
	{	_channels.top().init ( t ); 
	}
	else
	{	_channels.top().type ( t );
		_channels.top().connect ( j );
	}

	_floats += KnChannel::size(t);
	if ( _htable ) { delete _htable; _htable=0; }
}

bool KnChannels::insert ( int pos, KnChannel::Type type )
{
	if ( pos<0 || pos>_channels.size() ) return false;

	// add channel:
	_channels.insert ( pos );
	_channels[pos].init ( type );
	_floats += KnChannel::size ( type );
	if ( _htable ) { delete _htable; _htable=0; }

	return true;
}

KnSkeleton* KnChannels::skeleton () const
 {
   if ( _channels.size()==0 ) return 0;
   if ( _channels[0].status()!=KnChannel::JointConnection ) return 0;
   return _channels[0].joint()->skeleton();
 }

int KnChannels::count_floats ()
 {
   int i, csize = _channels.size();
   _floats = 0;
   for ( i=0; i<csize; i++ )
	{ _floats += _channels[i].size();
	}
   return _floats;
 }

int KnChannels::floatpos ( int c ) const
{
	if ( c>=_channels.size() ) c=_channels.size()-1;
	int i, floats=0;
	for ( i=0; i<c; i++ )
	{	floats += _channels[i].size();
	}
	return floats;
}

void KnChannels::apply ( const float* src )
{
	int i, csize = _channels.size();
	for ( i=0; i<csize; i++ )
	{	src += _channels[i].apply ( src );
	}
}

void KnChannels::apply ( const float* src, float *destbase )
{
	int i, csize = _channels.size();
	for ( i=0; i<csize; i++ )
	{	src += _channels[i].apply ( src, destbase );
	}
}

void KnChannels::get ( float* fp ) const
{
	int i, csize = _channels.size();
	for ( i=0; i<csize; i++ )
	{	fp += _channels[i].get ( fp );
	}
}

void KnChannels::get_random ( float* fp ) const
{
	int i, csize = _channels.size();
	for ( i=0; i<csize; i++ )
	{	fp += _channels[i].get_random ( fp );
	}
}

void KnChannels::make ( KnSkeleton* sk )
 {
   init ();

   int i, jsize = sk->joints().size();
   KnJoint* joint;
   
   for ( i=0; i<jsize; i++ )
	{ joint = sk->joints()[i];
	  
	  // position channels:
	  if ( !joint->pos()->frozen(0) ) add(joint,KnChannel::XPos);
	  if ( !joint->pos()->frozen(1) ) add(joint,KnChannel::YPos);
	  if ( !joint->pos()->frozen(2) ) add(joint,KnChannel::ZPos);

	  // rotation channels:
	  if ( joint->rot()->frozen() ) continue;

	  switch ( joint->rot_type() )
	   { case KnJoint::TypeQuat:
		  add(joint,KnChannel::Quat);
		  break;
		 case KnJoint::TypeST:
		  add(joint,KnChannel::Swing);
		  if ( !joint->st()->twist_frozen() ) add(joint,KnChannel::Twist);
		  break;
		 case KnJoint::TypeEuler:
		  if ( !joint->euler()->frozen(0) ) add(joint,KnChannel::XRot);
		  if ( !joint->euler()->frozen(1) ) add(joint,KnChannel::YRot);
		  if ( !joint->euler()->frozen(2) ) add(joint,KnChannel::ZRot);
		  break;
	   }
	}

   compress ();
 }

bool KnChannels::make ( const GsArray<KnPosture*>& postures, GsArray<int>* indices )
 {
   init ();
   if ( postures.size()<1 ) return false;

   // check the actual channels being used:
   int i, j;
   GsBuffer<float>& firstpost = postures[0]->values;
   int firstpostsize = firstpost.size();
   GsArray<int> locindices;
   GsArray<int>& index = (indices? *indices:locindices);

   // init as -1 meaning indices not used (or 1 if one channel):
   index.size ( firstpost.size() ); // index has size==floats, not channels
   if ( firstpost.size()==1 )
	index[0] = 1;
   else
	index.setall ( -1 );

   // mark with 1 the indices of active channels, ie, those with different values:
   for ( i=1; i<postures.size(); i++ )
	{ if ( postures[i]->values.size()!=firstpost.size() ) return false;
	  for ( j=0; j<firstpostsize; j++ )
	   { if ( postures[i]->values[j]!=firstpost[j] ) index[j]=1;
	   }
	}

   // now get only the used channels:
   KnChannels& ch = *postures[0]->channels();
   int csize;
   int postsize=0;
   i=0; j=0;
   for ( i=0; i<ch.size(); i++ )
	{ csize = ch[i].size();
	  if ( index[j]<0 ) // channel not used: advance j to next value and continue
	   { j+=csize; continue; }
	  else
	   { while ( csize-->0 ) // save in index the new indices of this channel
		  { index[j++] = postsize++; }
		 add ( ch[i] ); // get the channel
	   }
	}

   return true;
 }

int KnChannels::lsearch ( KnJointName jname, KnChannel::Type type, int* floatpos ) const
 {
   int i;
   int chs = _channels.size();

   if ( floatpos ) *floatpos=0;

   for ( i=0; i<chs; i++ )
	{ if ( _channels[i].type()==type && _channels[i].jname()==jname )
	   { return i;
	   }
	  if ( floatpos ) *floatpos += _channels[i].size();
	}

   return -1;
 }

int KnChannels::search ( KnJointName name, KnChannel::Type type ) const
 {
   if ( _channels.size()==0 ) return -1; 
   if ( !_htable ) rebuild_hash_table();
   return _htable->lookup ( name.id(), (char)type );
 }

void KnChannels::rebuild_hash_table () const
 {
   int i, csize=_channels.size();
   if ( !csize ) return;
   if ( !_htable ) _htable = new HashTable;

   // build the hash table:
   _htable->init ( csize*2 );
   for ( i=0; i<csize; i++ )
	{ _htable->insert ( _channels[i].jname().id(), (char)_channels[i].type(), (gsuint16)i );
	  // duplicated entries will not be inserted, but they should not exist.
	}
	
   //gsout<<"Channels size:"<<size();
   //gsout<<" htable size:"<<_htable->table.size()<<" longest:"<<_htable->longest_entry()<<gsnl;
 }

int KnChannels::connect ( const KnSkeleton* s )
{
	int i;
	int count = 0;
	int chsize = _channels.size();
	KnJoint* j=0;

	if ( !s ) { disconnect(); return 0; }

	for ( i=0; i<chsize; i++ )
	{	j = s->joint(_channels[i].jname());
		if ( j )
		{	_channels[i].connect ( j );
			count++;
		}
		else
		{	_channels[i].disconnect();
		}
	}
	return count;
}

// Without using the hash table, with 53 channels, a test performed 1431 comparisons
// to accomplish the mapping. With a hash size of 53*2=106, longest entry 2, 111 elements,
// and only 111-106==5 colliding elements, the number of comparisons was (53-5)+2*5==58!
int KnChannels::connect ( const KnPosture* p )
 {
   disconnect();
   if ( !p ) return 0;
   if ( p->values.empty() ) gsout.fatal("Connecting to posture with empty values!");

   const KnChannels* pch = p->cchannels();
   if ( !pch ) return 0;

   int i, pos, count=0, fcount=0;
   int pchsize = pch->size();

   // iterate over target buffer because otherwise cannot integrate target pointers
   for ( i=0; i<pchsize; i++ )
	{ pos = search ( pch->cget(i).jname(), pch->cget(i).type() );
	  if ( pos>=0 )
	   { _channels[pos].connect ( &(p->values[fcount]) );
		 count++;
	   }
	  fcount += pch->cget(i).size();
	}

   return count;
 }

int KnChannels::connect_indices ( const KnPosture* p )
 {
   disconnect();
   if ( !p ) return 0;

   const KnChannels* pch = p->cchannels();
   if ( !pch ) return 0;

   int i, pos, count=0;
   int pchsize = pch->size();

   for ( i=0; i<pchsize; i++ )
	{ pos = search ( pch->cget(i).jname(), pch->cget(i).type() );
	  if ( pos>=0 )
	   { _channels[pos].connect ( i );
		 count++;
	   }
	}

   return count;
 }

int KnChannels::count_connected () const
 {
   int i, n, chsize=_channels.size();
   for ( i=n=0; i<chsize; i++ )
	{ if ( _channels[i].status()!=KnChannel::Disconnected )
	   { n++;
		 if ( _channels[i].buffer()==0 ) 
		  { gsout.fatal("Null Connection Found: [%s]\n",(const char*)_channels[i].jname()); }
	   }
	}
   return n;
 }

int KnChannels::count_disconnected () const
 {
   int i, n, chsize=_channels.size();
   for ( i=n=0; i<chsize; i++ ) 
	{ if ( _channels[i].status()==KnChannel::Disconnected ) n++;
	   else if ( _channels[i].buffer()==0 ) 
			  { gsout.fatal("Null Connection Found: [%s]\n",(const char*)_channels[i].jname()); }
	}
   return n;
 }

void KnChannels::disconnect ()
 {
   int i, chsize=_channels.size();
   for ( i=0; i<chsize; i++ ) _channels[i].disconnect();
 }

void KnChannels::merge ( const KnChannels& ca )
 {
   int c, id;
   for ( c=0; c<ca.size(); c++ ) // for each channel c in ca
	{ id = search ( ca(c).jname(), ca(c).type() );
	  if ( id<0 ) // missing channel found
	   { add ( ca(c).jname(), ca(c).type() ); // add it
	   }
	}
 }

bool KnChannels::only_quat_channel () const
{
	int c, s=_channels.size();
	for ( c=0; c<s; c++ )
	{	if ( _channels[c].type()!=KnChannel::Quat ) return false;
	}
	return true;
}

int KnChannels::force_quat_channels ()
{
	int c, s=_channels.size(), n=0;
	for ( c=0; c<s; c++ )
	{	if ( _channels[c].type()!=KnChannel::Quat )
		{	_channels[c].type ( KnChannel::Quat );
			n++;
		}
		if ( _channels[c].status()>KnChannel::JointConnection )
			_channels[c].disconnect();
	}
	return n;
}

void KnChannels::copyfrom ( const KnChannels& c )
 {
   _channels = c._channels;
   _floats = c._floats;
   if ( _htable ) { delete _htable; _htable=0; }
 }

bool KnChannels::operator == ( const KnChannels& c )
 {
   if ( size()!=c.size() ) return false;
   
   int i;
   for ( i=0; i<size(); i++ )
	{ if ( _channels[i]!=c._channels[i] ) return false;
	}
   return true;
 }

GsOutput& operator<< ( GsOutput& o, const KnChannels& c )
 {
   int i;
   o << "channels " << c.size() << gsnl;
   for ( i=0; i<c.size(); i++ )
	{ o << (const char*)c(i).jname() << gspc << c(i).type_name() << gsnl;
	  //o << " status:"<<c(i).status()<<gsnl; // extra message for debug use only
	}
   return o;
 }
 
GsInput& operator>> ( GsInput& in, KnChannels& c )
 {
   in.get(); // read "channels"

   int n=-1;
   if ( in.check()==GsInput::Number ) n=in.geti(); // number of channels

   c.init ();
   c._channels.capacity(256); // reserve 256 entries

   KnJointName jname;
   while ( n!=0 )
	{ 
	  jname = in.gets();
	  c.add ( jname, KnChannel::type(in.gets()) );
	  if ( n<0 )
	   { if ( in.check()==GsInput::Delimiter ) { in.get(); break; } }
	  else
	   { n--; }
	}		 

   c.compress();

   return in;
 }

void KnChannels::htableout ( GsOutput& o ) const
 {
   if ( _htable ) _htable->out(o);
 }

//========================= KnChannels::HashTable ============================

static int hash ( int jname, char ctype, int size )
 {
   int h = jname + (int)ctype;
   GS_SETPOS(h);
   h = h%size;
   return h;
 }

void KnChannels::HashTable::init ( int hsize )
 {
   hash_size = hsize;
   table.capacity ( hsize );
   table.size ( hsize );
   int i;
   for ( i=0; i<hsize; i++ )
	table[i].set ( -1/*joint*/, -1/*channel*/, 0/*pos*/, -1/*next*/ );
 }

int KnChannels::HashTable::longest_entry () const
 {
   int i, j, len, longest=0;
   for ( i=0; i<hash_size; i++ )
	{ if ( table[i].jname_key<0 ) continue; // empty
	  len = 1;
	  j = table[i].next;
	  while ( j>=0 ) { len++; j=table[j].next; }
	  if ( len>longest ) longest=len;
	}
   return longest;
 }

int KnChannels::HashTable::lookup ( int jname, char ctype ) const
 {
   int id = ::hash ( jname, ctype, hash_size );
   if ( table[id].jname_key<0 ) return -1; // empty entry, not found

   while ( true )
	{ if ( table[id].jname_key==jname && table[id].ctype_key==ctype ) // found
	   return (int)table[id].pos_data;
	  // else check next colliding entry:
	  if ( table[id].next<0 ) return -1; // no more entries, not found
	  id = table[id].next;
	}
 }

int KnChannels::HashTable::insert ( int jname, char ctype, gsuint16 pos )
 {
   int id = ::hash ( jname, ctype, hash_size );

   if ( table[id].jname_key<0 ) // empty entry, just take it
	{ table[id].set ( jname, ctype, pos, -1/*next*/ );
	  return id;
	}

   while ( true )
	{ if ( table[id].jname_key==jname && table[id].ctype_key==ctype ) return -1; // already there

	  // else check next colliding entry:
	  if ( table[id].next<0 ) // no more entries, add one:
	   { table[id].next = table.size();
		 table.push().set ( jname, ctype, pos, -1/*next*/ );
		 return table.size()-1;
	   }
	  
	  id = table[id].next;
	}
 }

void KnChannels::HashTable::out ( GsOutput& o )
 {
   for ( int i=0; i<table.size(); i++ )
	{ if ( table[i].jname_key<0 ) continue;
	  o << i << ":";
	  o << " name:"<<KnJointName::st(table[i].jname_key);
	  o << " nameid:"<<table[i].jname_key;
	  o << " type:"<<int(table[i].ctype_key);
	  o << " pos:"<<int(table[i].pos_data);
	  o << " nxt:"<<int(table[i].next);
	  o << gsnl;
	}
 }

//============================ End of File ============================

