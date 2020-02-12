/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_points.h>
# include <sig/gs_vec2.h>
# include <sig/gs_box.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= SnPoints ====================================

const char* SnPoints::class_name = "SnPoints";
SN_SHAPE_RENDERER_DEFINITIONS(SnPoints);

SnPoints::SnPoints () : SnShape(class_name)
{
	GS_TRACE1 ( "Constructor" );
	_psize=1;
}

SnPoints::~SnPoints ()
{
	GS_TRACE1 ( "Destructor" );
}

void SnPoints::init ()
{
	P.size(0);
	C.size(0);
	touch();
}

void SnPoints::compress ()
{
	P.compress();
	C.compress();
}

void SnPoints::push ( float x, float y, float z )
{
	if ( C.size()>0 )
	{	GsColor x = C.top();
		while ( C.size()<=P.size() ) C.push()=x;
	}
	P.push().set(x,y,z);
	touch();
}

void SnPoints::push ( float x, float y, float z, GsColor c )
{ 
	if ( C.size()<P.size() )
	{	GsColor x = C.empty()? color() : C.top();
		while ( C.size()<P.size() ) C.push()=x;
	}
	C.push() = c;
	P.push().set(x,y,z);
	touch();
}

void SnPoints::get_bounding_box ( GsBox& b ) const
{
	int i;
	b.set_empty ();
	for ( i=0; i<P.size(); i++ ) 
	{	b.extend ( P[i] );
	}
}

//================================ EOF =================================================
