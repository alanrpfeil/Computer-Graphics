/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sa_render_mode.h>
# include <sig/sn_shape.h>

//# define GS_USE_TRACE1 // constructor / destructor
# include <sig/gs_trace.h>

//=============================== SaRenderMode ====================================

bool SaRenderMode::shape_apply ( SnShape* s )
 {
   if ( _override )
	 s->override_render_mode ( _render_mode );
   else
	 s->restore_render_mode ();

   return true;
 }

//======================================= EOF ====================================
