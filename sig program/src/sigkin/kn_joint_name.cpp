/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigkin/kn_joint_name.h>

//============================= KnJointName ============================

GsTable<gsintp> KnJointName::_htable;
gsuint16 KnJointName::_undefid = gsuint16(65535);

void KnJointName::operator= ( const char* st )
 {
   // check st:
   if ( !st ) { _id=_undefid; return; }

   // make sure hash table has been initialized:
   _check();
   
   // insert index:
   _htable.insert ( st, 0 ); // user data is not used
	
   // get new or duplicated entry id:
   _id = _htable.lastid();
   
   //gsout<<"KnJointName Hash Table Longest Entry: "<<_htable.longest_entry()<<gsnl;
   //gsout<<"KnJointName Hash Table Size: "<<_htable.size()<<gsnl;
 }

bool KnJointName::operator== ( const char* st )
 {
   int id = _htable.lookup_index(st);
   if ( id<0 ) return false;
   return id==int(_id);
 }

bool KnJointName::exists ( const char* name ) // static
 {
   return _htable.lookup_index(name)<0? false:true;
 }

//============================ End of File ============================
