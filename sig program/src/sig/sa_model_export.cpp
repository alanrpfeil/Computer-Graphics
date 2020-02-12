/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sa_model_export.h>
# include <sig/sn_model.h>

//# define GS_USE_TRACE1 // Const / Dest 
//# define GS_USE_TRACE2 // 
# include <sig/gs_trace.h>

//=============================== SaModelExport ====================================

SaModelExport::SaModelExport ( const GsString& dir )
 { 
   GS_TRACE1 ( "Constructor" );
   directory ( dir );
   _num = 0;
   _prefix = "model";
 }

SaModelExport::~SaModelExport ()
 {
   GS_TRACE1 ( "Destructor" );
 }

void SaModelExport::directory ( const GsString& dir )
 {
   _dir = dir;
   char c = _dir.lchar();
   if ( c==0 || c=='/' || c=='\\' ) return;
   _dir << '/';
 }

bool SaModelExport::apply ( SnNode* n )
 {
   _num = 0;
   bool result = SaAction::apply ( n );
   return result;
 }

//==================================== virtuals ====================================

bool SaModelExport::shape_apply ( SnShape* s )
 {
   if ( gs_compare(s->instance_name(),"model")!=0 ) return true;
   if ( !s->visible() ) return true;

   const GsMat& mat = get_top_matrix();

   GsModel model;
   model = *((SnModel*)s)->model();
   model.transform ( mat );

   GsString fname;
   fname.setf ( "%s%s%04d.gsm", (const char*)_dir,(const char*)_prefix, _num++ );
   GsOutput out ( fopen(fname,"wt") );
   if ( !out.valid() ) return false;

   model.save ( out );

   return true;
 }

//======================================= EOF ====================================

