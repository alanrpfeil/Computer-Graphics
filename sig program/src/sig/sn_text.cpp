/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_text.h>
# include <sig/gs_font.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
# include <sig/gs_trace.h>

//======================================= SnText ====================================

const char* SnText::class_name = "SnText";
SN_SHAPE_RENDERER_DEFINITIONS(SnText);

SnText::SnText () : SnPlanarObjects ( class_name )
{
	GS_TRACE1 ( "Constructor" );
	_scaling = 1.0f;
}

SnText::~SnText ()
{
	GS_TRACE1 ( "Destructor" );
}

void SnText::init ()
{
	touch();
	_entries.init();
}

void SnText::add_entry ()
{
	_entries.push();
}

void SnText::compress ()
{
	_entries.compress();
}

void SnText::font_style ( const GsFontStyle& fs )
{
	touch();
	cur().fs = fs;
}

GsFontStyle& SnText::font_style ()
{
	touch();
	return cur().fs;
}

void SnText::pos ( float x, float y )
{ 
	touch();
	cur().position.set(x,y);
}

void SnText::zcoordinate ( float z )
{
	touch();
	SnPlanarObjects::zcoordinate = z;
}

void SnText::scaling ( float s )
{
	touch();
	_scaling = s;
}

void SnText::set ( const char* st )
{ 
	touch();
	cur().text.set(st);
}

void SnText::append ( const char* st )
{ 
	touch();
	cur().text.append(st);
}

void SnText::color ( GsColor c )
{ 
	touch();
	Entry& e = cur();
	if ( e.e3c ) { delete[] e.e3c; e.e3c=0; }
	e.color = c;
}

void SnText::xmax ( float xmax )
{ 
	touch();
	Entry& e = cur();
	e.xmax = xmax;
}

void SnText::multicolor ( GsColor c1, GsColor c2, GsColor c3, GsColor c4 )
{ 
	touch();
	Entry& e = cur();
	if ( !e.e3c ) e.e3c = new GsColor[3];
	e.color = c1;
	e.e3c[0] = c2;
	e.e3c[1] = c3;
	e.e3c[2] = c4;
}

const char* SnText::ctext ( int i )
{
	if ( empty() ) add_entry();
	if ( i<0 || i>=entries() ) i = entries()-1;
	return _entries[i]->text.pt; 
}

//================================ EOF =================================================
