/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_node.h>
# include <sig/gs_buffer.h>

//# define GS_USE_TRACE1  // SnNode Const/Dest
# include <sig/gs_trace.h>

//======================================= SnNode ====================================

SnNode::SnNode ( SnNode::Type t, const char* class_name ) 
 {
   GS_TRACE1 ( "Constructor" );
   _visible = 1;
   _nodeuptodate = 0;
   _type = t;
   _udata = 0;
   _instance_name = class_name;
 }

SnNode::~SnNode ()
 {
   GS_TRACE1 ( "Destructor" );
   delete _udata;
 }

int SnNode::user_data ( void* pt )
 { 
   if ( !_udata ) _udata=new GsBuffer<void*>;
   _udata->push();
   _udata->top()=pt;
   return _udata->size()-1;
 }

void* SnNode::user_data ( int id ) const 
 { 
   if ( !_udata ) return 0;
   if ( id<0 || id>=_udata->size() ) return 0;
   return _udata->cget(id);
 }

//======================================= EOF ====================================

