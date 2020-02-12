/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>
# include <sig/gs_vars.h>
# include <sig/gs_string.h>

//====================================== GsVars =======================================

GsVars::GsVars ( const GsVars& v ) : GsShareable()
 {
   _table.size ( v.size() );
   int i;
   for ( i=0; i<_table.size(); i++ )
	_table[i] = new GsVar ( v[i] );

   _name = 0;
   gs_string_set ( _name, v._name );
 }

GsVars::~GsVars ()
 {
   delete _name;
   init();
 }

void GsVars::name ( const char* n )
 {
   gs_string_set ( _name, n );
 }

const char* GsVars::name () const
 {
   return _name? _name:"";
 }

void GsVars::init ()
 {
   while ( _table.size()>0 )
	delete _table.pop();
 }

GsVar* GsVars::test ( const char* name ) const
 {
   int i = search ( name ); 
   if ( i<0 ) gsout.fatal ( "Var [%s] not found!", name );
   return _table[i];
 }

GsVar* GsVars::get ( const char* name ) const
 {
   int i = search ( name ); 
   return i<0? 0 : _table[i];
 }

//===== get =====

bool GsVars::get ( const char* name, int& value, int defvalue )
 {
   int i = search ( name ); 
   if ( i<0 )
	{ value=defvalue; return false; }
   else
	{ value=get(i).geti(); return true; }
 }

bool GsVars::get ( const char* name, bool& value, bool defvalue )
 {
   int i = search ( name ); 
   if ( i<0 )
	{ value=defvalue; return false; }
   else
	{ value=get(i).getb(); return true; }
 }

bool GsVars::get ( const char* name, float& value, float defvalue )
 {
   int i = search ( name ); 
   if ( i<0 )
	{ value=defvalue; return false; }
   else
	{ value=get(i).getf(); return true; }
 }

bool GsVars::get ( const char* name, double& value, double defvalue )
 {
   int i = search ( name ); 
   if ( i<0 )
	{ value=defvalue; return false; }
   else
	{ value=(double)get(i).getf(); return true; }
 }

bool GsVars::get ( const char* name, char*& value, const char* defvalue )
 {
   int i = search ( name ); 
   if ( i<0 )
	{ gs_string_set(value,defvalue); return false; }
   else
	{ gs_string_set(value,get(i).gets()); return true; }
 }

bool GsVars::get ( const char* name, GsString& value, const char* defvalue )
 {
   int i = search ( name ); 
   if ( i<0 )
	{ value=defvalue; return false; }
   else
	{ value=get(i).gets(); return true; }
 }

GsVec GsVars::getv ( const char* name ) const
 {
   GsVec vec;
   GsVar* v = test(name);
   if ( !v ) return vec;
   vec.set ( v->getf(0), v->getf(1), v->getf(2) );
   return vec;
 }

void GsVars::getv ( const char* name, float* fp, int size ) const
 {
   GsVar* v = test(name);
   if ( !v ) return;
   int i;
   for ( i=0; i<size; i++ ) fp[i] = v->getf(i);
 }

//===== set =====

GsVar* GsVars::set ( const char* name, int value, int index )
 {
   int i = search ( name ); 
   if ( i<0 ) return 0;
   _table[i]->set(value,index);
   return _table[i];
 }

GsVar* GsVars::set ( const char* name, bool value, int index )
 {
   int i = search ( name ); 
   if ( i<0 ) return 0;
   _table[i]->set(value,index);
   return _table[i];
 }

GsVar* GsVars::set ( const char* name, float value, int index )
 {
   int i = search ( name ); 
   if ( i<0 ) return 0;
   _table[i]->set(value,index);
   return _table[i];
 }

GsVar* GsVars::set ( const char* name, const char* value, int index )
 {
   int i = search ( name ); 
   if ( i<0 ) return 0;
   _table[i]->set(value,index);
   return _table[i];
 }

static int fcmp ( GsVar* const* pt1, GsVar* const* pt2 )
 {
   return GsVar::compare ( *pt1, *pt2 );
 }

int GsVars::search ( const char* name ) const
 {
   GsVar v;
   v.name ( name );
   return _table.bsearch ( &v, fcmp ); 
 }

int GsVars::add ( GsVar* v )
 {
   int pos;
   pos = _table.uniqinsort ( v, fcmp );
   if ( pos<0 ) // already there
	{ pos = _table.bsearch ( v, fcmp );
	  _table[pos]->init(*v);
	  delete v;
	}
   return pos;
 }

void GsVars::remove ( int i )
 {
   if ( i<0 || i>=_table.size() ) return;
   delete _table[i];
   _table.remove ( i );
 }

void GsVars::merge ( GsVars& vt )
 {
   int i, id;
   
   for ( i=0; i<vt.size(); i++ )
	{ id = search(vt.get(i).name());
	  if ( id>=0 ) // found with same name
	   { get(id).init ( vt.get(i) ); // get the value
		 delete vt._table[i];
		 vt._table[i] = 0;
	   }
	}

   for ( i=0; i<vt.size(); i++ )
	{ if ( vt._table[i] )
	   _table.push() = vt._table[i];
	}

   _table.sort ( fcmp );
   vt._table.size(0);
 }

GsVar* GsVars::extract ( int i )
 {
   if ( i<0 || i>=_table.size() ) return 0;
   GsVar* v = _table[i];
   _table.remove ( i );
   return v;
 }

GsOutput& operator<< ( GsOutput& o, const GsVars& v )
 {
   int i;

   if ( v.name()[0] ) o<<GsSafeWrite(v.name())<<gspc;

   o << '{' << gsnl;
   switch ( v._outfmt )
	{ case GsVars::ReverseFormat:
		   for( i=v.size()-1; i>=0; i-- ) o << v[i] << gsnl;
		   break;
	  default: // GsVars::NormalFormat case
		   for( i=0; i<v.size(); i++ ) o << v[i] << gsnl;
	}
   o << '}' << gsnl;

   return o;
 }

GsInput& operator>> ( GsInput& in, GsVars& v )
 {
   v.init ();
   v.name ( 0 );

   in.get(); // name or '{'
   if ( in.ltype()==GsInput::String )
	{ v.name ( in.ltoken() );
	  in.get(); // now it should come '{'
	  if ( in.ltoken()[0]!='{' && in.ltoken()[0]!='[' ) // consider there is no name scope
	   { in.unget(); in.unget(v.name()); v.name(0); }
	}

   while ( true )
	{ GsInput::TokenType t = in.check();
	  if ( t==GsInput::End ) break;
	  if ( t==GsInput::Delimiter )
	   { int c = in.getc();
		 if ( c=='}' || c==']' ) break; else in.unget();
	   }
	  v._table.push () = new GsVar;
	  in >> *v._table.top();
	}

   // sort elements in case data was edited by hand
   // note: duplicated entries are not fixed
   v._table.sort ( fcmp );
   
   return in;
 }

//================================ End of File =================================================


