/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_time.h>
# include <sig/gs_string.h>

//======================= GsTime =====================================

GsTime::GsTime ()
 { 
   _h = _m = 0;
   _s = 0;
 }

void GsTime::set ( int h, int m, float s )
 { 
   _h = h;
   _m = m;
   _s = s;

   if ( _s>=60.0f )
	{ m = (int) (_s/60.0f);
	  _s -= (float) (m*60);
	  _m += m;
	}

   if ( _m>=60 )
	{ h = _m/60;
	  _m -= h*60;
	  _h += h;
	}
 }

/* parameters must be positive. */
void GsTime::set ( float h, float m, float s )
 { 
   // First convert to int,int,float :
   _h = (int)h;	 // take the hours
   h -= (float)_h;  // extract hours fraction
   m = m + (h*60);  // accumulate in minutes
   _m = (int)m;	 // take minutes
   m -= (float)_m;  // extract minutes fraction
   _s = s + (m*60); // accumulate in seconds

   // Then, normalize values:
   set ( _h, _m, _s );
 }

void GsTime::add ( const GsTime& t )
 {
   set ( _h+t._h, _m+t._m, _s+t._s );
 }

void GsTime::add_hours ( float h )
 {
   set ( h+(float)_h, (float)_m, _s );
 }

void GsTime::add_minutes ( float m )
 {
   set ( (float)_h, m+(float)_m, _s );
 }

void GsTime::add_seconds ( float s )
 {
   set ( _h, _m, _s+s );
 }

GsTime& GsTime::operator += ( const GsTime& t )
 {
   set ( _h+t._h, _m+t._m, _s+t._s );
   return *this;
 }

GsString& GsTime::print ( GsString& s ) const
 {
   s.setf ( "%02d:%02d:%02d", _h, _m, int(_s) );
   return s;
 }

GsString& GsTime::printms ( GsString& s ) const
 {
   float ms = (_s - float(int(_s))) * 1000.0f;
   s.setf ( "%02d:%02d:%02d.%03d", _h, _m, int(_s), int(ms) );
   return s;
 }

//============================ friends ==========================

GsOutput& operator<< ( GsOutput& o, const GsTime& t )
 {
   GsString s;
   return o << t.print(s);
 }

GsInput& operator>> ( GsInput& in, GsTime& t )
 {
   t._h = in.geti(); in.get();
   t._m = in.geti(); in.get();
   t._s = in.getf();
   return in;
 }

//============================= end of file ==========================
