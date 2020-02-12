/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
 
# include <sig/sn_lines.h>
# include <sig/sn_model.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>

# include <sigkin/kn_scene.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_joint.h>
# include <sigkin/kn_skin_scene.h>

//# define GS_USE_TRACE1  // connect
# include <sig/gs_trace.h>

# define DEF_CYL_RADIUS 1.0f
# define DEF_SPH_FACTOR 1.6f
# define DEF_AXIS_LEN   1.0f

# define DEF_SPH_OFFSETRATIO	0.06f
# define DEF_AXIS_OFFSETRATIO	0.25f
# define DEF_CYLRAD_OFFSETRATIO	0.04f

//# define DEF_SPH_OFFSETRATIO	1
//# define DEF_AXIS_OFFSETRATIO	1
//# define DEF_CYLRAD_OFFSETRATIO	1
//============================= KnScene ============================

KnScene::KnScene ( KnSkeleton* s )
{
	_cradius = DEF_CYL_RADIUS;
	_sfactor = DEF_SPH_FACTOR;
	_axislen = DEF_AXIS_LEN;
	_avgoffsetlen = 1.0f;
	_skeleton = 0;
	_skinsc = 0;
	_skelgroup = 0;
	if ( s ) connect ( s );
}

KnScene::~KnScene ()
{
	if ( _skeleton ) _skeleton->unref();
}

void KnScene::init ()
{
	remove_all ();
	_skinsc = 0;
	_skelgroup = 0;
	_jgroup.capacity ( 0 );
	if ( _skeleton ) { _skeleton->unref(); _skeleton=0; }
}

static SnGroup* make_joint_group ( const KnJoint* j, KnSkeleton* s, GsArray<SnGroup*>& _jgroup )
{
	SnGroup* g = new SnGroup;
	g->separator ( true );
	_jgroup [ j->index() ] = g;

	// insert children recursively
	for ( int i=0; i<j->children(); i++ )
		g->add ( make_joint_group(j->child(i),s,_jgroup) );

	return g;
}

//===== virtual methods =====

enum GroupPos { AxisPos=0, SpherePos=1, MatrixPos=2, GeoPos=3 };
enum GeoGroupPos { VisgeoPos=0, ColgeoPos=1, FirstCylPos=2 };

void KnScene::connect ( KnSkeleton* s )
{
	GS_TRACE1 ( "KnScene::connect..." );
	init ();

	if ( !s ) return;
	_skeleton = s;
	_skeleton->ref();

	GsMat arot;
	SnGroup* gaxis, *gsphere;
	SnLines* axis;
	SnPrimitive* sphere;
	SnModel* smodel;
	bool vgeovis=true;

	const GsArray<KnJoint*>& joints = s->joints ();
	_jgroup.size ( joints.size() );

	SnGroup* g = _skelgroup = make_joint_group ( s->root(), s, _jgroup );
	g->separator ( true );
	add ( g ); // group containing per-joint visualization

	if ( _skeleton->skin() ) // skin group
	{	add ( _skinsc=new KnSkinScene(_skeleton->skin()) );
		vgeovis = false;
	}

	float lavg=0, lmin=-1.0, lmax=-1.0f, lf;
	if (joints[0])
	{	for ( int i=0, s=joints.size(); i<s; i++ )
		{	GS_TRACE1 ( "pre processing joint "<<i<<"..." );
			float l = joints[i]->offset().len();
			lavg += l;
			if ( l!=0.0f && (lmin==-1.0f || l<lmin) ) lmin=l;
			if ( l!=0.0f && (lmax==-1.0f || l>lmax) ) lmax=l;
		}
	}

	lavg/=joints.size();
	if ( lmax<0.0f || lmin<0.0f ) { lmax=1.0f; lmin=0.0f; }
	lf = (lmax-lmin) * 0.5f;
	if ( lf==0.0f ) lf = lmax * 0.5f;

	_avgoffsetlen = lf;
	//_avgoffsetlen = 1.0f;
	//_cradius = lmin;

	sphere = new SnPrimitive; // shared sphere
	sphere->prim().sphere ( _cradius * _sfactor * lf * DEF_SPH_OFFSETRATIO );
	sphere->color ( GsColor::gray );

	axis = new SnLines; // shared axis
	axis->push_axis ( GsVec::null, _axislen * lf * DEF_AXIS_OFFSETRATIO, 3, "xyz"/*let*/, false/*rule*/ );

	for ( int i=0, s=joints.size(); i<s; i++ )
	{	GS_TRACE1 ( "processing joint "<<i<<"..." );
		quat2mat ( joints[i]->quat()->prerot(), arot );
		arot.setrans ( joints[i]->offset() );

		// a sphere shows a joint center:
		gsphere = new SnGroup;
		gsphere->separator ( true );
		gsphere->add ( new SnTransform(arot) );
		gsphere->add ( sphere );
		gsphere->visible ( false );

		// axis shows the frame after correction, but before the joint local rotation:
		gaxis = new SnGroup;
		gaxis->separator ( true );
		gaxis->add ( new SnTransform(arot) );
		gaxis->add ( axis );
		gaxis->visible ( false );

		// add all elements:
		_jgroup[i]->add ( gaxis, AxisPos );
		_jgroup[i]->add ( gsphere, SpherePos );
		_jgroup[i]->add ( new SnTransform, MatrixPos );

		g = new SnGroup; // geometry group of the joint
		_jgroup[i]->add ( g, GeoPos );

		smodel = new SnModel(joints[i]->visgeo()); // ok if visgeo is null
		smodel->visible ( vgeovis );
		g->add ( smodel ); // at VisgeoPos

		smodel = new SnModel(joints[i]->colgeo());
		smodel->visible ( false );
		g->add ( smodel ); // at ColgeoPos
	
		for ( int j=0, jc=joints[i]->children(); j<jc; j++ )
		{	SnPrimitive* c = new SnPrimitive;
			c->prim().cylinder ( GsPnt::null, 
								 joints[i]->child(j)->offset(),
								 _cradius * lf * DEF_CYLRAD_OFFSETRATIO );
			c->visible ( false );
			g->add ( c ); // starting at FirstCylPos
		}
	}
   
	update ();
	GS_TRACE1 ( "done." );
}

void KnScene::update ()
{
	if ( !_skeleton ) return;

	if ( _skinsc ) _skinsc->update();

	if ( _skelgroup->visible() )
	{	const GsArray<KnJoint*>& joints = _skeleton->joints ();
		for ( int i=0, s=joints.size(); i<s; i++ ) update ( i );
	}
}

void KnScene::update ( int j )
{
	if ( !_skeleton ) return;
	KnJoint* joint = _skeleton->joints()[j];
	joint->update_lmat();
	((SnTransform*)_jgroup[j]->get(MatrixPos))->set ( joint->lmat() );
}

void KnScene::rebuild ()
{
	if ( !_skeleton ) return;
	const GsArray<KnJoint*>& joints = _skeleton->joints ();
	for ( int i=0; i<joints.size(); i++ ) rebuild ( i );
}

void KnScene::rebuild ( int j )
{
	if ( !_skeleton ) return;
	SnGroup* g;
	KnJoint* joint = _skeleton->joints()[j];
	joint->update_lmat();

	// update framerot:
	GsMat arot;
	quat2mat ( joint->quat()->prerot(), arot );
	arot.setrans ( joint->offset() );
	g = (SnGroup*)_jgroup[j]->get(AxisPos);
	((SnTransform*)g->get(0))->set ( arot );

	g = (SnGroup*)_jgroup[j]->get(SpherePos);
	((SnTransform*)g->get(0))->set ( arot );

	// update matrix:
	((SnTransform*)_jgroup[j]->get(MatrixPos))->set ( joint->lmat() );
   
	// update geometries:
	g = (SnGroup*)_jgroup[j]->get(GeoPos);
	((SnModel*)g->get(VisgeoPos))->changed(true);
	((SnModel*)g->get(ColgeoPos))->changed(true);

	// update links:
	for ( int i=0; i<joint->children(); i++ )
	{	SnPrimitive* c = (SnPrimitive*)g->get(FirstCylPos+i);
		c->prim().cylinder ( GsPnt::null, joint->child(i)->offset(), -1 ); // no change to radius
	}
}

//===== visibility methods =====

void KnScene::set_visibility ( int skin, int skel, int visgeo, int colgeo, int vaxis )
{
	if ( !_skeleton ) return;

	const GsArray<KnJoint*>& joints = _skeleton->joints();
	for ( int i=0, s=joints.size(); i<s; i++ )
	{	set_visibility ( joints[i], skel, visgeo, colgeo, vaxis );
	}

	if ( _skinsc && skin!=-1 )
	{	_skinsc->visible ( skin==1 );
	}

	// check if per-joint info shuld be visible:
	_skelgroup->visible ( !skel && !visgeo && !colgeo && !vaxis? false:true );
}

void KnScene::set_visibility ( KnJoint* joint, int skel, int visgeo, int colgeo, int vaxis )
{
	SnGroup* g;
	SnPrimitive* cyl;
	SnGroup* gaxis, *gsphere;
	SnModel* vismodel;
	SnModel* colmodel;

	int ji = joint->index();
	g = _jgroup[ji];

	if ( skel!=-1 )
	{	gsphere = (SnGroup*) g->get(SpherePos);
		gsphere->visible ( skel==1 );
	}

	if ( vaxis!=-1 )
	{	gaxis = (SnGroup*) g->get(AxisPos);
		gaxis->visible ( vaxis==1 );
	}

	g = (SnGroup*)_jgroup[ji]->get(GeoPos); // the geometry group

	if ( skel!=-1 )
	{	for ( int i=FirstCylPos, s=g->size(); i<s; i++ )
		{	cyl = (SnPrimitive*)g->get(i);
			cyl->visible ( skel==1 );
		}
	}

	if ( visgeo!=-1 )
	{	vismodel = (SnModel*)g->get(VisgeoPos);
		vismodel->visible ( visgeo==1 );
	}

	if ( colgeo!=-1 )
	{	colmodel = (SnModel*)g->get(ColgeoPos);
		colmodel->visible ( colgeo==1 );
	}
}

void KnScene::set_skeleton_joint_color ( KnJoint* joint, const GsColor& color )
{
	SnGroup* g = _jgroup[joint->index()];
	SnPrimitive* sphere = g->get<SnGroup>(SpherePos)->get<SnPrimitive>(1);
	sphere->color(color);
}

void KnScene::set_skeleton_joint_color ( const GsColor& color )
{
	int i;
	const GsArray<KnJoint*>& joints = _skeleton->joints();
	for ( i=0; i<joints.size(); i++ )
		set_skeleton_joint_color ( joints[i], color );
}

void KnScene::set_skeleton_radius ( float r )
{
	SnGroup* g;
	SnPrimitive* cyl;
	SnPrimitive* sphere;

	if ( _cradius==r ) return;
	_cradius = r;
   
	int i, j;
	for ( i=0; i<_jgroup.size(); i++ )
	{	g = _jgroup[i];
		sphere = (SnPrimitive*) ((SnGroup*) g->get(SpherePos))->get(1);
		sphere->prim().ra = _cradius * _sfactor * _avgoffsetlen * DEF_SPH_OFFSETRATIO;

		g = (SnGroup*)_jgroup[i]->get(GeoPos); // the geometry group

		for ( j=FirstCylPos; j<g->size(); j++ )
		{	cyl = (SnPrimitive*) g->get(j);
			cyl->prim().ra = cyl->prim().rb = _cradius * _avgoffsetlen * DEF_CYLRAD_OFFSETRATIO;
		}
	}
}

void KnScene::set_axis_length ( float l )
{
	SnGroup* g;
	SnLines* axis;

	if ( _axislen==l || _skeleton==0 ) return;
	_axislen = l;

	for ( int i=0; i<_jgroup.size(); i++ )
	{	g = _jgroup[i];
		axis = (SnLines*) ((SnGroup*)g->get(AxisPos))->get(1);
		axis->init();
		axis->push_axis ( GsVec::null, _axislen * _avgoffsetlen * DEF_AXIS_OFFSETRATIO, 3, "xyz"/*let*/, false/*rule*/ );
		break; // as the axis is shared, we can update only the first one
	}
}

void KnScene::mark_geometry ( KnJoint* j, GsColor color )
{
	SnGroup* g;
	SnModel* model;
	GsMaterial mtl;
	mtl.diffuse = color;

	g = (SnGroup*)_jgroup[j->index()]->get(GeoPos); // the geometry group

	for ( int k=FirstCylPos; k<g->size(); k++ )
		((SnPrimitive*)g->get(k))->override_material ( mtl );

	model = (SnModel*) g->get(VisgeoPos);
	model->override_material ( mtl );

	model = (SnModel*) g->get(ColgeoPos);
	model->override_material ( mtl );
}

static void sUnmark ( SnGroup* g )
{
	for ( int k=FirstCylPos; k<g->size(); k++ )
	{	((SnPrimitive*)g->get(k))->restore_material (); }

	((SnModel*)g->get(VisgeoPos)) -> restore_material ();
	((SnModel*)g->get(ColgeoPos)) -> restore_material ();
}

void KnScene::unmark_geometry ( KnJoint* j )
{
	sUnmark ( (SnGroup*)_jgroup[j->index()]->get(GeoPos) ); // the geometry group
}

void KnScene::set_geometry_style ( KnJoint* j, GsModel* m, gsRenderMode mode, bool mark, int alpha )
{
	int geo;

	if ( !m ) return;
	else if ( j->visgeo()==m ) geo=VisgeoPos;
	else if ( j->colgeo()==m ) geo=ColgeoPos;
	else return;

	SnGroup* g = (SnGroup*)_jgroup[j->index()]->get(GeoPos); // the geometry group

	SnModel* model = (SnModel*) g->get(geo);

	if ( mode==gsRenderModeDefault )
		model->restore_render_mode ();
	else
		model->override_render_mode ( mode );

	if ( alpha>=0 )
	{	GsModel& m = *model->model();
		for ( int i=0; i<m.M.size(); i++ ) m.M[i].diffuse.a = alpha;
	}

	if ( mark )
	{	GsMaterial mtl;
		mtl.diffuse = GsColor::red;
		if ( alpha>=0 ) mtl.diffuse.a = alpha;
		model->override_material ( mtl );
	}
	else
	{	model->restore_material (); }
}

void KnScene::unmark_all_geometries ()
{
	for ( int i=0; i<_jgroup.size(); i++ )
	{	sUnmark ( (SnGroup*)_jgroup[i]->get(GeoPos) ); }
}

//============================= static ===================================

void KnScene::get_defaults ( float& gsadius, float& alen )
{
	gsadius = DEF_CYL_RADIUS;
	alen = DEF_AXIS_LEN;
}

//============================= misc ===================================

void draw_swing_limits ( KnJoint* j, SnLines* snlines, float r, GsColor color )
{
	if ( !j || !snlines ) return;
	SnLines& l = *snlines;
	j->skeleton()->update_global_matrices();

	GsPnt c ( j->gmat().e14, j->gmat().e24, j->gmat().e34 ); // center
	float ex = j->st()->ellipsex();
	float ey = j->st()->ellipsey();
	GsPnt p, z(0.0f,0.0f,r);
	float ang;
	float inc = GS_TORAD(5.0f);
	GsQuat sw, jrot = j->quat()->prerot();

	if ( j->parent() )
	{	GsQuat q;
		q.set ( j->parent()->gmat() );
		jrot = q * jrot;
	}

	// draw border:
	l.push ( color );
	l.begin_polyline ();
	for ( ang=0; ang<gs2pi; ang+=inc )
	{	st2quat ( cosf(ang)*ex, sinf(ang)*ey, 0, sw );
		l.push ( c+(jrot*sw).apply(z) );
	}
	l.end_polyline ();

	// connect to center:
	inc = GS_TORAD(15.0f);
	for ( ang=0; ang<gs2pi; ang+=inc )
	{	st2quat ( cosf(ang)*ex, sinf(ang)*ey, 0, sw );
		l.push ( c, c+(jrot*sw).apply(z) );
	}

   // draw the spherical ellipse surface:
   /*inc = GS_TORAD(15.0f);
   for ( ang=-gspidiv2; ang<=gspidiv2; ang+=inc )
	{ st2quat ( cosf(ang)*ex, sinf(ang)*ey, 0, sw );
	  GsVec axis = sw.axis();
	  float aang = sw.angle(); if ( aang<0 ) { aang=-aang; axis=-axis; }
	  l.begin_polyline ();
	  float a;
	  for ( a=-aang; a<aang; a+=inc )
	   { sw.set ( axis, a );
		 p = c+(jrot*sw).apply(z);
		 l.push ( p );
	   }
	  l.end_polyline ();
   }*/
}

/* possible extension: to display cylinders according to the 
translation DOF of the joint:
inline GsVec translation ( KnJoint* j, bool tlinks )
 {
   return tlinks? j->offset()+j->pos()->value() : j->offset();
 }

// If true, will draw cylinders based both on the offsets and the translation dofs
void set_translation_links ( bool mode ) { _tlinks=mode; rebuild(); }

- have also to upgrade/fix update method()
*/

//============================= EOF ===================================
