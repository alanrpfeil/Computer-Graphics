/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/sn_lines.h>
# include <sig/sn_model.h>
# include <sig/sn_group.h>
# include <sig/sn_shape.h>
# include <sig/sn_manipulator.h>

# include "mv_scene.h"

# define GS_USE_TRACE1  // event tracing
# include <sig/gs_trace.h>

MvScene::MvScene () 
{
	_lastselfac = _lastselmtl = -1;
}

void MvScene::add_model ( SnModel* model )
{
	SnManipulator* manip = new SnManipulator;
	SnGroup* group = new SnGroup;
	SnLines* lines = new SnLines;

	lines->visible ( false );
	manip->visible ( false );

	group->add ( model );
	group->add ( lines );
	manip->child ( group );
	add ( manip );
}

static void Check ( int i, int s )
{
	if ( i<0 || i>=s ) gsout.fatal("Invalid index when accessing scene item!");
}

void MvScene::remove_model ( int i )
{
	Check(i,size());
	remove ( i );
}

SnManipulator* MvScene::manipulator ( int i )
{
	Check(i,size());
	return (SnManipulator*)get(i);
}

SnGroup* MvScene::group ( int i )
{
	return (SnGroup*)manipulator(i)->child();
}

SnModel* MvScene::smodel ( int i )
{
	return (SnModel*)group(i)->get(0);
}

GsModel* MvScene::model ( int i )
{
	return smodel(i)->model();
}

void MvScene::reset_normals ( int i )
{
	SnGroup* g = group(i);
	SnLines* slines = (SnLines*)g->get(1);
	slines->init ();
	if ( slines->visible() ) make_normals (i);
}

void MvScene::make_normals ( int i )
{
	SnGroup* g = group(i);
	const GsModel& m = *model(i);
	SnLines& l = *g->get<SnLines>(1);

	l.init ();
	if ( m.geomode()==GsModel::Empty ) return;

	// compute appropriate length:
	GsPnt center;
	float nlen=0;
	for ( int f=0, s=m.F.size(); f<s; f++ )
	{	center=m.face_center(f);
		nlen += dist2(center,m.V[m.F[f].a] );
		nlen += dist2(center,m.V[m.F[f].b] );
		nlen += dist2(center,m.V[m.F[f].c] );
	}
	nlen = sqrtf(nlen/(m.F.size()*3)); // get total average

	# define ADDNORMAL(c,n) center=c; l.push(center,center+(n*nlen))
	l.push ( GsColor::red );

	if ( m.geomode()==GsModel::Faces ) // there are no normals in N
	{	for ( int f=0, s=m.F.size(); f<s; f++ )
		{	ADDNORMAL ( m.face_center(f), m.face_normal(f) );
		}
	}
	else if ( m.geomode()==GsModel::Flat ) // N.size()==F.size()
	{	for ( int f=0, s=m.F.size(); f<s; f++ )
		{	ADDNORMAL ( m.face_center(f), m.N[f] );
		}
	}
	else if ( m.geomode()==GsModel::Smooth ) // N.size()==V.size()
	{	for ( int n=0, s=m.N.size(); n<s; n++ )
		{	ADDNORMAL ( m.V[n], m.N[n] );
		}
	}
	else if ( m.geomode()==GsModel::Hybrid ) // Fn.size()==F.size()
	{	for ( int f=0, s=m.F.size(); f<s; f++ )
		{	ADDNORMAL ( m.V[m.F[f].a], m.N[m.Fn[f].a] );
			ADDNORMAL ( m.V[m.F[f].b], m.N[m.Fn[f].b] );
			ADDNORMAL ( m.V[m.F[f].c], m.N[m.Fn[f].c] );
		}
	}
	else
	{	gsout.fatal("Unknown model geomode()!"); }

	# undef ADDNORMAL
}

void MvScene::get_visibility ( int i, bool& model, bool& normals, bool& manip, bool& manbox )
{
	SnManipulator* smanip = manipulator(i);
	SnGroup* group = (SnGroup*)smanip->child();
	SnModel* smodel = (SnModel*)group->get(0);
	SnLines* slines = (SnLines*)group->get(1);
	model = smodel->visible();
	normals = slines->visible();
	manip = smanip->visible();
	manbox = smanip->draw_box();
	//gsout<<smodel->shape().name<<": "<<model<<gspc<<normals<<gspc<<manip<<gspc<<manbox<<gsnl;
}

void MvScene::set_visibility ( int i, int model, int normals, int manip, int manbox )
{
	SnManipulator* smanip = manipulator(i);
	SnGroup* group = (SnGroup*)smanip->child();
	SnModel* smodel = (SnModel*)group->get(0);
	SnLines* slines = (SnLines*)group->get(1);
	if ( model>=0 ) { smodel->visible(model!=0); }
	if ( normals>=0 ) { slines->visible(normals!=0); if ( normals&&slines->empty() ) make_normals(i); }
	if ( manip>=0 ) { smanip->visible(manip!=0); smanip->update(); }
	if ( manbox>=0 ) { smanip->draw_box(manbox!=0); smanip->update(); }
}

void MvScene::set_scene_visibility ( int model, int normals, int manips, int manbox )
{
	for ( int i=0; i<size(); i++ )
		set_visibility ( i, model, normals, manips, manbox );
}

void MvScene::set_group_visibility ( int model, int normals, int manbox )
{
	for ( int i=0; i<size(); i++ )
	{	if ( manipulator(i)->visible() ) set_visibility(i,model,normals,-1,manbox);
	}
}

void MvScene::get_group_visibility ( int& model, int& normals, int& manbox )
{
	model = normals = manbox = -1; // initially mark as undefined
	bool firstone=true;
	bool mo, no, ma, mb;
	for ( int i=0, s=size(); i<s; i++ )
	{	if ( manipulator(i)->visible() )
		{	get_visibility(i,mo,no,ma,mb);
			if ( firstone ) // get first one
			{	model=mo; normals=no; manbox=mb; firstone=false; }
			else // update
			{	if ( model>=0 ) { if (model!=(int)mo) model=-1; }
				if ( normals>=0 ) { if (normals!=(int)no) normals=-1; }
				if ( manbox>=0 ) { if (manbox!=(int)mb) manbox=-1; }
				if ( model<0 && normals<0 && manbox<0 ) return; // undefined for all
			}
		}
	}
}

//============================= EOF ===================================
