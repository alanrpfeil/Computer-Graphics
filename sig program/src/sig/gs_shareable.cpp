/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_shareable.h>
# include <sig/gs_output.h>

//================================ GsShareable ========================================

GsShareable::~GsShareable() 
{ 
	if (_ref) gsout.fatal("GsShareable object deleted with ref>0"); 
}

void GsShareable::unref() 
{
	if (_ref==0) gsout.fatal("GsShareable::unref() called for 0 references");
	_ref--;
	if (_ref==0) delete this; // will trigger chain of virtual destructors
}

void unrefref ( GsShareable* obj1, GsShareable* obj2 )
{
	if ( obj2 ) obj2->ref(); // calling this one first is needed
	if ( obj1 ) obj1->unref();
}

//============================== End of File ========================================
