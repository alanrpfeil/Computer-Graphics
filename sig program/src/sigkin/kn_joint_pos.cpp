/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs.h>
# include <sig/gs_random.h>

# include <sigkin/kn_joint.h>
# include <sigkin/kn_joint_pos.h>

//============================= KnJointPos ============================

void KnJointPos::value ( int d, float val )
 {
   if ( KnVecLimits::value(d,val) )
	{ _joint->set_lmat_changed();
	}
 }

void KnJointPos::value ( float x, float y, float z )
 {
   bool a = KnVecLimits::value(0,x);
   bool b = KnVecLimits::value(1,y);
   bool c = KnVecLimits::value(2,z);
   if ( a||b||c ) { _joint->set_lmat_changed(); }
 }

//============================ End of File ============================
