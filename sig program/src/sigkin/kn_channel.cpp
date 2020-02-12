/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <memory.h>
# include <sigkin/kn_channel.h>
# include <sigkin/kn_joint.h>

//============================= KnChannel ============================

void KnChannel::connect ( KnJoint* j )
{
	if ( !j )
	{	disconnect();
	}
	else
	{	_status = JointConnection;
		_jname = j->name();
		_con.j = j;
	}
}

void KnChannel::connect ( float* f )
{
	_status = BufferConnection;
	_con.f = f;
}

void KnChannel::connect ( int i )
{
	_status = IndexConnection;
	_con.i = i;
}

void KnChannel::init ( Type t )
{
	_type = t;
	_status = Disconnected;
	_con.j = 0;
}

KnSkeleton* KnChannel::skeleton () const
{
	if ( _status!=KnChannel::JointConnection ) return 0;
	return _con.j->skeleton();
}

int KnChannel::apply ( const float* v ) const
 {
   if ( _status==JointConnection )
	{
	  if ( _type==Quat )
	   { _con.j->quat()->value ( v ); // will take 4 elements
		 return 4;
	   }
	  else if ( _type<=ZRot )
	   { 
		if ( _type<=ZPos )
		 { _con.j->pos()->value ( _type, v[0] );
		   return 1;
		 }
		else // _type<=ZRot
		 { _con.j->euler()->value ( _type-3, v[0] );
		   return 1;
		 }
	   }
	  else if ( _type==IKPos )
	   { _con.j->iksolve ( GsVec(v[0],v[1],v[2]) ); // will take 3 elements
		 return 3;
	   }
	  else if ( _type==IKGoal )
	   { _con.j->iksolve ( GsVec(v[0],v[1],v[2]), GsQuat(v+3) ); // will take 7 elements
		 return 7;
	   }
	  else if ( _type==Swing )
	   { _con.j->st()->swing ( v[0], v[1] );
		 return 2;
	   }
	  else // Twist
	   { _con.j->st()->twist ( v[0] );
		 return 1;
	   }
	}
   else if ( _status==BufferConnection )
	{
	  if ( _type==Quat )
	   { _con.f[0] = v[0];
		 _con.f[1] = v[1];
		 _con.f[2] = v[2];
		 _con.f[3] = v[3];
		 return 4;
	   }
	  else if ( _type==IKGoal )
	   { memcpy ( _con.f/*dest*/, v/*src*/, sizeof(float)*7 );
		 return 7;
	   }
	  else if ( _type==IKPos )
	   { _con.f[0] = v[0];
		 _con.f[1] = v[1];
		 _con.f[2] = v[2];
		 return 3;
	   }
	  else if ( _type==Swing )
	   { _con.f[0] = v[0];
		 _con.f[1] = v[1];
		 return 2;
	   }
	  else
	   { _con.f[0] = v[0];
		 return 1;
	   }
	}
   else // Disconnected
	{
	  return size();
	}
 }

int KnChannel::apply ( const float* v, float* destbase ) const
 {
   int s = size ();
   if ( _status==IndexConnection )
	{ memcpy ( destbase+_con.i/*dest*/, v/*src*/, sizeof(float)*s );
	}
   return s;
 }

int KnChannel::get ( float* v ) const
 {
   if ( _status==JointConnection )
	{
	  if ( _type<=ZPos )
	   { v[0] = _con.j->cpos()->value ( _type );
		 return 1;
	   }
	  else if ( _type<=ZRot )
	   { v[0] = _con.j->ceuler()->value ( _type-3 );
		 return 1;
	   }
	  else if ( _type==Quat )
	   { const GsQuat& q = _con.j->quat()->value();
		 v[0] = q.w;
		 v[1] = q.x;
		 v[2] = q.y;
		 v[3] = q.z;
		 return 4;
	   }
	  else if ( _type==IKPos )
	   {
		 _con.j->ikgetmat();
		 if ( _con.j->ik() )
		  { _con.j->ik()->goal.getrans(v[0],v[1],v[2]); }
		 else
		  { v[0]=v[1]=v[2]=0; }
		 return 3;
	   }
	  else if ( _type==IKGoal )
	   {
		 _con.j->ikgetmat();
		 if ( _con.j->ik() )
		  { _con.j->ik()->goal.getrans(v[0],v[1],v[2]);
			GsQuat q;
			mat2quat ( _con.j->ik()->goal, q );
			memcpy ( v+3/*dest*/,q.e/*src*/,sizeof(float)*4);
		  }
		 else memset(v,0,sizeof(float)*7);
		 return 7;
	   }
	  else if ( _type==Swing )
	   { v[0] = _con.j->st()->swingx();
		 v[1] = _con.j->st()->swingy();
		 return 2;
	   }
	  else // Twist
	   { v[0] = _con.j->st()->twist();
		 return 1;
	   }
	}
   else if ( _status==BufferConnection )
	{
	  if ( _type==Quat )
	   { v[0] = _con.f[0];
		 v[1] = _con.f[1];
		 v[2] = _con.f[2];
		 v[3] = _con.f[3];
		 return 4;
	   }
	  else if ( _type==Swing )
	   { v[0] = _con.f[0];
		 v[1] = _con.f[1];
		 return 2;
	   }
	  else
	   { v[0] = _con.f[0];
		 return 1;
	   }
	}
   else // Disconnected
	{
	  if ( _type==Quat )
	   { v[0] = v[1] = v[2] = v[3] = 0;
		 return 4;
	   }
	  else if ( _type==Swing )
	   { v[0] = v[1] = 0;
		 return 2;
	   }
	  else
	   { v[0] = 0;
		 return 1;
	   }
	}
 }

int KnChannel::get_random ( float* v ) const
{
	if ( _status!=JointConnection ) return get(v);
	
	if ( _type<=ZPos )
	{	v[0] = _con.j->cpos()->random ( int(_type) );
		return 1;
	}
	else if ( _type<=ZRot )
	{	v[0] = _con.j->ceuler()->random ( int(_type)-3 );
		return 1;
	}
	else if ( _type==Quat )
	{	GsQuat q;
		q.random ();
		v[0] = q.w;
		v[1] = q.x;
		v[2] = q.y;
		v[3] = q.z;
		return 4;
	}
	else if ( _type==Swing )
	{	_con.j->cst()->get_random_swing ( v[0], v[1] );
		return 2;
	}
	else // Twist
	{	v[0] = _con.j->cst()->get_random_twist();
		return 1;
	}
}

int KnChannel::interp ( Type _type, const float* v1, const float* v2, float t, float* v )  // static
 {
   # define MIX(a,b) ((a)*(r)+(b)*(t))
   if ( _type==Quat )
	{ gslerp ( (float*)v1, v2, t, v );
	  return 4;
	}
   if ( _type==IKGoal )
	{ float r = 1-t;
	  v[0] = MIX ( v1[0], v2[0] );
	  v[1] = MIX ( v1[1], v2[1] );
	  v[2] = MIX ( v1[2], v2[2] );
	  gslerp ( (float*)(v1+3), v2+3, t, v+3 );
	  return 7;
	}
   if ( _type==IKPos )
	{ float r = 1-t;
	  v[0] = MIX ( v1[0], v2[0] );
	  v[1] = MIX ( v1[1], v2[1] );
	  v[2] = MIX ( v1[2], v2[2] );
	  return 3;
	}
   if ( _type<=ZPos )
	{ v[0] = GS_MIX ( v1[0], v2[0], t );
	  return 1;
	}
   if ( _type<=ZRot )
	{ v[0] = gs_anglerp ( v1[0], v2[0], t );
	  return 1;
	}
   if ( _type==Swing )
	{ float r = 1.0f-t;
	  v[0] = MIX ( v1[0], v2[0] );
	  v[1] = MIX ( v1[1], v2[1] );
	  return 2;
	}
   // else
	{ v[0]= gs_anglerp ( v1[0], v2[0], t );
	  return 1;
	}
   # undef MIX
 }

int KnChannel::interp ( Type _type, const GsArray<float*>& values, const GsArray<float>& w, float* v )  // static
 {
   int i;
   if ( _type==Quat )
	{ GsQuat Q (0.0f, 0.0f, 0.0f, 0.0f);
	  for ( i=0; i<values.size(); i++ )
		{ //gsout<<values[i][0]<<gstab<<values[i][1]<<gstab<<values[i][2]<<gstab<<values[i][3]<<gstab<<gsnl;
		  Q.w += values[i][0] * w[i];
		  Q.x += values[i][1] * w[i];
		  Q.y += values[i][2] * w[i];
		  Q.z += values[i][3] * w[i];
		}
		Q.normalize();
		v[0] = Q.w;
		v[1] = Q.x;
		v[2] = Q.y;
		v[3] = Q.z;
		//gsout<<Q.w<<gstab<<Q.x<<gstab<<Q.y<<gstab<<Q.z<<gstab<<gsnl;
		return 4;
	}

   if ( _type<=ZPos )
	{ v[0] = 0;
   	  for ( i=0; i<values.size(); i++)
	   { v[0] += values[i][0] * w[i];
	   }
	  return 1;
	}
	if ( _type<=ZRot )
	{ v[0] = 0;
   	  for ( i=0; i<values.size(); i++ )
	   { v[0] += values[i][0] * w[i];
	   }
	  return 1;
	}
	if ( _type==Swing )
	{ v[0] = 0; v[1] = 0;
   	  for ( i=0; i<values.size(); i++ )
	   { v[0] += values[i][0] * w[i];
		 v[1] += values[i][1] * w[i];
	   }
	  return 2;
	}

   if ( _type==IKGoal )
	{ v[0] = 0; v[1] = 0; v[2] = 0;
   	  for ( i=0; i<values.size(); i++ )
	   { v[0] += values[i][0] * w[i];
		 v[1] += values[i][1] * w[i];
		 v[2] += values[i][2] * w[i];
	   }
	  GsQuat Q(0.0f, 0.0f, 0.0f, 0.0f);
	  for ( i=0; i<values.size(); i++ )
	   { Q.w += values[i][3] * w[i];
		 Q.x += values[i][4] * w[i];
		 Q.y += values[i][5] * w[i];
		 Q.z += values[i][6] * w[i];
	   }
	  Q.normalize();
	  v[3] = Q.w;
	  v[4] = Q.x;
	  v[5] = Q.y;
	  v[6] = Q.z;
	  return 7;
	}
   if ( _type==IKPos )
	{ v[0] = 0; v[1] = 0; v[2] = 0;
   	  for ( i=0; i<values.size(); i++)
	   { v[0] += values[i][0] * w[i];
		 v[1] += values[i][1] * w[i];
		 v[2] += values[i][2] * w[i];
	   }
	  return 3;
	}

	// else Twist
	{ v[0] = 0;
   	  for ( i=0; i<values.size(); i++ )
	   { v[0] += values[i][0] * w[i];
	   }
	  return 1;
	}
 }

int KnChannel::change_values ( Type type, float* values, float mfactor, const float* offset ) // static
 {
   if ( type==Quat )
	{ GsQuat qv(values), qo(offset);
	  if ( mfactor>0 )
	   { GsVec axis;
		 float radians;
		 qv.get ( axis, radians );
		 qv.set ( axis, radians*mfactor );
	   }
	  qv = qo * qv;
	  values[0] = qv.w;
	  values[1] = qv.x;
	  values[2] = qv.y;
	  values[3] = qv.z;
	  return 4;
	}
   else if ( type==Swing )
	{ values[0] = values[0]*mfactor + offset[0];
	  values[1] = values[1]*mfactor + offset[1];
	  return 2;
	}
   else if ( type==IKPos )
	{ values[0] = values[0]*mfactor + offset[0];
	  values[1] = values[1]*mfactor + offset[1];
	  values[2] = values[2]*mfactor + offset[2];
	  return 3;
	}
   else if ( type==IKGoal )
	{ change_values ( IKPos, values, mfactor, offset );
	  change_values ( Quat, values+3, mfactor, offset );
	  return 7;
	}
   else // 1 DOF types
	{ values[0] = values[0]*mfactor + offset[0];
	  return 1;
	}
 }
 
const char* KnChannel::type_name ( Type type ) // static
 {
   switch ( type )
	{ case XPos : return "XPos";
	  case YPos : return "YPos";
	  case ZPos : return "ZPos";
	  case Quat : return "Quat";
	  case XRot : return "XRot";
	  case YRot : return "YRot";
	  case ZRot : return "ZRot";
	  case IKPos : return "IKPos";
	  case IKGoal : return "IKGoal";
	  case Swing : return "Swing";
	  case Twist : return "Twist";
	  default: return "Invalid";
	}
 }

KnChannel::Type KnChannel::type ( const char* s ) // static
 {
   # define IS(st) gs_compare(s,st)==0
   if ( IS("Quat") ) return KnChannel::Quat;
   if ( IS("IKGoal") ) return KnChannel::IKGoal;
   if ( IS("IKPos") ) return KnChannel::IKPos;
   if ( IS("Swing") ) return KnChannel::Swing;
   if ( IS("Twist") ) return KnChannel::Twist;
   if ( IS("XRot") ) return KnChannel::XRot;
   if ( IS("YRot") ) return KnChannel::YRot;
   if ( IS("ZRot") ) return KnChannel::ZRot;
   if ( IS("XPos") ) return KnChannel::XPos;
   if ( IS("YPos") ) return KnChannel::YPos;
   if ( IS("ZPos") ) return KnChannel::ZPos;
   if ( IS("XRotation") ) return KnChannel::XRot;
   if ( IS("YRotation") ) return KnChannel::YRot;
   if ( IS("ZRotation") ) return KnChannel::ZRot;
   if ( IS("XPosition") ) return KnChannel::XPos;
   if ( IS("YPosition") ) return KnChannel::YPos;
   if ( IS("ZPosition") ) return KnChannel::ZPos;
   return KnChannel::Invalid;
   # undef IS
 }

static void printv ( GsOutput& o, float v ) // optimized print
 {
   FILE* f = o.filept();
   int i = int(v);

   if ( float(i)==v )
	{ if (f) fprintf ( f, "%d", i ); else o<<i; }
   else
	{ if (f) fprintf ( f, "%f", v ); else o<<v; }
 }
 
int KnChannel::save ( Type type, GsOutput& o, const float* v ) // static
 {
   if ( type<=ZPos )
	{ printv ( o, v[0] );
	  return 1;
	}
   if ( type<=ZRot || type==Twist )
	{ float ang = GS_TODEG(v[0]);
	  printv ( o, ang );
	  return 1;
	}
   if ( type==Quat )
	{ GsQuat q ( v[0], v[1], v[2], v[3] );
	  GsVec axis = q.axis();
	  float ang = q.angle();
	  axis.len ( ang );
	  printv ( o, axis.x ); o<<gspc;
	  printv ( o, axis.y ); o<<gspc;
	  printv ( o, axis.z );
	  return 4; // the channel size is 4
	}
   if ( type==IKPos )
	{ printv ( o, v[0] ); o<<gspc;
	  printv ( o, v[1] ); o<<gspc;
	  printv ( o, v[2] );
	  return 3;
	}
   if ( type==IKGoal )
	{ save ( IKPos, o, v ); o<<gspc;
	  save ( Quat, o, v+3 );
	  return 7;
	}
   if ( type==Swing )
	{ printv ( o, v[0] ); o<<gspc;
	  printv ( o, v[1] );
	  return 2;
	}
   return 0;
 }

int KnChannel::load ( Type type, GsInput& in, float* v ) // static
 {
   if ( type<=ZPos )
	{ in >> v[0];
	  return 1;
	}
   if ( type<=ZRot || type==Twist )
	{ in >> v[0];
	  v[0] = GS_TORAD(v[0]);
	  return 1;
	}
   if ( type==Quat )
	{ GsVec axisang;
	  in >> axisang;
	  GsQuat q ( axisang );
	  v[0] = q.w;
	  v[1] = q.x;
	  v[2] = q.y;
	  v[3] = q.z;
	  return 4;
	}
   if ( type==Swing )
	{ in >> v[0] >> v[1];
	  return 2;
	}
   if ( type==IKPos )
	{ in >> v[0] >> v[1] >> v[2];
	  return 3;
	}
   if ( type==IKGoal )
	{ load ( IKPos, in, v );
	  load ( Quat, in, v+3 );
	  return 7;
	}
   return 0;
 }

//============================ End of File ============================
