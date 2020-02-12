/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sa_bbox.h>
# include <sig/sn_shape.h>

//# define GS_USE_TRACE1 // constructor / destructor
# include <sig/gs_trace.h>

//================================== SaBBox ====================================

bool SaBBox::shape_apply ( SnShape* s )
{
	if ( !s->visible() ) return true;
	GsBox b;
	s->get_bounding_box ( b );
	_box.extend ( get_top_matrix()*b );
	return true;
}

//======================================= EOF ====================================
