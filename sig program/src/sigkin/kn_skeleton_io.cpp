/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_model.h>
# include <sig/gs_dirs.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_posture.h>
# include <sigkin/kn_skin.h>

//# define GS_USE_TRACE1 // keyword tracking
//# define GS_USE_TRACE2 // geometry loading
//# define GS_USE_TRACE3 // load
//# define GS_USE_TRACE4 // save
# include <sig/gs_trace.h>

//============================ load_skeleton ============================

enum KeyWord { ROOT, OFFSET, CENTER, CHANNELS, CHANNEL, JOINT, END, 
			   VISGEO, COLGEO, MODELMAT, MODELROT, PREROT, POSTROT,
			   ALIGN, TYPE, EULER, IK, NOKEYWORD, RESETGEO }; 

static KeyWord read_keyword ( GsInput& in )
{
	KeyWord kw = NOKEYWORD;
   
	if ( in.get()!=GsInput::String ) return kw;

	const GsString& s = in.ltoken();

	if ( s=="root" ) kw=ROOT; // root declaration
	else 
	if ( s=="offset" ) kw=OFFSET; // offset to position joints
	else
	if ( s=="center" ) kw=CENTER; // same as offset, more natural for global coords (not supported now)
	else
	if ( s=="channels" ) kw=CHANNELS; // bvh channels definition
	else
	if ( s=="channel" ) kw=CHANNEL; // bvh channels definition
	else
	if ( s=="joint" ) kw=JOINT; // joint declaration
	else
	if ( s=="modelmat" ) kw=MODELMAT; // matrix to adjust geometries
	else
	if ( s=="modelrot" ) kw=MODELROT; // rotation to adjust geometries
	else
	if ( s=="prerot" || s=="framerot" ) kw=PREROT; // to pre-multiply joint rotations
	else
	if ( s=="postrot" ) kw=POSTROT; // to post-multiply joint rotations
	else
	if ( s=="align" ) kw=ALIGN; // adjusts pre/post to have 1st child with given offset
	else
	if ( s=="visgeo" ) kw=VISGEO; // visualization geometry
	else
	if ( s=="colgeo" ) kw=COLGEO; // collision geometry
	else
	if ( s=="euler" ) kw=EULER; // euler order/type specification
	else
	if ( s=="ik" ) kw=IK; // initialize joint IK
	else
	if ( s=="end" ) kw=END; // default string comparison is case-insensitive
	else
	if ( s=="resetgeo" ) kw=RESETGEO; // default string comparison is case-insensitive

	return kw;
}

// used in bvh format
static bool sReadChannels ( GsInput& in, KnJoint* j, GsArray<KnChannel::Type>& channels )
 {
   channels.size(0);

   if ( in.get()!=GsInput::Number ) return false;
   int c, n = in.ltoken().atoi();

   if ( KnSkeleton::ConvertBvhToQuat )
	{ j->rot_type ( KnJoint::TypeQuat );
	}
   else
	{ j->rot_type ( KnJoint::TypeEuler );
	  j->euler()->type ( KnJointEuler::TypeYXZ );
	}

   while ( n-->0 )
	{ if ( in.get()!=GsInput::String ) return false;
	  const GsString& s = in.ltoken();
	  c = KnChannel::type(s);
	  if ( c<=KnChannel::ZPos )
	   { j->pos()->limits(c,false);  // turn off dof limits
		 channels.push() = (KnChannel::Type)c;
	   }
	  else if ( c<=KnChannel::ZRot )
	   { if ( KnSkeleton::ConvertBvhToQuat )
		  { c = KnChannel::Quat;
			if ( channels.size()>0 ) if (channels.top()==c) c=-1;
		  }
		 else
		  { j->euler()->limits(c-3,false); // turn off dof limits
		  }
		 j->rot()->thaw();
		 if ( c>=0 ) channels.push() = (KnChannel::Type)c;
	   }
	  else
	   { j->rot()->thaw();
		 channels.push() = (KnChannel::Type)c;
	   }
	}		 

   return true;
 }

// only in kn format:
//  XPos <val> [free | <min><max> | lim <min><max>]
//  Quat [axis <x> <y> <z> ang <degrees>] [frozen]
//  Swing [axis <x> <y> ang <degrees>] [lim <xradius> <yradius>]
//  Twist <val> [free | <min><max> | lim <min><max>]
static bool sReadChannel ( GsInput& in, KnJoint* j )
 {
   in.get();
   KnChannel::Type c =  KnChannel::type ( in.ltoken() );
	
   if ( c==KnChannel::Quat )
	{ j->rot_type ( KnJoint::TypeQuat );
	  j->rot()->thaw();
	  bool axis = false;
	  in.get();
	  if ( in.ltoken()=="axis" ) axis=true;
	  in.unget();
	  if ( axis )
	   { GsQuat q; in>>q; j->quat()->value(q);
		 in.get();
		 if ( in.ltoken()=="frozen" )
		  j->quat()->freeze();
		 else
		  in.unget();
	   }
	  return true;
	}

   if ( c==KnChannel::Swing )
	{ j->rot_type ( KnJoint::TypeST );
	  j->rot()->thaw();
	  in.get();
	  if ( in.ltoken()=="axis" )
	   { GsVec2 axis;
		 float ang;
		 in >> axis;
		 in.get(); // "ang"
		 in >> ang;
		 axis.len(GS_TORAD(ang));
		 j->st()->swing ( axis.x, axis.y );
		 in.get();
	   }
	  if ( in.ltoken()=="lim" )
	   { float rx, ry;
		 in >> rx >> ry;
		 j->st()->ellipse ( GS_TORAD(rx), GS_TORAD(ry) );
	   } 
	  else // not a parameter
	   { in.unget(); }
	  return true;
	}

   if ( c==KnChannel::Twist )
	{ j->rot_type ( KnJoint::TypeST );
	  j->rot()->thaw();
	  float tw;
	  in >> tw;
	  j->st()->twist ( tw );
	  in.get();
	  if ( in.ltoken()=="free" )
	   { j->st()->twist_limits(false); }
	  else
	   { if ( in.ltoken()!="lim" ) in.unget();
		 float min, max;
		 in >> min >> max;
		 j->st()->twist_limits ( GS_TORAD(min), GS_TORAD(max) );
	   } 
	  return true;
	}

   // at this point the final option is PosX/Y/Z or RotX/Y/Z

   float lo=0, up=0, val=0;
   bool free;

   in >> val; // first num is the value

   in.get();
   if ( in.ltoken()=="free" )
	{ free = true;
	}
   else
	{ if ( in.ltoken()!="lim" ) in.unget(); // "lim" is optional
	  in >> lo >> up;
	  free = false;
	}

   if ( c<=2 ) // 0:XPos, 1:YPos, or 2:ZPos
	{ if ( free )
	   { j->pos()->limits ( c, false ); }
	  else
	   { j->pos()->limits ( c, true );
		 j->pos()->limits ( c, lo, up );
	   }
	  j->pos()->value ( c, val );
	  return true;
	}

   if ( c<=5 ) // 3:XRot, 4:YRot, 5:ZRot
	{ j->rot_type ( KnJoint::TypeEuler );
	  j->rot()->thaw();
	  int d = c-3;
	  if ( free )
	   { j->euler()->limits ( d, false ); }
	  else
	   { j->euler()->limits ( d, true );
		 j->euler()->limits ( d, GS_TORAD(lo), GS_TORAD(up) );
	   }
	  j->euler()->value ( d, GS_TORAD(val) );
	  return true;
	}

   return false; // should not reach here
 }

static GsModel* sReadModel ( GsInput& in, GsDirs& paths, GsMat* mat, GsModel* theother, GsInput* igeo )
{
	if ( in.get()!=GsInput::String ) return 0;
	GsString fname = in.ltoken();
	GS_TRACE2 ( "sReadModel:"<<fname );

	if ( fname=="primitive" ) // create a primitive without loading from a file
	{	GsPrimitive prim;
		in >> prim;
		GsModel* m = new GsModel;
		m->make_primitive ( prim );
		if ( mat ) m->transform ( *mat );
		m->ref();
		return m;
	}
	else if ( fname=="shared" ) // explicit command to share the other geometry
	{	if ( !theother ) return 0;
		theother->ref();
		return theother;
	}

	// if we are reading a same model again in the same joint, share it:
	if ( theother )
	if ( gs_comparecs(fname,theother->filename)==0 )
	{	theother->ref();
		return theother;
	}

	if ( igeo )
	{	igeo->get(); // newgeo keyword
		igeo->get();
		if ( igeo->ltoken()!=fname ) gsout<<"sReadModel Error in "<<fname<<gsnl;
	}
	else
	{	if ( !paths.checkfull(fname) )
		{	GS_TRACE2 ( "Could not read model." );
			return 0; // file not found
		}
	}

	GsModel* m = new GsModel;

	GS_TRACE2 ( "Loading geometry [" << fname << "]..." );
	bool ok;
	if ( igeo )
	{	ok = m->load ( *igeo ); }
	else
	{	ok = m->load ( fname ); }

	if ( ok )
	{	GS_TRACE2 ( "Loaded model with "<<m->F.size()<<" triangles" );
		if ( mat ) m->transform ( *mat );
		m->ref();
	}
	else
	{	GS_TRACE2 ( "Loading Error!" );
		delete m;
		m=0;
	}

	return m;
}

static void sSetModel ( GsModel*& curm, GsModel* newm )
{
	if ( !newm ) return;
	if ( curm )
	{
		curm->add_model ( *newm );
		newm->unref();
	}
	else
	{	curm = newm;
		curm->ref();
	}
}

// Load joint data and returns -1 if } or error is reached, or kw JOINT, END or NOKEYWORD
int KnSkeleton::_loadjdata ( GsInput& in, KnJoint* j, GsDirs& paths, GsInput* igeo )
{
	KeyWord kw;
	bool hasmat=false;
	GsMat curmat;
	GsArray<KnChannel::Type> channels;

	while (true) // get next items
	{ 
		kw = read_keyword(in);
		GS_TRACE1 ( in.ltoken() );

		if ( kw==OFFSET || kw==CENTER )
		{
			in >> j->_offset;
		}
		else if ( kw==CHANNELS ) // only in bvh format
		{
			if ( !sReadChannels(in,j,channels) ) return -1;
		}
		else if ( kw==EULER )
		{ 
			j->rot_type ( KnJoint::TypeEuler );
			in.get();
			if ( in.ltoken()=="XYZ" )
				j->euler()->type ( KnJointEuler::TypeXYZ );
			else if ( in.ltoken()=="YXZ" )
				j->euler()->type ( KnJointEuler::TypeYXZ );
			else if ( in.ltoken()=="ZY" )
				j->euler()->type ( KnJointEuler::TypeZY );
			else if ( in.ltoken()=="YZX" )
				j->euler()->type ( KnJointEuler::TypeYZX );
		}
		else if ( kw==CHANNEL ) // only in kn format
		{
			if ( !sReadChannel(in,j) ) return -1;
		}
		else if ( kw==MODELMAT )
		{
			in >> curmat;
			hasmat = true;
			if ( j->_visgeo ) j->_visgeo->transform ( curmat );
			if ( j->_colgeo ) j->_colgeo->transform ( curmat );
		}
		else if ( kw==MODELROT )
		{
			GsQuat q;
			in >> q;
			quat2mat ( q, curmat );
			hasmat = true;
			if ( j->_visgeo ) j->_visgeo->rotate ( q );
			if ( j->_colgeo ) j->_colgeo->rotate ( q );
		}
		else if ( kw==PREROT )
		{
			GsQuat q;
			in >> q;
			j->quat()->prerot ( j->quat()->prerot()*q );
		}
		else if ( kw==POSTROT )
		{
			GsQuat q;
			in >> q;
			j->quat()->postrot ( j->quat()->postrot()*q );
		}
		else if ( kw==ALIGN )
		{
			GsVec v;
			GsString type;
			in >> type >> v;
			j->quat()->align ( type=="pre"? KnJointRot::AlignPre :
							   type=="post"? KnJointRot::AlignPost :
							   type=="prepost"? KnJointRot::AlignPrePost :
							   type=="preinv"? KnJointRot::AlignPreInv :
											   KnJointRot::AlignPostInv, v );
		}
		else if ( kw==VISGEO ) // keyword only in hm format
		{
			sSetModel ( j->_visgeo, sReadModel(in,paths,hasmat?&curmat:0,j->_colgeo,igeo) );
		}
		else if ( kw==COLGEO ) // keyword only in hm format
		{
			sSetModel ( j->_colgeo, sReadModel(in,paths,hasmat?&curmat:0,j->_visgeo,igeo) );
		}
		else if ( kw==JOINT )
		{
			return kw;
		}
		else if ( kw==RESETGEO )
		{	if ( j->_visgeo ) { j->_visgeo->unref(); j->_visgeo=0; }
			if ( j->_colgeo ) { j->_colgeo->unref(); j->_colgeo=0; }
		}
		else if ( kw==END )
		{	if ( in.gets()=="Site" ) // bvh "end site" joint
			{	in.unget("EndSite");
				return JOINT;
			}
			else { in.unget(); return kw; }
		}
		else if ( in.ltoken()[0]=='}' ) return -1;
	}
}

static char getname ( GsString& name, GsInput& in )
{
	name = "";
	while ( true ) // we try to cope with names containing spaces
	{	in.get();
		if ( in.end() ) return 0; // unexpected EOF
		if ( in.ltoken()[0]=='{' ) return '{';
		if ( in.ltoken()[0]==':' ) return ':'; // new feature: define explicitly the parent!
		if ( in.ltoken()[0]=='}' ) return 0; // should never enter here
		if ( name.len()>0 ) name << '_';
		name << in.ltoken();
	}
	return 0; // should never enter here
}

// recursivelly load joint definitions
enum KnLoadMode { SkeletonMode, JointDataMode };
KnJoint* KnSkeleton::_loadj ( GsInput& in, KnJoint* p, GsDirs& paths, int mode, GsInput* igeo )
{
	KnJoint* j=0;

	// get joint name
	GsString name;
	char lchar = getname(name,in);
	if ( !lchar ) return 0;

	// check if root is set
	if ( mode==JointDataMode )
	{	if ( !_root ) // add root, so this is a new joint definition
		{	add_joint ( KnJoint::TypeQuat, 0, name );
			if ( lchar==':' ) in.get(); // some protection
		}
		else if ( lchar==':' ) // parent provided, so this is a new joint definition
		{	GsString pname;
			char lchar = getname(pname,in);
			if ( !lchar ) return 0;
			p = joint ( pname );
			if ( !p ) return 0;
			j = add_joint ( KnJoint::TypeQuat, p, name );
		}

		if ( !j ) j = joint ( name );

		if ( !j ) // name not found, skip this joint definitions
		{	do { in.get(); } while ( !in.end() && in.ltoken()[0]!='}' );
		}
		else
		{	j->set_lmat_changed ();
			_loadjdata ( in, j, paths, igeo );
		}
		return j;
	}

	// at this point, a hierarchy is to be loaded:
	j = add_joint ( KnJoint::TypeQuat, p, name );

	int kw=0;
	while ( kw>=0 )
	{	kw = _loadjdata ( in, j, paths, igeo );

		if ( kw==JOINT ) // recurse
		{
			_loadj ( in, j, paths, SkeletonMode, igeo );
		}
	}

	return j;
}

bool KnSkeleton::load ( const char* filename, const char* basedir, bool lookforsd, bool lookforw )
{
	GS_TRACE3("Load from file...");
	GsInput in;

	// load skeleton .s/.bvh or joint .sd file
	if ( !in.open(filename) ) return false;
	if ( !load(in,basedir) ) return false;

	// check if there is a .sd data file to load
	GsString filest(filename);
	remove_extension(filest);
	filest.append(".sd");
	if ( lookforsd && in.open(filest) ) // there is extra joint data to load
	{	if ( !load(in,basedir) )
		{	gsout << "Error loading " << filest << gsnl;
			return false;
		}
	}

	// check if there is a .w data file to load
	remove_extension(filest);
	filest.append(".w");
	if ( lookforw && gs_exists(filest) )
	{	if ( _skin ) _skin->unref();
		_skin = new KnSkin;
		_skin->ref();
//SkinDev: add load/save skin functionality
		//if ( !_skin->init ( this, filest, basedir ) )
		//{	gsout<<"Error loading "<<filest<<gsnl;
		//	_skin->unref();
		//	_skin=0;
		//	return false;
		//}
	}
	return true;
}

bool KnSkeleton::load ( GsInput& in, const char* basedir )
{
	GS_TRACE3("Load from input...");
	GsString path;
	GsStrings ikdefs;
	GsDirs paths;
	GsStrings coldetpairs;
	GsInput* igeo=0;

	float scale=1.0f;
	bool scale_offsets=false;
	bool scale_limits=false;
	bool scale_visgeo=false;
	bool scale_colgeo=false;
	bool globalgeo = false;

	if ( in.filename() ) filename(in.filename());

	in.commentchar ( '#' );
	in.lowercase ( false );

	// check signature, type, etc:
	in.get();
	if ( in.ltoken()=="KnSkeleton" ) // .s or .sd
	{	// ok, .s or .sd format
	}
	else if ( in.ltoken()=="HIERARCHY" ) // .bvh
	{	in.unget();	// .bvh format has no signature
		name ( "bvhskel" ); // .bvh has no name, use a default one
	}
	else // .sk?
	{	// we could return false, but we are now letting it try to parse
		// as the old .sk format does not have a signature
		in.unget();
	}

	// .s or .sd: test if creating new skeleton or merging joint data
	in.get();
	if ( in.ltoken()=="skeldata" || in.ltoken()=="joint_data" )
	{	// merging joint data: do not init current skeleton
	}
	else
	{	in.unget();
		init ();
	}

	// check basedir:
	if ( basedir )
	{	paths.basedir ( basedir );
	}
	else if ( in.filename() )
	{	paths.basedir_from_filename ( in.filename() );
	}

	while (true)
	{	in.get();
		if ( in.end() ) break;
		if ( in.ltype()!=GsInput::String ) continue;

		const GsString& s = in.ltoken();

		if ( s=="path" || s=="add_path" )
		{	if ( in.get()!=GsInput::String ) return false;
			path = in.ltoken();
			GS_TRACE1 ( "adding path [" << path << "]..." );
			paths.push(path);
		}
		else if ( s=="name" || s=="set_name" )
		{	in.get();
			_name = in.ltoken();
		}
		else if ( s=="scale" )
		{	in >> scale;
			scale_offsets = true;
			scale_limits = true;
			scale_visgeo = true;
			scale_colgeo = true;
		}
		else if ( s=="collision_free_pairs" )
		{	_colfreepairs.size(0);
			while ( in.get()==GsInput::String ) coldetpairs.push ( in.ltoken() );
		}
		else if ( s=="posture" )
		{	if ( _channels->size()==0 ) make_channels ();
			KnPosture* post = new KnPosture;
			post->init(this);
			post->ref();
			_postures.push() = post;
			in >> *post;
		}
		else if ( s=="dist_func_joints" )
		{	if ( !_dfjoints ) { _dfjoints = new KnPostureDfJoints; _dfjoints->ref(); }
			_dfjoints->input ( in, this );
			for ( int i=0; i<_postures.size(); i++ ) _postures[i]->dfjoints(_dfjoints);
		}
		else if ( s=="userdata" )
		{	in >> *_userdata;
		}
		else if ( s=="skeleton" || s=="HIERARCHY" )
		{	KeyWord kw = read_keyword(in);
			if ( kw!=ROOT && kw!=JOINT ) return false;
			_loadj ( in, 0, paths, SkeletonMode, igeo );
		}
		else if ( s=="joint" || s=="root" ) // joint list or merge data case
		{	_loadj ( in, 0, paths, JointDataMode, igeo );
		}
		else if ( s=="globalgeo"  )
		{	in.get();
			globalgeo = in.ltoken()=="true";
		}
		else if ( s=="ik"  )
		{	in.get(); ikdefs.push ( in.ltoken() ); // ik type
			in.get(); ikdefs.push ( in.ltoken() ); // joint name
		}
		else if ( s=="geofile" )
		{	in.get();
			GsString fname = in.ltoken();
			if ( paths.checkfull(fname) )
			{	delete igeo; // in case several geofile keywords are there by mistake
				igeo = new GsInput;
				igeo->open ( fname );
			}
		}
		else if ( s=="end"  )
		{	break;
		}
	}

	int i, d;
	float val;
	KnJoint* j;
	KnJoint* j1;
	KnJoint* j2;

	// check scalings
	if ( scale_offsets )
		for ( i=0; i<_joints.size(); i++ )
			_joints[i]->_offset *= scale;

	if ( scale_limits )
	{	for ( i=0; i<_joints.size(); i++ )
		{	j = _joints[i];
			for ( d=0; d<3; d++ )
			{	if ( j->pos()->frozen(d) ) continue;
				val = j->pos()->value(d);
				j->pos()->limits ( d, j->pos()->lolim(d)*scale,	j->pos()->uplim(d)*scale );
				j->pos()->value ( d, val*scale );
			}
		}
	}

	if ( scale_visgeo )
	{	for ( i=0; i<_joints.size(); i++ )
		{	j = _joints[i];
			if(j->visgeo()) j->visgeo()->scale(scale);
		}
	}

	if ( scale_colgeo )
	{	for ( i=0; i<_joints.size(); i++ )
		{	j = _joints[i];
			if(j->colgeo()) j->colgeo()->scale(scale);
		}
	}

	// transform coldet pair names to joint pointers
	int npairs = (coldetpairs.size()/2)*2;
	for ( i=0; i<npairs; i+=2 )
	{	j1 = joint ( coldetpairs[i] );
		j2 = joint ( coldetpairs[i+1] );
		if ( j1 && j2 )
		{	_colfreepairs.push() = j1;
			_colfreepairs.push() = j2;
		}
	}

	// make channels table if derived class agrees
	if ( _skeleton_event(EvMakeChannels) ) make_channels ();

	if ( globalgeo ) set_geo_local();

	// initialize declared IKs
	for ( i=0; i<ikdefs.size(); i+=2 )
	{	KnIk::Type type = gs_compare(ikdefs[i],"LeftArm")==0? KnIk::LeftArm :
						  gs_compare(ikdefs[i],"RightArm")==0? KnIk::RightArm :
						  gs_compare(ikdefs[i],"LeftLeg")==0? KnIk::LeftLeg : KnIk::RightLeg;
		j = joint ( ikdefs[i+1] );
		if ( !j ) continue;
		j->ikinit ( type );
	}
	
	delete igeo;
	compress ();

	GS_TRACE1 ( "DONE." );

	return _root? true:false;
}

//============================ Save ============================

static void outgeo ( GsOutput& out, const char* s, KnJoint* j, const char* geopath,
					 GsOutput* outg )
 {
   // check if we should write a mesh primitive "inline"
   // vis/col primitive meshes are never shared as they should have different resolutions
   if ( !geopath && s[0] )
	{ GsModel* m=0;
	  if ( s[0]=='v' && j->visgeo()->primitive )
	   { m = j->visgeo(); }
	  else if ( s[0]=='c' && j->colgeo()->primitive )
	   { m = j->colgeo(); }
	  if ( m )
	   { out.outm (); out << s << "geo primitive\n";
		 out.incmargin(7);
		 out << *(m->primitive) << gsnl;
		 out.incmargin(-7);
		 return; // processed as primitive
	   }
	}

   GsString name;
   name << j->skeleton()->name() << '_' << j->name();
   if ( s[0] ) name << '_' << s; // s will have "vis" or "col"
   name << ".m";
   name.lower ();

   if ( s[0] ) // s will have "vis" or "col"
	{ out.outm (); out << s << "geo \"" << name << "\"\n";
	}
   else // both visgeo and colgeo are the same:
	{ out.outm (); out << "colgeo \"" << name << "\"\n";
	  out.outm (); out << "visgeo \"" << name << "\"\n";
	}

   GsModel* model = s[0]=='v'? j->visgeo() : j->colgeo();

   if ( outg )
	if ( outg->valid() )
	{
	  *outg << "newgeo \"" << name << "\"\n\n";
	  model->save ( *outg );
	  *outg << "\nend\n\n";
	  geopath = 0;
	}

   if ( geopath ) // if geopath is given, save mesh file as well
	{ GsString fullname(geopath);
	  fullname.append ( name );
	  GsOutput o ( fopen(fullname,"wt") );
	  model->save(o);
	}
 }

// syntax: channel <XPos|YPos|ZPos/XRot|YRot|ZRot> <val> [free | <min><max> | lim <min><max>]
static void outvlim ( GsOutput& out, const char* ctype, KnVecLimits* vl, int d )
 {
   if ( vl->value(d)==0 && vl->frozen(d) ) return;
   
   char clet = 'X'+d;
   if ( ctype[0]=='w' ) clet='T'; // we also handle here Twist channel

   float val = vl->value(d);
   float llim = vl->lolim(d);
   float ulim = vl->uplim(d);

   if ( ctype[0]!='P' ) // handle euler and twist
	{ val=GS_TODEG(val); llim=GS_TODEG(llim); ulim=GS_TODEG(ulim); }
	
   out.outm ();
   out << "channel " << clet << ctype << gspc << val;
   
   if ( vl->limits(d) )
	out << " lim " << llim << gspc << ulim << gsnl;
   else	
	out << " free\n";
 }
 
// syntax: channel Quat [axis <x> <y> <z> ang <degrees>] [frozen]
static void outquat ( GsOutput& out, KnJointRot* q )
 {
   float w = q->value().w;
   if ( q->frozen() && w==1.0f ) return;
   out.outm ();
   out << "channel Quat";
   if ( w!=1.0 ) out << gspc << q->value();
   if ( q->frozen() ) out << " frozen";
   out<<gsnl;
 }
 
// syntax: channel Swing [axis <x> <y> ang <degrees>] [lim <xradius> <yradius>]
//		 channel Twist <val> [free | <min><max> | lim <min><max>]
static void outst ( GsOutput& out, KnJointST* st )
 {
   // output swing:  
   out.outm();
   GsVec2 v ( st->swingx(), st->swingy() );
   out << "channel Swing";
   if ( v!=GsVec::null )
	{ float ang = v.len();
	  v.normalize();
	  out << " axis " << v << " ang " << GS_TODEG(ang);
	}
   // output swing limits:
   v.set ( GS_TODEG(st->ellipsex()), GS_TODEG(st->ellipsey()) );
   if ( v.x<179.999f || v.y<179.999f )  out << " lim " << v;
   out << gsnl;

   // output twist, using the VecLimits class:   
   KnVecLimits vl;
   vl.limits ( 0, st->twist_lolim(), st->twist_uplim() );
   vl.limits ( 0, st->twist_limits() );
   vl.value ( 0, st->twist() );
   outvlim ( out, "wist", &vl, 0 );
 }

static void write_joint ( GsOutput& out, KnJoint* j, int marg, const char* geopath, bool writeall,
						  GsOutput* outg )
 {
   int i;
   out<<gsnl;
   out.margin ( marg );

   out.outm();
   if ( j->parent() ) out << "joint "; else out << "root ";
   out << j->name() << gsnl;

   // write offset :
   out.outm();
   out << "{ offset " << j->offset() << gsnl;
   out.margin ( marg+2 );
   
   if ( writeall )
	{ if ( j->visgeo() && j->visgeo()==j->colgeo() )
	   { // if both geometries are the same, write it only once:
		 outgeo ( out, "", j, geopath, outg );
	   }
	  else
	   { // write visualization geometry if any :
		 if ( j->visgeo() ) outgeo ( out, "vis", j, geopath, outg );
   
		 // write collision geometry if any :
		 if ( j->colgeo() ) outgeo ( out, "col", j, geopath, outg );
	   }
	}

   // write translation channels:  XPos <val> [free | <min><max> | lim <min><max>]
   for ( i=0; i<3; i++ ) outvlim ( out, "Pos", j->pos(), i );

   // write rotation channels:
   switch ( j->rot_type() )
	{ case KnJoint::TypeST : outst ( out, j->st() ); break;
	  case KnJoint::TypeQuat : outquat ( out, j->quat() ); break;
	  case KnJoint::TypeEuler :
		   if ( j->euler()->type()==KnJointEuler::TypeZY )
			{ out.outm(); out << "euler ZY\n"; }
		   else if ( j->euler()->type()==KnJointEuler::TypeXYZ )
			{ out.outm(); out << "euler XYZ\n"; }
		   else if ( j->euler()->type()==KnJointEuler::TypeYZX )
			{ out.outm(); out << "euler YZX\n"; }
		   for ( i=0; i<3; i++ ) outvlim ( out, "Rot", j->euler(), i );
		   break;
	};

   // write frame adjustments if any (must come after joint rotation definition):
   if ( j->quat()->hasprepost() )
   if ( j->quat()->prerot()!=GsQuat::null )
	{ out.outm();
	  out << "prerot " << j->quat()->prerot() << gsnl;
	}
   if ( j->quat()->hasprepost() )
   if ( j->quat()->postrot()!=GsQuat::null )
	{ out.outm();
	  out << "postrot " << j->quat()->postrot() << gsnl;
	}
   
   if ( writeall )
	{ // recurse into children:
	  for ( i=0; i<j->children(); i++ )
	   write_joint ( out, (KnJoint*)j->child(i), marg+2, geopath, writeall, outg );
	}
 
   out.margin ( marg );
   out.outm();
   out << '}' << gsnl;
 }

bool KnSkeleton::save ( const char* filename, bool exportgeo, bool singlegeo, bool extmtls )
 {
   GsOutput out;

   GS_TRACE4 ( "Opening [" << filename << "]...\n" );
   if ( !out.open(filename) ) return false;
   
   GS_TRACE4("Saving...");
   return save(out,exportgeo,singlegeo,extmtls);
 }

bool KnSkeleton::save ( GsOutput& out, bool exportgeo, bool singlegeo, bool extmtls )
 {
   int i;
   GsString s;

   // write header:
   GS_TRACE4("Writing header...");
   out << "# SIG Skeleton Definition\n\n";
   out << "KnSkeleton\n\n";

   if ( !_root ) return false;

   out.margin_char ( gspc );
   
   // write name, if any:
   GS_TRACE4("Writing name...");
   if ( _name.len() ) out << "name " << GsSafeWrite(_name) << gsnl << gsnl;

   // extract geometry path:
   GsOutput* outgeo=0; // only used when singlegeo is true
   GsString geopathst;
   const char* geopath=0;
   if ( exportgeo && out.filename() )
	{ geopathst = out.filename();
	  if ( singlegeo )
	   { remove_extension ( geopathst );
		 geopathst << ".m";
		 outgeo = new GsOutput;
		 outgeo->open ( geopathst );
		 remove_path ( geopathst );
		 out << "geofile \"" <<geopathst<< "\"\n\n"; // here
	   }
	  else
	   { remove_filename ( geopathst );
		 if ( validate_path(geopathst) ) geopath=geopathst; // just to be safe
	   }
	  geopath = geopathst;
	}

   // save shared materials if it is the case:
   if ( extmtls )
	{ GS_TRACE4("Sharing materials...");
	  /*TodoNote
	  GsStrings libnames; GsArray<GsMaterial> libmtls;
	  GsString path ( out.filename() );
	  remove_filename ( path );
	  for ( int i=0; i<_joints.size(); i++ )
	   { KnJoint* j = _joints[i];
		 if ( j->visgeo() ) j->visgeo()->share_materials ( path, libnames, libmtls );
		 if ( j->colgeo() && j->colgeo()!=j->visgeo() ) j->colgeo()->share_materials ( path, libnames, libmtls );
	   }*/
	  GS_TRACE4("End of sharing materials.");
	}

   // write the skeleton:
   GS_TRACE4("Writing joints...");
   out << "skeleton";
   write_joint ( out, _root, 0, geopath, true, outgeo );
   delete outgeo;
   out << gsnl;

   // write dfjoints:
   if ( _dfjoints )
	{ out << "dist_func_joints ";
	  _dfjoints->output ( out );
	}

   // write posture list:
   if ( _postures.size()>0 )
	{ for ( i=0; i<_postures.size(); i++ )
	   { out << "add_posture ";
		 if ( _channels==_postures[i]->channels() )
		  _postures[i]->output(out,false,true);
		 else
		  _postures[i]->output(out,true,true);
	   }
	  out << gsnl;
	}

   // write collision-free pairs list:
   if ( _colfreepairs.size()>0 )
	{ output_coldet_free_pairs ( out );
	}

   // write the (optional) end keyword:
   out << "end\n";

   GS_TRACE4("End writing.");
   return true;
 }

void KnSkeleton::output_coldet_free_pairs ( GsOutput& out )
 {
   out << "collision_free_pairs\n";
   if ( _colfreepairs.size()==0 ) { out<<";\n"; return; }

   int i;
   for ( i=0; i<_colfreepairs.size(); i++ )
	{ out.safewrite ( _colfreepairs[i]->name() );
	  if ( i%2==0 ) out<<gspc;
	   else if ( i+1==_colfreepairs.size() ) out<<";\n\n";
		else out<<gsnl;
	}
 }

// export in joint list format
// and adapt load to do also a merge
bool KnSkeleton::export_joints ( GsOutput& out )
 {
   int i;
   GsString s;

   out.margin_char ( gspc );

   // write header:
   out << "# SIG Toolkit Skeleton Data - Marcelo Kallmann (c) 2014-2017\n";
   out << "# this file is automatically parsed when a .s of same file name is loaded\n\n";

   out << "skeldata\n";

   if ( _root ) // write joints:
	{ for ( i=0; i<_joints.size(); i++ )
	   write_joint ( out, _joints[i], 0, "", false, 0 );
	}

   // write the (optional) end keyword:
   out << "\nend\n";

   return true;
 }

//============================ End of File ============================
