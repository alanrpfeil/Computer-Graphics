/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>
# include <sig/gs_val.h>
# include <sig/gs_string.h>

//======================================== GsVal =======================================

void GsVal::init ( char type )
 {
   if ( _type=='s' ) gs_string_delete(_val.s);

   _val.s = 0;
   _type = type;
   if ( type=='i' || _type=='f' || type=='s' || type=='b' ) return;
   _type = 'i';
 }

void GsVal::init ( const GsVal& v )
 {
   if ( _type=='s' ) gs_string_delete(_val.s);
   _val.s = 0;
   _type = v.type();

   if ( _type=='s' )
	gs_string_set ( _val.s, v._val.s );
   else
	_val = v._val;
 }

void GsVal::set ( bool b )
 {
   switch ( _type )
	{ case 'b': _val.b=b; break;
	  case 'i': _val.i=int(b); break;
	  case 'f': _val.f=float(b); break;
	  case 's': gs_string_set ( _val.s, b? "true":"false" ); break;
	}
 }

# define SNPRINTF(st,v) char s[64]; snprintf(s,64,st,v);

void GsVal::set ( int i )
 {
   switch ( _type )
	{ case 'i': _val.i=i; break;
	  case 'f': _val.f=float(i); break;
	  case 'b': _val.b=bool(i? true:false); break;
	  case 's': { SNPRINTF("%d",i); gs_string_set(_val.s,s); } break;
	}
 }

void GsVal::set ( float f )
 {
   switch ( _type )
	{ case 'f': _val.f=f; break;
	  case 'i': _val.i=int(f); break;
	  case 'b': _val.b=bool(f==1.0f? true:false); break;
	  case 's': { SNPRINTF("%f",f); gs_string_set(_val.s,s); } break;
	}
 }

# undef SNPRINTF

void GsVal::set ( const char* s )
 {
   switch ( _type )
	{ case 's': gs_string_set(_val.s,s); break;
	  case 'i': _val.i=atoi(s); break;
	  case 'f': _val.f=(float)atof(s); break;
	  case 'b': _val.b=bool(gs_compare(s,"true")==0? true:false); break;
	}
 }

//--------------------------- get value ---------------------------

bool GsVal::getb () const
 {
   switch ( _type )
	{ case 's': if ( !_val.s ) return false;
				return gs_compare(_val.s,"true")==0;
	  case 'i': return _val.i? true:false;
	  case 'f': return _val.f==0? false:true;
	  case 'b': return _val.b;
	}
   return false;
 }

int GsVal::geti () const
 {
   switch ( _type )
	{ case 's': return _val.s? atoi(_val.s):0;
	  case 'i': return _val.i;
	  case 'f': return (int)_val.f;
	  case 'b': return (int)_val.b;
	}
   return 0;
 }

float GsVal::getf () const
 {
   switch ( _type )
	{ case 's': return _val.s? (float)atof(_val.s):0;
	  case 'i': return (float)_val.i;
	  case 'f': return _val.f;
	  case 'b': return (float)_val.b;
	}
   return 0;
 }

const char* GsVal::gets () const
 {
   if ( _type=='s' && _val.s ) return _val.s;
   return "";
 }

//============================== friends ========================================

GsOutput& operator<< ( GsOutput& o, const GsVal& v )
 {
   switch ( v._type )
	{ case 'f': return o<<v._val.f;
	  case 'i': return o<<v._val.i;
	  case 's': if (v._val.s) o<<v._val.s; return o;
	  case 'b': return o<<v._val.b;
	}
   return o;
 }

GsInput& operator>> ( GsInput& in, GsVal& v )
 {
   switch ( in.get() )
	{ case GsInput::String : 
		   if ( in.ltoken()=="true" ) 
			{ v.init('b'); v._val.b=true; }
		   else if ( in.ltoken()=="false" )
			{ v.init('b'); v._val.b=false; }
		   else
			{ v.init('s'); gs_string_set ( v._val.s, in.ltoken() ); }
		   break;

	  case GsInput::Number : // can be int or float type
		   if ( in.lnumreal() )
			{ v.init('f'); v._val.f=in.ltoken().atof(); }
		   else
			{ v.init('i'); v._val.i=in.ltoken().atoi(); }
		   break;

	  default : v.init('i'); break;
	};

   return in;
 }

//================================ End of File =================================================

