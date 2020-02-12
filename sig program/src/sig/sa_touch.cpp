/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sa_touch.h>

//# define GS_USE_TRACE1 // constructor / destructor
# include <sig/gs_trace.h>

//=============================== SaTouchShapes ====================================

//--------------------------------- virtuals -----------------------------------

bool SaTouch::shape_apply ( SnShape* s )
 {
   s->touch();
   return true;
 }

//======================================= EOF ====================================
