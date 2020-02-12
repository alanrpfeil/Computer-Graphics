/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigkin/kn_ik.h>
# include <sig/sn_lines.h>
# include <sigkin/kn_coldet.h>
# include <sigkin/kn_skeleton.h>

/* ImprNote:
	- To check: one the orbit angle, when the foot is almost stretched it returned a value of -3.nnn
	- Implement singularity-free orbit angle control
	- Implement hybrid analytic-CCD with manipulation ellipsoid as heuristic.
*/

//======================================= KnIk =====================================

void KnIk::_init () // private init method
{
	_type = LeftArm;
	_solve_rot_goal = true;
	_solve_closest = false;

	_base = _mid = _end = 0;
	_midflexaxis = Y;

	_midtwist = 0;
	_midtwistaxis = Y;

	_snlines = 0;

	for ( int i=0; i<7; i++ ) _result[i]=0;
   
	_d1 = _d2 = -1;
}

KnIk::KnIk ()
{
	_init ();
}

KnIk::KnIk ( Type t, KnJoint* end )
{
	_init ();
	init ( t, end );
}

KnIk::~KnIk()
{
	if ( _snlines ) _snlines->unref();
}

void KnIk::lines ( SnLines* sl )
{
	if ( _snlines ) _snlines->unref(); // unref old
	if ( sl ) sl->ref(); // ref new
	_snlines = sl; // store new
}

bool KnIk::init ( Type t, KnJoint *base, KnJoint* mid, RotAxis midflexaxis, KnJoint* end,
						  KnJoint* midtwist, RotAxis midtwistaxis )
{
	_base = _mid = _end = 0;
	_midtwist = 0;
	_d1 = _d2 = -1;
   
	if ( !base || !mid || !end ) return false;

	if ( base->rot_type()!=KnJoint::TypeST ) return false;
	if ( mid->rot_type()!=KnJoint::TypeEuler ) return false;
	if ( end->rot_type()!=KnJoint::TypeST ) return false;
   
	if ( midtwist )
		if ( midtwist->rot_type()!=KnJoint::TypeEuler ) return false;

	_base = base;
	_mid = mid;
	_end = end;
	_midtwist = midtwist;
	_midflexaxis = midflexaxis;
	_midtwistaxis = midtwistaxis;
	_type = t;

	return true;
}

bool KnIk::init ( Type t, KnSkeleton* kn, const char* base, const char* mid, RotAxis midflexaxis,
				  const char* end, const char* midtwist, RotAxis midtwistaxis )
{
	if ( !kn ) return false;
	KnJoint *b = kn->joint(base);
	KnJoint *m = kn->joint(mid);
	KnJoint *e = kn->joint(end);
	KnJoint *mt = 0;
	if ( midtwist )
	{	mt = kn->joint(end);
		if ( !mt ) return false;
	}
	return init ( t, b, m, midflexaxis, e, mt, midtwistaxis );
}

bool KnIk::init ( Type t, KnJoint* end )
{
	if ( !end ) return false;
	if ( end->rot_type()!=KnJoint::TypeST ) return false;

	KnJoint* mid = end->parent();
	if ( !mid ) return false;
	if ( mid->rot_type()!=KnJoint::TypeEuler ) return false;

	KnJoint* base = mid->parent();
	if ( !base ) return false;

	int i;
	int midfaxis = -1;
	int midtaxis = -1;

	if ( base->rot_type()!=KnJoint::TypeST ) // there must be a midtwist joint
	{	KnJoint* midt = mid;
		mid = base;
		base = base->parent(); // base must be now in SwingTwist (checked in the other init())
		if ( mid->rot_type()!=KnJoint::TypeEuler ) return false;
		for ( i=0; i<3; i++ ) if ( !mid->euler()->frozen(i) ) break;
		if ( i==3 ) return false; // no active joint to put the flexion
		midfaxis = i;
		for ( i=0; i<3; i++ ) if ( !midt->euler()->frozen(i) ) break;
		if ( i==3 ) return false; // no active joint to put the twist
		midtaxis = i;
		return init ( t, base, mid, (RotAxis)midfaxis, end, midt, (RotAxis)midtaxis );
	}
	else if ( end->st()->twist_frozen() ) // mid joint has both flexion and twist
	{	for ( i=0; i<3; i++ )
		{	if ( !mid->euler()->frozen(i) )
			{	if (mid->euler()->lolim(i)==0 || mid->euler()->uplim(i)==0)
					midfaxis=i; // flexion has always one limit equal to 0
				else
					midtaxis=i; // twist has always both limits different than 0
			}
		}
		//gs_out<<mid->name()<<": "<<midfaxis<<gsspc<<midtaxis<<gsnl;
		if ( midfaxis<0 || midtaxis<0 ) return false; // could not guess rotation axis
		return init ( t, base, mid, (RotAxis)midfaxis, end, mid, (RotAxis)midtaxis );
	}
	else // mid joint has only flexion
	{	for ( i=0; i<3; i++ ) if ( !mid->euler()->frozen(i) ) midfaxis=i;
		if ( midfaxis<0 ) return false; // no active DOF for flexion!
		return init ( t, base, mid, (RotAxis)midfaxis, end );
	}
}

void KnIk::remove_joint_limits ()
{
	if ( !_base ) return;
	_base->st()->ellipse ( gspi, gspi );
	_base->st()->twist_limits ( -gspi, gspi );
	_mid->euler()->limits ( _midflexaxis, -gspi, gspi );
	_end->st()->ellipse ( gspi, gspi );
	if ( _midtwist )
		_midtwist->euler()->limits ( _midtwistaxis, -gspi, gspi );
	else
		_end->st()->twist_limits ( -gspi, gspi );
}

void KnIk::update_base_up ()
{
	if ( !_base ) return;
	if ( _base->parent() )
	{	if ( !_base->skeleton()->global_matrices_uptodate() ) _base->parent()->update_gmat_up();
	}
}
 
void KnIk::update_base_end ()
{
	if ( !_base ) return;
	_base->update_gmat_local();
	_mid->update_gmat_local();
	if ( _midtwist!=_mid ) _midtwist->update_gmat_local();
	_end->update_gmat_local();
}

void KnIk::base_frame ( GsMat& bframe )
{
	if ( !_base ) return;
   
	// bframe will be the same as : {_base->init_rot(); bframe=_base->gmat();}
	// but we do this without changing joint values (without calling init_rot)
	GsMat jmat;
	quat2mat ( _base->quat()->prerot(), jmat );
	jmat.setrans ( _base->offset() ); // mat upd

	if ( _base->parent() )
	{	bframe.mult ( _base->parent()->gmat(), jmat ); // mat upd
	}
}

void KnIk::set_local ( GsMat& goal )
{
	GsMat bframe;
	base_frame ( bframe );
	bframe.invert();
	GsMat local;
	local.mult ( bframe, goal ); // put goal matrix in local frame // mat upd
	goal = local;
}

float KnIk::linkage_len ()
{
	if ( _d1<0 )
	{	_d1 = _mid->offset().len(); // could be optimized if guaranteed along single axis
		_d2 = _end->offset().len();

		if ( _midtwist && _midtwist!=_mid )
		{	_d2 += _midtwist->offset().len();
		}
	}
	
	return _d1+_d2;
}

// usually we get here precision of 10E-5 */
KnIk::Result KnIk::solve ( const GsMat& goal, float oang )
{  
	/* Notation:
	  m = mid point in base frame
	  e = end point in base frame
	  c = center of the orbit circle in base frame
	  r = orbit circle radius
	  u,v = orbit angle frame axis
	  d1 = distance from the base to the mid joint
	  d2 = distance from the mid joint to the end joint
	  dist = distance from the base to the goal point
	  midr = the found flexion rotation of the mid joint
	  basest = vector where (x,y)=found swing and z=sound twist of the base joint
	*/

	// End point:
	GsPnt e ( goal.e14, goal.e24, goal.e34 );

	// 1. get the lengths of the limbs and check goal distance:
	//	(offsets must be along one single axis!)
	if ( _d1<0 ) linkage_len();
	float d1 = _d1;
	float d2 = _d2;
	float d2d2 = d2*d2;
	float d1d1 = d1*d1;
	float dist = e.len();
	float distdist = dist*dist;
	if ( dist==0 ) return NotReachable; // not reachable
	bool aim=false;
	if ( dist>d1+d2 )
	{	if ( !_solve_closest ) return NotReachable; // not reachable
		dist = d1+d2;
		distdist = dist*dist;
		e.len ( dist );
		aim = true; // goal too far so just aim at it
	}

	// 2. Find the flexion of the mid joint:
	float k = ( d1d1 + d2d2 - distdist ) / ( 2.0f * d2 * d1 );
	if (k<=-1) k=-1; //this case happens when the effector is out of reach 
	float midr = gspi - acosf( k ); // the 2 solutions are: gspi+-acos(k)

	GsVec axis = -GsVec::j; // axis projected to form 2D orbit angle frame

	if ( _type==LeftArm )
	{	midr=-midr; oang*=-1;
	}
	else if ( _type==RightLeg )
	{	oang=-oang+gspidiv2;
	}
	else if ( _type==LeftLeg )
	{	oang=oang+gspidiv2;
	}

	_result[3] = midr; // Store mid flexion

	// 3. Specify the orbit angle frame (u,v) and mid position m:
	GsVec n = e/dist; // n is the unit vector pointing from the base to the end joint
	GsVec u = axis + n*dot(-axis,n); // u is the projection of the Y axis in the mid's rotation circle
	u.normalize();
	GsVec v = cross ( u, n ); // v is orthogonal to n and u
	float cosa = (d2d2-d1d1-distdist) / (-2.0f*d1*dist);
	float sina = (d2*sinf(gspi-midr)) / dist;
	GsPnt c = (cosa*d1)*n; // c is the center of the circle in local coords
	float r;
	GsPnt m = c;
	if ( aim ) { r=0; c=e*_d1; }
	else
	{	r = d1 * GS_ABS(sina); // r is the circle radius
		m += r * ( u*cosf(oang) + v*sinf(oang) ); // mid pos in _base coords
	}

	// Draw things if required:
	if ( _snlines )
	{	GsMat bframe;
		base_frame ( bframe );
		GsPnt shoulderp ( bframe.e14, bframe.e24, bframe.e34 ); // _base joint in global coords
		GsPnt bfc=bframe*c, bfm=bframe*m, bfe=bframe*e;
		GsMat bframe3x3 = bframe;
		bframe3x3.setrans(0,0,0);
		n = e/dist; // unit vector pointing from the base to the end joint
		SnLines& lines = *_snlines;
		lines.init();
		lines.push ( GsColor::cyan );
		lines.push ( shoulderp, bfe ); // _base-end line
		lines.push ( GsColor::red );
		lines.push ( bfc, bframe*(c+u) ); // u vector (red)
		lines.push ( GsColor::green );
		lines.push ( bfc, bframe*(c+v) ); // v vector (green)
		lines.push ( GsColor::cyan );
		if ( r>0 ) lines.push_circle_approximation ( bfc, bframe3x3*(u*r), bframe3x3*n, 60 ); // orbit circle
		lines.push ( GsColor::cyan );
		lines.push ( bfc, bfm ); // circenter-mid line
		lines.push ( GsColor::magenta );
		lines.push ( shoulderp, bfm ); // _base-mid line
		lines.push ( bfm, bfe );
	}

	// 4. Specify base joint axis-angle to reach mid position:
	n.set ( -m.y, m.x, 0.0f ); // exactly the same as: n = cross ( GsVec::k, m );
	n.len ( acosf(m.z/d1) ); // exactly the same as: n.len ( angle ( GsVec::k, m ) );
   
	_result[0] = n.x; // Store base swing
	_result[1] = n.y;

	// 5. Specify base twist to reach the end position:
	GsQuat QS ( n ); // The base swing rotation (init from axis-angle n)
	GsQuat QE ( GsVec::j, midr ); // The elbow flexion rotation

	if ( (d1+d2-dist)<1.0E-5f ) 
	{	_result[2] = 0; // use zero base twist when hierarchy has max extension
	}
	else
	{	GsVec b ( 0.0f, 0.0f, d2 );
		GsVec a = QE.apply(b); // a.z component is not used for _result[2]
		b = QS.inverse().apply(e);
		// solve Rz a = b => (Cz -Sz 0, Sz Cz 0, 0 0 1) (ax ay az) = (bx by bz)
		// => tan (z) = ((by*ax-ay*bx)/(ax*ax-ay*ay)) / ((bx*ax+by*ay)/(ax*ax+ay*ay))
		_result[2] = atan2f ( a.x*b.y - a.y*b.x, a.x*b.x + a.y*b.y ); // Store base twist
	}

	// 6. Determine the end rotation matrix:
	if ( _solve_rot_goal )
	{
		GsQuat QT ( GsVec::k, _result[2] ); // twist rotation of the shoulder
		GsQuat QW = QS * QT * QE;			// composed swing*twist*elbow rotations
		GsQuat QG ( goal );					// goal rotation

		if ( _end->quat()->hasprepost() )  // there is a postrot rotation at the wrist
			QG = QG * _end->quat()->postrot().inverse();

		// We want to find q, such that: QG = QW * q;
		GsQuat q = QW.inverse() * QG;

		// Now we need to decompose q into the final 3 DOF rotations, according to the
		// defined linkage, which may require that the twist comes before or after the
		// swing, thus requiring different computations for each of these 2 cases.
   
		if ( _midtwist==0 ) // the twist comes after the swing, ie, in the twist DOF of the end joint
		{
			quat2st ( q, _result[4], _result[5], _result[6] );
		}
		else // the twist comes before, in the twist euler joint, before the end joint
		{
			quat2ts ( q, _result[4], _result[5], _result[6] );
		}
	}
	else // leave end rotation untouched
	{
	}

	// In "solve closest" mode we always return ok:
	if (_solve_closest) return Ok;

	// The computation is done, now check limits:
	if ( !_base->st()->swing_inlimits(_result[0],_result[1]) ) return NoBaseSwing;
	if ( !_base->st()->twist_inlimits(_result[2]) ) return NoBaseTwist;
	if ( !_mid->euler()->inlimits(_midflexaxis,_result[3]) ) return NoMidFlexion;
	if ( _midtwist==0 )
	{	if ( !_end->st()->swing_inlimits(_result[4],_result[5]) ) return NoEndSwing;
		if ( !_end->st()->twist_inlimits(_result[6]) ) return NoMidTwist;
	}
	else
	{	if ( !_midtwist->euler()->inlimits(_midtwistaxis,_result[4]) ) return NoMidTwist;
		if ( !_end->st()->swing_inlimits(_result[5],_result[6]) ) return NoEndSwing;
	}

	return Ok;
}

KnIk::Result KnIk::solve ( const GsMat& goal, float oang, KnColdet* coldet )
{
	Result res = solve ( goal, oang );
	if ( res!=Ok ) return res;
	apply_last_result ();
	if ( !coldet ) return res;
	coldet->update ( _base->skeleton() );
	return coldet->collide()? Collision:Ok;
}

KnIk::Result KnIk::solve ( const GsMat& goal, KnIkOrbitSearch& osearch, KnColdet* coldet )
{
	osearch.iterations = 1;
	osearch.oangle = osearch.init;

	Result res = KnIk::solve ( goal, osearch.oangle, coldet );
	if ( res==Ok || res==NotReachable ) return res;

	float inc  = osearch.inc;
	float ang1 = osearch.init-inc;
	float ang2 = osearch.init+inc;

	bool run = true;
	while ( run )
	{	run = false;
		inc += osearch.rate;
		if ( osearch.min<=ang1 )
		{	osearch.iterations++;
			osearch.oangle = ang1;
			res = KnIk::solve ( goal, ang1, coldet );
			if ( res==Ok ) break;
			ang1 -= inc;
			run = true;
		}
		if ( ang2<=osearch.max )
		{	osearch.iterations++;
			osearch.oangle = ang2;
			res = KnIk::solve ( goal, ang2, coldet );
			if ( res==Ok ) break;
			ang2 += inc;
			run = true;
		}
	}

	return res;
}

float KnIk::orbit_angle ()
{  
	// Get end e position in local coordinates:
	GsMat mat = _end->gmat();
	set_local ( mat );
	GsPnt e ( mat[12], mat[13], mat[14] );

	// Specify the orbit angle frame (u,v) and the center c of the orbit circle:
	if ( _d1<0 ) linkage_len();
	float d1 = _d1;
	float d2 = _d2;
	float dist = e.len();
	if ( dist>d1+d2-0.0001f ) return 0; // any value could be the orbit angle with total extension
	GsVec axis = -GsVec::j; // axis projected to form 2D orbit angle frame
	GsVec n = e/dist; // n is the unit vector pointing from the _base to the end joint
	GsVec u = axis + n*dot(-axis,n); // u is the projection of the Y axis in the mid's rotation circle
	u.normalize();
	GsVec v = cross ( u, n ); // v is orthogonal to n and u
	float cosa = ((d2*d2)-(d1*d1)-(dist*dist)) / (-2.0f*d1*dist);
	GsPnt c = cosa*d1*n; // c is the center of the circle in local coords
   
	// Get the current mid position from the skeleton:
	GsPnt m ( _mid->gmat()[12], _mid->gmat()[13], _mid->gmat()[14] ); // global mid position
	base_frame ( mat );
	m = m * mat.inverse(); // put m in the shoulder frame (local coordinates)
	m = m-c; // put m in relation to c
	mat.identity();
	mat.setl1 ( u ); // set mat to be the uvn frame
	mat.setl2 ( v );
	mat.setl3 ( n ); // (a,b,c)*uvnframe=m => (a,b,c)=m*uvnframe^-1
	m = m * mat.inverse(); // a in u axis, b in v axis, c==0
   
	// Find the orbit angle in relation to (u,v) frame:
	float oang = atan2f ( m.y, m.x );
	if ( _type==LeftArm )
	{	oang=-oang;
	}
	else if ( _type==RightLeg )
	{	oang=gspidiv2-oang; if ( oang>gspi ) oang-=gs2pi;
	}
	else if ( _type==LeftLeg )
	{
		oang=oang-gspidiv2; if ( oang<-gspi ) oang+=gs2pi; 
	}

	return oang;
}

void KnIk::get_sk_values ( float values[7] )
{
	if ( !_base ) return;
	values[0] = _base->st()->swingx();
	values[1] = _base->st()->swingy();
	values[2] = _base->st()->twist();
	values[3] = _mid->euler()->value ( _midflexaxis );
   
	if ( _midtwist )
	{	values[4] = _midtwist->euler()->value ( _midtwistaxis );
		values[5] = _end->st()->swingx();
		values[6] = _end->st()->swingy();
	}
	else
	{	values[4] = _end->st()->swingx();
		values[5] = _end->st()->swingy();
		values[6] = _end->st()->twist();
	}
}

void KnIk::apply_values ( const float values[7] )
{
	if ( !_base ) return;
	_base->st()->swing ( values[0], values[1] );
	_base->st()->twist ( values[2] );
	_mid->euler()->value ( _midflexaxis, values[3] );
   
	if ( _midtwist )
	{	_midtwist->euler()->value ( _midtwistaxis, values[4] ); 
		_end->st()->swing ( values[5], values[6] );
	}
	else
	{	_end->st()->swing ( values[4], values[5] );
		_end->st()->twist ( values[6] );
	}
}

//===== static methods ======

const char* KnIk::message ( Result res )
{
	switch (res)
	{	case Ok : return "Ok";
		case NotReachable : return "Not Reachable";
		case NoBaseSwing : return "No Base Swing";
		case NoBaseTwist : return "No Base Twist";
		case NoMidFlexion : return "No Mid Flexion";
		case NoMidTwist : return "No Mid Twist";
		case NoEndSwing : return "No End Swing";
		case Collision : return "Collision";
		default: break; // it seems gcc needs this
	}
	return "Undef";
}

bool KnIk::configure_skeleton ( KnJoint* e, const GsQuat& pre, const GsQuat& post, float l[10] )
{
	KnJoint* m = e->parent();
	if ( !m ) return false;
	KnJoint* b = m->parent();
	if ( !b ) return false;

	b->rot_type ( KnJoint::TypeST ); // base joint
	b->rot()->thaw();
	b->rot()->st()->ellipse ( l[0], l[1] ); // base swing limits
	b->rot()->st()->twist_limits (true);
	b->rot()->st()->twist_limits ( l[2], l[3] ); // base twist limits
	b->rot()->prerot ( pre );
	b->rot()->postrot ( post );

	m->rot_type ( KnJoint::TypeEuler ); // mid joint
	m->euler()->type ( KnJointEuler::TypeZY );
	m->rot()->thaw();
	m->euler()->limits ( 1, l[4], l[5] ); // mid Y limits
	m->euler()->limits ( 2, l[6], l[7] ); // mid Z limits
	m->rot()->prerot ( pre );
	m->rot()->postrot ( post );

	e->rot_type ( KnJoint::TypeST ); // end joint
	e->rot()->thaw();
	e->rot()->st()->ellipse ( l[8], l[9] ); // end swing limits
	e->rot()->prerot ( pre );
	e->rot()->postrot ( post );

	return true;
}

bool KnIk::configure_skeleton ( KnJoint* e, Type t )
{
	if ( t==LeftArm )
	{	float l[10] = { GS_TORAD(130), GS_TORAD(175), GS_TORAD(-90), GS_TORAD(120), // base limits
						GS_TORAD(-160), 0, GS_TORAD(-48), GS_TORAD(130), // mid limits
						GS_TORAD(60), GS_TORAD(90) }; // end limits
		GsQuat pre ( GsVec(0,1,0), GS_TORAD(90) );
		GsQuat post ( GsVec(0,1,0), GS_TORAD(-90) );
		return configure_skeleton ( e, pre, post, l );
	}
	else if ( t==RightArm )
	{	float l[10] = { GS_TORAD(130), GS_TORAD(175), GS_TORAD(-90), GS_TORAD(120), // base limits
						0, GS_TORAD(160), GS_TORAD(-130), GS_TORAD(48), // mid limits
						GS_TORAD(60), GS_TORAD(90) }; // end limits
		GsQuat pre ( GsVec(0,1,0), GS_TORAD(-90) );
		GsQuat post ( GsVec(0,1,0), GS_TORAD(90) );
		return configure_skeleton ( e, pre, post, l );
	}
	else if ( t==LeftLeg )
	{	float l[10] = { GS_TORAD(65), GS_TORAD(135), GS_TORAD(-100), GS_TORAD(35), // base limits
						0, GS_TORAD(160), GS_TORAD(-75), GS_TORAD(55), // mid limits
						GS_TORAD(45), GS_TORAD(75) }; // end limits
		GsQuat pre ( GsVec(1,1,-1), GS_TORAD(120) );
		GsQuat post ( GsVec(1,1,-1), GS_TORAD(-120) );
		return configure_skeleton ( e, pre, post, l );
	}
	else if ( t==RightLeg )
	{	float l[10] = { GS_TORAD(65), GS_TORAD(135), GS_TORAD(-35), GS_TORAD(100), // base limits
						0, GS_TORAD(160), GS_TORAD(-55), GS_TORAD(75), // mid limits
						GS_TORAD(45), GS_TORAD(75) }; // end limits
		GsQuat pre ( GsVec(1,1,-1), GS_TORAD(120) );
		GsQuat post ( GsVec(1,1,-1), GS_TORAD(-120) );
		return configure_skeleton ( e, pre, post, l );
	}
	return false;
}

//======================================= KnIkOrbitSearch =====================================

void KnIkOrbitSearch::set_defaults ( KnIk::Type t )
{
	init = GS_TORAD(32.0f);
	inc  = GS_TORAD(2.0f);
	rate = GS_TORAD(0.1f);
	oangle = init;
	iterations = 0;
   
	switch ( t )
	{	case KnIk::LeftArm :
		case KnIk::RightArm : min = GS_TORAD(-15.0f);
							  max = GS_TORAD(130.0f);
							  break;
		case KnIk::LeftLeg :
		case KnIk::RightLeg : min = GS_TORAD(-30.0f);
							  max = GS_TORAD(90.0f);
							  break;
	}
}

//======================================= EOF =====================================
