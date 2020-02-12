/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <ctype.h>
# include <string.h>
# include <stdlib.h>

# include <sig/gs_var.h>
# include <sig/gs_string.h>

//======================================== GsVar =======================================

GsVar::GsVar ()
 {
   _name = 0;
   _type = 'i';
 }

GsVar::GsVar ( char type )
 {
   _name = 0;
   _type = type;
   if ( type=='i' || _type=='f' || type=='s' || type=='b' ) return;
   _type = 'i';
 }

GsVar::GsVar ( const char* name, char type )
 {
   _name = 0;
   gs_string_set ( _name, name );
   _type = type;
   switch ( _type )
	{ case 'i': break;
	  case 'f': break;
	  case 's': break;
	  case 'b': break;
	  default: _type = 'i';
	}
 }

GsVar::GsVar ( const char* name, bool value )
 {
   _name = 0;
   gs_string_set ( _name, name );
   _type = 'b';
   _data.push().b = value;
 }

GsVar::GsVar ( const char* name, int value )
 {
   _name = 0;
   gs_string_set ( _name, name );
   _type = 'i';
   _data.push().i = value;
 }

GsVar::GsVar ( const char* name, float value )
 {
   _name = 0;
   gs_string_set ( _name, name );
   _type = 'f';
   _data.push().f = value;
 }

GsVar::GsVar ( const char* name, const char* value )
 {
   _name = 0;
   gs_string_set ( _name, name );
   _type = 's';
   _data.push().s = 0;
   gs_string_set ( _data[0].s, value );
 }

GsVar::GsVar ( const GsVar& v )
 {
   _type = 'i';
   _name = 0;
   init ( v );
 }

GsVar::~GsVar ()
 {
   name ( 0 );
   init ( 'i' );
 }

void GsVar::name ( const char* n )
 {
   gs_string_set ( _name, n );
 }

const char* GsVar::name () const
 {
   return _name? _name:"";
 }

void GsVar::init ( char type )
 {
   if ( _type=='s' ) // delete all used string
	{ while ( _data.size() ) delete[] _data.pop().s;
	}

   _data.size ( 0 );
   _type = type;

   switch ( _type )
	{ case 'i': break;
	  case 'f': break;
	  case 's': break;
	  case 'b': break;
	  default: _type = 'i';
	}
 }

void GsVar::init ( const GsVar& v )
 {
   name ( v.name() );
   init ( v.type() );

   if ( _type!='s' )
	{ _data = v._data;
	}
   else
	{ int i;
	  _data.size ( v.size() );
	  for ( i=0; i<_data.size(); i++ )
	   { _data[i].s = 0;
		 gs_string_set ( _data[i].s, v._data[i].s );
	   }
	}  
 }

void GsVar::size ( int ns )
 {
   int i, s = _data.size();

   if ( ns<s )
	{ if ( _type=='s' )
	   { for ( i=ns; i<s; i++ ) delete[] _data[i].s; }
	  _data.size ( ns );
	}
   else if ( ns>s )
	{ _data.size ( ns );
	  for ( i=s; i<ns; i++ ) _data[i].s=0; // ok for all types
	}
 }

//--------------------------- set value ---------------------------

void GsVar::set ( bool b, int index )
 {
   switch ( _type )
	{ case 'b':
	   if ( index<0 || index>=_data.size() )
		{ _data.push().b = b; }
	   else
		{ _data[index].b = b; }
	   break;

	  case 'i': set ( (int)b, index ); break;
	  case 'f': set ( (float)b, index ); break;
	  case 's': set ( b? "true":"false", index ); break;
	}
 }

# define SNPRINTF(st,v) char s[64]; snprintf(s,64,st,v);

void GsVar::set ( int i, int index )
 {
   switch ( _type )
	{ case 'i':
	   if ( index<0 || index>=_data.size() )
		{ _data.push().i = i; }
	   else
		{ _data[index].i = i; }
	   break;

	  case 'f': set ( float(i), index ); break;
	  case 'b': set ( bool(i? true:false), index ); break;
	  case 's': { SNPRINTF("%d",i); set ( s, index ); } break;
	}
 }

void GsVar::set ( float f, int index )
 {
   switch ( _type )
	{ case 'f':
	   if ( index<0 || index>=_data.size() )
		{ _data.push().f = f; }
	   else
		{ _data[index].f = f; }
	   break;

	  case 'i': set ( int(f), index ); break;
	  case 'b': set ( bool(f==1.0f? true:false), index ); break;
	  case 's': { SNPRINTF("%f",f); set ( s, index ); } break;
	}
 }

# undef SNPRINTF

void GsVar::set ( const char* s, int index )
 {
   switch ( _type )
	{ case 's':
	   if ( index<0 || index>=_data.size() )
		{ _data.push().s = 0;
		  index = _data.size()-1;
		}
	   gs_string_set ( _data[index].s, s );
	   break;

	  case 'i': set ( atoi(s), index ); break;
	  case 'f': set ( atof(s), index ); break;
	  case 'b': set ( bool(::gs_compare(s,"true")==0? true:false), index ); break;
	}
 }

void GsVar::push ( bool b )
 {
   if ( _type!='b' ) gsout.fatal("GsVar is not of type b!");
   _data.push().b = b;
 }

void GsVar::push ( int i )
 {
   if ( _type!='i' ) gsout.fatal("GsVar is not of type i!");
   _data.push().i = i;
 }

void GsVar::push ( float f )
 {
   if ( _type!='f' ) gsout.fatal("GsVar is not of type f!");
   _data.push().f = f;
 }

void GsVar::push ( const char* s )
 {
   if ( _type!='s' ) gsout.fatal("GsVar is not of type s!");
   _data.push().s = 0;
   gs_string_set ( _data.top().s, s );
 }

//--------------------------- get value ---------------------------

bool GsVar::getb ( int index ) const
 {
   if ( index<0 || index>=_data.size() ) return 0;
   switch ( _type )
	{ case 's': if ( !_data[index].s ) return false;
				if ( ::gs_compare(_data[index].s,"true")==0 ) return true;
				return false;
	  case 'i': return _data[index].i? true:false;
	  case 'f': return _data[index].f==0? false:true;
	  case 'b': return _data[index].b;
	}
   return false;
 }

int GsVar::geti ( int index ) const
 {
   if ( index<0 || index>=_data.size() ) return 0;
   switch ( _type )
	{ case 's': return _data[index].s? atoi(_data[index].s):0;
	  case 'i': return _data[index].i;
	  case 'f': return (int)_data[index].f;
	  case 'b': return (int)_data[index].b;
	}
   return 0;
 }

float GsVar::getf ( int index ) const
 {
   if ( index<0 || index>=_data.size() ) return 0;
   switch ( _type )
	{ case 's': return _data[index].s? (float)atof(_data[index].s):0;
	  case 'i': return (float)_data[index].i;
	  case 'f': return _data[index].f;
	  case 'b': return (float)_data[index].b;
	}
   return 0;
 }

const char* GsVar::gets ( int index ) const
 {
   if ( index<0 || index>=_data.size() ) return 0;
   if ( _type=='s' && _data[index].s )
	return _data[index].s;
   return "";
 }

//-----------------------------------------------------------------

void GsVar::remove ( int i, int n )
 {
   if ( n<=0 || i<0 || i>=_data.size() ) return;
   if ( _type=='s' )
	{ int pi, pe=i+n;
	  for ( pi=i; pi<=pe && pi<_data.size(); pi++ ) 
	   delete[] _data[pi].s;
	}
   _data.remove ( i, n );
 }

void GsVar::insert ( int i, int n )
 {
   if ( n<=0 || i<0 || i>_data.size() ) return;
   _data.insert ( i, n );
   while ( i<i+n )
	 _data[i].s = 0; // will work for all types
 }

GsVar& GsVar::operator= ( const GsVar& v )
 {
   init ( v );
   return *this;
 }

//============================== friends ========================================

GsOutput& operator<< ( GsOutput& o, const GsVar& v )
 {
   if ( v._name==0 )
	o << "var";
   else if ( v._name[0]==0 )
	o << "var";
   else
	o << v._name;

   o << " = ";

   if ( v._data.size()==0 )
	{ switch ( v._type )
	   { case 'b' : o<<"false;"; break;
		 case 'i' : o<<"0;";	 break;
		 case 'f' : o<<"0.0;";   break;
		 case 's' : o<<"\"\";";  break;
	   }
	  return o;
	}

   int len=0;
   int i, s=v._data.size();
   int e = s-1;
   GsString buf;

   switch ( v._type )
	{ case 'f': for ( i=0; i<s; i++ )
				 { o<<v._data[i].f;
				   len+=1;
				   if (i<e)
					{ if ( len>16 ) { o<<gsnl<<gstab; len=0; }
					   else o<<gspc;
					}
				 } break;

	  case 'i': for ( i=0; i<s; i++ )
				 { o<<v._data[i].i;
				   len+=1;
				   if (i<e)
					{ if ( len>20 ) { o<<gsnl<<gstab; len=0; }
					   else o<<gspc;
					}
				 } break;

	  case 's': for ( i=0; i<s; i++ ) 
				 { buf = v._data[i].s;
				   buf.safewrite ( o );
				   len += buf.len();
				   if (i<e)
					{ if ( len>80 ) { o<<gsnl<<gstab; len=0; }
					   else o<<gspc;
					}
				 } break;

	  case 'b': for ( i=0; i<s; i++ )
				 { o<<v._data[i].b;
				   len += v._data[i].b? 5:7;
				   if (i<e)
					{ if ( len>80 ) { o<<gsnl<<gstab; len=0; }
					   else o<<gspc;
					}
				 } break;
	}

   return o<<';';
 }

GsInput& operator>> ( GsInput& in, GsVar& v )
 {
   GsString buf;

   // parses any kind of names until '=' is found:
   for ( ;; )
	{ in.get();
	  if ( in.end() ) return in;
	  if ( in.ltoken()[0] == '=' ) break;
	  buf << in.ltoken();
	}
   // set the name:
   v.name ( buf );
   v.init ( 'i' );
   in.get();
   if ( in.end() ) return in;

   // get first token and determine GsVar type:
   switch ( in.ltype() )
	{ case GsInput::String : 
		   if ( in.ltoken()=="true" ) 
			{ v.init('b'); v.set(true); break; }
		   else if ( in.ltoken()=="false" )
			{ v.init('b'); v.set(false); break; }
		   else
			{ v.init('s'); v.set((const char*)in.ltoken()); }
		   break;

	  case GsInput::Number : // can be int or float type
		   if ( in.lnumreal() )
			{ v.init('f'); v.set(in.ltoken().atof()); }
		   else
			{ v.init('i'); v.set(in.ltoken().atoi()); }
		   break;

	  default : break;
	};

   // read while ';' is not found:
   in.get();
   if ( in.end() ) return in;
   while ( !in.end() && in.ltoken()[0]!=';' )
	{ switch ( v._type )
	   { case 'f': v.push ( in.ltoken().atof() ); break;
		 case 'i': v.push ( in.ltoken().atoi() ); break;
		 case 's': v.push ( in.ltoken() ); break;
		 case 'b': v.push ( in.ltoken()=="true"? true:false ); break;
	   }
	  in.get();
	}

   return in;
 }

int GsVar::compare ( const GsVar* v1, const GsVar* v2 )
 {
   return ::gs_compare ( v1->name(), v2->name() );
 }

//================================ End of File =================================================


/* Here is a "value comparison" code:

   int cmp = v1->type()-v2->type();
   if ( cmp!=0 ) return cmp;

   switch ( v1->type() )
	{ case 'f': cmp = GS_COMPARE ( v1->getf(), v2->getf() ); break;
	  case 'i': cmp = GS_COMPARE ( v1->geti(), v2->geti() ); break;
	  case 's': cmp = gs_compare ( v1->gets(), v2->gets() ); break;
	  case 'b': cmp = GS_COMPARE ( v1->getb(), v2->getb() ); break;
	}

   return cmp;
*/
