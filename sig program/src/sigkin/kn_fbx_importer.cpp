/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_dirs.h>
# include <sig/gs_image.h>
# include <sig/gs_model.h>
# include <sig/gs_euler.h>

# include <sigkin/kn_fbx_importer.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_posture.h>
# include <sigkin/kn_motion.h>
# include <sigkin/kn_skin.h>

//# define GS_USE_TRACE1 // Geometry
//# define GS_USE_TRACE2 // Models
//# define GS_USE_TRACE3 // Object types
//# define GS_USE_TRACE4 // Connections
//# define GS_USE_TRACE5 // Animation nodes
//# define GS_USE_TRACE6 // Animation curves
//# define GS_USE_TRACE7 // Joint connections
//# define GS_USE_TRACE8 // Materials
//# define GS_USE_TRACE9 // Deformers
# include <sig/gs_trace.h>

//======================================= KnFbxImporter ====================================

KnFbxImporter::KnFbxImporter ()
{
}

KnFbxImporter::~KnFbxImporter ()
{
}

void KnFbxImporter::init ()
{
	// tables:
	M.init ( 256 );
	A.init ( 256 );
	C.init ( 512 );
	G.init ( 32 );
	Mtl.init ( 32 );
	Txt.init ( 32 );
	Def.init ( 128 );

	// arrays:
	J.size ( 0 );
	GSM.init ();

	// extra:
	_fbxfname="";
	_buffer.size(0);
}

static const char* modelname ( const GsString& s, bool simplify )
{
	if ( s.len()==0 ) return "-";
	if ( !simplify ) return s.pt();
	const char* pt = s.pt()+s.len()-1;
	while ( pt!=s.pt() ) { pt--; if ( *pt==':' ) return pt+1; }
	return pt;
}

void KnFbxImporter::read_entry ( GsInput& lin, Entry* e, GsString& idst, bool simpnames )
{
	lin.get(); // ':'
	lin.get(); // number
	if ( lin.ltype()!=GsInput::Number ) gsout.fatal("Unexpected token!");
	idst = lin.ltoken();
	e->id = lin.ltoken().atol();
	lin.get(); // ','
	lin.get();
	e->name = modelname(lin.ltoken(),simpnames); // name
	lin.get(); // ','
	lin.get();
	e->type = lin.ltoken();
}

bool KnFbxImporter::load ( const char* filename, const char* options )
{
	GS_TRACE2("Loading ["<<filename<<"]...");
	GsInput fin;
	if ( !fin.open(filename) ) return false;
	init ();
	_fbxfname = filename;

	// defaut options:
	bool simpnames = false;

	// user options:
	if ( !options ) options="s";
	for ( int i=0; options[i]; i++ )
	{	if ( options[i]=='s' ) simpnames=true;
	}

	// parse file:
	GsString idst, linebuf;
	GsInput lin;
	while (true)
	{	fin.readline(linebuf);
		if ( fin.end() ) break;
		lin.init(linebuf);
		lin.get();
		if ( lin.ltype()!=GsInput::String ) continue;

		const GsString& s = lin.ltoken();

		if ( s=="ObjectType" )
		{	fin.skipto ( "}", GsInput::Delimiter ); // skip section to avoid parsing conflicts
		}
		else if ( s=="Model" )
		{	Model* m = new Model;
			read_entry ( lin, m, idst, simpnames ); // types: Mesh, LimbNode, Null, Camera, CameraSwitcher
			M.insert ( idst, m );
			GS_TRACE2 ( "Model "<<m->id<<" : ["<<m->name<<"] ["<<m->type<<"]..." );
			if ( m->type=="LimbNode" || m->type=="Null" )
			{	J.push()=m; // joints
				read_properties ( fin, m );
			}
			else if ( m->type=="Mesh" )
			{	GEO.push()=m; // geometry
				read_properties ( fin, m );
			}
		}
		else if ( s=="Geometry" )
		{	Geom* g = new Geom;
			read_entry ( lin, g, idst, simpnames ); // type: Mesh, Shape
			G.insert ( idst, g );
			//GS_TRACE1 ( "Geometry "<<g->id<<" : ["<<g->name<<"] ["<<g->type<<"]..." );
			if ( g->type=="Mesh" )
			{	read_mesh ( fin, g );
			}
		}
		else if ( s=="Material" )
		{	Material* m = new Material;
			read_entry ( lin, m, idst, simpnames ); // type: 
			Mtl.insert ( idst, m );
			GS_TRACE8 ( "Material "<<m->id<<" : ["<<m->name<<"] ["<<m->type<<"]..." );
			read_material ( fin, m );
		}
		else if ( s=="Texture" )
		{	Texture* t = new Texture;
			read_entry ( lin, t, idst, simpnames ); // type: 
			Txt.insert ( idst, t );
			GS_TRACE8 ( "Texture "<<t->id<<" : ["<<t->name<<"] ["<<t->type<<"]..." );
			read_texture ( fin, t );
		}
		else if ( s=="Deformer" )
		{	Deformer* d = new Deformer;
			read_entry ( lin, d, idst, simpnames ); // type: Skin, Cluster, BlendShapeChannel
			Def.insert ( idst, d );
			read_deformer ( fin, d );
			GS_TRACE9 ( "Deformer "<<d->id<<" : ["<<d->name<<"] ["<<d->type<<"]..." );
		}
		else if ( s=="ObjectType" )
		{	lin.get(); // ':'
			lin.get();
			GS_TRACE3 ( "ObjectType: [" << lin.ltoken() << "]..." );
		}
		else if ( s=="Connections" )
		{	lin.get(); // ':'
			lin.get(); // '{'
			GS_TRACE4 ( "Connections..." );
			read_connections ( fin );
		}
		else if ( s=="AnimationCurveNode" )
		{	lin.get(); // ':'
			read_anim ( lin, fin );
		}
		else if ( s=="AnimationCurve" ) // only connected to one CurveNode
		{	lin.get(); // ':'
			read_curve ( lin, fin );
		}
		else if ( s=="end"  ) // this is not a fbx keyword - can be used to force stop
		{	break;
		}
	}

	return true;
}

static int _readnumseq ( GsInput& fin, GsArray<double>& A, double mf=1.0 )
{
	A.size(0);
	while ( !fin.end() )
	{	fin.get();
		if ( fin.ltype()==GsInput::Number )
		{	A.push()=fin.ltoken().atod();
			A.top() *= mf;
		}
		else if ( fin.ltoken()[0]=='}' ) break;
	}
	return A.size();
}

static int _read_size ( GsInput& fin )
{
	fin.get(); // ':'
	fin.get(); // '*'
	int s = fin.geti();
	fin.get(); // '{'
	return s;
}

static int _read_numbers ( GsInput& fin, GsArray<double>& A, double mf=1.0 )
{
	int s = _read_size ( fin );
	A.sizecap ( 0, s );
	return _readnumseq ( fin, A, mf );
}

void KnFbxImporter::read_mesh ( GsInput& fin, Geom* g )
{
	// add GsModel entry:
	g->gsm = GSM.size();
	GsModel* m = GSM.push();
	GSMG.push() = g;

	// states:
	//char maptype = 'V'; // V:per vertex, F:per face
	_vi.size(0);
	_ni.size(0);
	_ti.size(0);

	// read fields:
	int level=0;
	while (true)
	{	fin.get();
		if ( fin.end() ) break;
		const GsString& s = fin.ltoken();

		if ( level==0 && s=="Vertices" )
		{	int s = _read_numbers(fin,_buffer);
			m->V.sizecap(0,s/3);
			for ( int i=0; i<s; i+=3 ) m->V.push().set ( _buffer[i], _buffer[i+1], _buffer[i+2] );
		}
		else if ( level==0 && s=="PolygonVertexIndex" )
		{	int s = _read_numbers(fin,_buffer);
			_vi.size(s);
			for ( int i=0; i<s; i++ ) _vi[i] = (int)_buffer[i];
		}
		else if ( s=="MappingInformationType" )
		{	fin.get(); // ':'
			fin.get();
			//maptype = fin.ltoken()=="ByPolygonVertex"? 'V':'F';
		}
		else if ( s=="Normals" )
		{	int s = _read_numbers(fin,_buffer);
			//gsout<<_buffer.size()<<gsnl; gsout.pause();
			m->N.sizecap(0,s/3);
			for ( int i=0; i<s; i+=3 ) m->N.push().set ( _buffer[i], _buffer[i+1], _buffer[i+2] );
		}
		else if ( s=="UV" )
		{	int s = _read_numbers(fin,_buffer);
			m->T.sizecap(0,s/2);
			for ( int i=0; i<s; i+=2 ) m->T.push().set ( _buffer[i], _buffer[i+1] );
		}
		else if ( s=="NormalsIndex" )
		{	int s = _read_numbers(fin,_buffer);
			_ni.size(s);
			for ( int i=0; i<s; i++ ) _ni[i] = (int)_buffer[i];
		}
		else if ( s=="UVIndex" )
		{	int s = _read_numbers(fin,_buffer);
			_ti.size(s);
			for ( int i=0; i<s; i++ ) _ti[i] = (int)_buffer[i];
		}
		else if ( s[0]=='{' )
		{	level++;
		}
		else if ( s[0]=='}' )
		{	if ( --level<0 ) break;
		}
	}

	// convert data to triangles:
	int visize = _vi.size();
	if ( visize==0 ) gsout.fatal("Empty faces?");
	int nisize = _ni.size();
	int tisize = _ti.size();
	if ( nisize && nisize!=visize )  gsout.fatal("Normals size mismatch?");
	if ( tisize && tisize!=visize )  gsout.fatal("Tex coords size mismatch?");

	m->F.sizeres(0,visize/2); // may be converting triangles or quads
	if ( nisize ) m->Fn.sizeres(0,visize/2);
	if ( tisize ) m->Ft.sizeres(0,visize/2);
	for ( int i=0; i<visize; )
	{	int k=i+2;
		while ( true )
		{	m->F.push().set ( _vi[i], _vi[k-1], _vi[k]>=0? _vi[k]:-_vi[k]-1 );
			if ( nisize ) m->Fn.push().set ( _ni[i], _ni[k-1], _ni[k] );
			if ( tisize ) m->Ft.push().set ( _ti[i], _ti[k-1], _ti[k] );
			if ( _vi[k]<0 ) break;
			k++;
		}
		i=k+1;
	}
	# ifdef GS_USE_TRACE1
	m->statistics(gsout);
	# endif
}

void KnFbxImporter::read_properties ( GsInput& fin, Model* m )
{
	// advance to properties:
	do { fin.get(); } while ( !fin.end() && fin.ltoken()!="P" );
	if ( fin.end() ) return;

	// read fields
	while (true)
	{	fin.get();
		if ( fin.end() ) break;

		const GsString& s = fin.ltoken();

		if ( s=="Lcl Translation" )
		{	do { fin.get(); } while ( fin.ltype()!=GsInput::Number );
			m->t.x = fin.ltoken().atof();
			fin.get(); // ','
			fin >> m->t.y;
			fin.get(); // ','
			fin >> m->t.z;
		}
		else if ( s=="Lcl Rotation" )
		{	do { fin.get(); } while ( fin.ltype()!=GsInput::Number );
			m->r.x = fin.ltoken().atof();
			fin.get(); // ','
			fin >> m->r.y;
			fin.get(); // ','
			fin >> m->r.z;
			m->r *= gspi/180.0f; // convert to radians
		}
		else if ( s=='}' )
		{	break;
		}
	}
}

void KnFbxImporter::read_anim ( GsInput& lin, GsInput& fin )
{
	lin.get ();
	Anim* a = A.insert ( lin.ltoken() );
	if ( !a ) return;
	a->id = lin.ltoken().atol(); // id

	lin.get(); // ,
	lin.get(); // AnimCurveNode:T/R
	a->type = lin.ltoken().lchar(); // T or R

	// advance to properties:
	fin.skipto ( "P" );
	if ( fin.end() ) return;

	// read fields
	while (true)
	{	fin.get();
		if ( fin.end() ) break;
		const GsString& s = fin.ltoken();
		if ( s=="d|X" )
		{	// no useful property to read
		}
		else if ( fin.ltype()==GsInput::Number )
		{	// values can be found here
		}
		else if ( s[0]=='}' )
		{	break;
		}
	}
	GS_TRACE5 ( "AnimationCurveNode " << a->id << ": type=" << a->type );
}

void KnFbxImporter::read_curve ( GsInput& lin, GsInput& fin )
{
	lin.get ();
	Curve* c = C.insert ( lin.ltoken() );
	if ( !c ) return;
	c->id = lin.ltoken().atol(); // id

	// read fields
	int level = 0;
	while (true)
	{	fin.get();
		if ( fin.end() ) break;
		const GsString& s = fin.ltoken();

		if ( s=="KeyTime" )
		{	_read_numbers(fin,c->times,1.0E-11);
			//gsout<<c->times<<gsnl; gsout.pause();
		}
		else if ( s=="KeyValueFloat" )
		{	_read_numbers(fin,c->values);
		}
		else if ( s[0]=='{' )
		{	level++;
		}
		else if ( s[0]=='}' )
		{	if ( --level<0 ) break;
		}
	}
	GS_TRACE6 ( "AnimationCurve " << c->id << ": times=" << c->times.size()<<" values=" << c->values.size() );
}

void KnFbxImporter::read_texture ( GsInput& fin, Texture* t )
{
	// special fname read to avoid \x convertions:
	# define GETFNAME(s) while ( fin.readchar()!='"' ); \
		fin.readline(s); while (true) { char c=s.lchar(); s.lchar(0); if(c=='"' ) break; }

	// read fields
	int level = 0;
	while (true)
	{	fin.get();
		if ( fin.end() ) break;
		const GsString& s = fin.ltoken();

		if ( s=="UseMaterial" )
		{	// not considered
		}
		else if ( s=="FileName" )
		{	fin.get(); // ':'
			GETFNAME(t->fname);
			remove_path ( t->fname );
		}
		else if ( s=="RelativeFilename" )
		{	fin.get(); // ':'
			GETFNAME(t->relfname);
		}
		else if ( s[0]=='{' )
		{	level++;
		}
		else if ( s[0]=='}' )
		{	if ( --level<0 ) break;
		}
	}
	GS_TRACE8 ( "Texture files: "<<t->fname<<"] ["<<t->relfname );
}

static void _read_color ( GsInput& fin, GsColor& c, float* sh=0 )
{
	GsVec v;
	int n=0;
	while (n<3)
	{	fin.get();
		if ( fin.end() ) break;
		if ( fin.ltype()==GsInput::Number )
		{	v.e[n++] = fin.ltoken().atof();
			if ( sh ) { *sh=v.e[0]; return; }
		}
	}
	c.set ( v.x, v.y, v.z );
}

void KnFbxImporter::read_material ( GsInput& fin, Material* m )
{
	// read fields
	int level = 0;
	while (true)
	{	fin.get();
		if ( fin.end() ) break;
		const GsString& s = fin.ltoken();

		if ( s=="Ambient" )
		{	_read_color ( fin, m->mtl.ambient );
		}
		else if ( s=="Diffuse" )
		{	_read_color ( fin, m->mtl.diffuse );
		}
		else if ( s=="Specular" )
		{	_read_color ( fin, m->mtl.specular );
		}
		else if ( s=="Emissive" )
		{	_read_color ( fin, m->mtl.specular );
		}
		else if ( s=="Shininess" )
		{	
			GsColor c;
			_read_color ( fin, c, &m->mtl.shininess );
		}
		else if ( s[0]=='{' )
		{	level++;
		}
		else if ( s[0]=='}' )
		{	if ( --level<0 ) break;
		}
	}
	GS_TRACE8 ( "Material: "<<m->mtl );
}

void KnFbxImporter::read_deformer ( GsInput& fin, Deformer* d )
{
	
	// read fields
	int level = 0;
	while (true)
	{	fin.get();
		if ( fin.end() ) break;
		const GsString& s = fin.ltoken();

		if ( s=="Indexes" )
		{	int s = _read_numbers(fin,_buffer);
			d->i.size(s);
			for ( int i=0; i<s; i++ ) d->i[i]=(int)_buffer[i];
		}
		else if ( s=="Weights" )
		{	_read_numbers(fin,d->w);
		}
		else if ( s=="Transform" )
		{	_read_numbers(fin,d->t);
		}
		else if ( s=="TransformLink" )
		{	_read_numbers(fin,d->tl);
		}
		else if ( s=="FullWeights" ) // BlendShapeChannel type only
		{	_read_numbers(fin,d->fw);
		}
		else if ( s[0]=='{' )
		{	level++;
		}
		else if ( s[0]=='}' )
		{	if ( --level<0 ) break;
		}
	}
	//GS_TRACE9 ( "xx"
}

void KnFbxImporter::read_connections ( GsInput& fin )
{
	char c;
	GsString obj1, obj2;
	GsString code, id1, id2;

	while ( !fin.end() )
	{
		do { fin.get(); c=fin.ltoken()[0]; } while ( c!='}' && c!=';' );
		if ( c=='}' ) break;
		if ( c==';' ) fin>>obj1;

		do { fin.get(); c=fin.ltoken()[0]; } while ( c!='}' && c!=',' );
		if ( c=='}' ) break;
		if ( c==',' ) fin>>obj2;

		do { fin.get(); c=fin.ltoken()[0]; } while ( c!='}' && fin.ltoken()!="C" );
		if ( c=='}' ) break;

		fin.get(); // ':'
		fin >> code; // "OO" or "OP"
		fin.get(); // ','
		fin >> id1;
		fin.get(); // ','
		fin >> id2;

		if ( obj1=="Model" && obj2=="Model" )
		{	Model* m = M.lookup(id1);
			Model* p = M.lookup(id2);
			if ( m && p )
			{	GS_TRACE7 ( p->name << " <- " << m->name );
				m->parent = p;
			}
		}
		else if ( obj1=="AnimCurveNode" && obj2=="Model" )
		{
			//id1: curve node just says: 3 rotations, id2: joint
			// put joint name in curvenode struct
		}
		else if ( obj1=="AnimCurve" && obj2=="AnimCurveNode" )
		{
			//will put curve data (id1) in joint of curvenode (id2)
		}
		else if ( obj1=="Texture" && obj2=="Material" )
		{	Texture* txt = Txt.lookup(id1);
			Material* mtl = Mtl.lookup(id2);
			if ( txt && mtl )
			{	fin.get(); // ','
				GsString type; fin>>type;
				GS_TRACE8 ( "Txt-Mtl: " << txt->name << " - " << mtl->name << " Tyype:"<<type );
				if ( type=="DiffuseColor" )
					mtl->difmap = txt;
				else if ( type=="NormalMap" )
					mtl->normap = txt;
				else if ( type=="TransparentColor" )
					mtl->opacmap = txt;
				else if ( type=="SpecularColor" )
					mtl->specmap = txt;
				else if ( type=="ShininessExponent" )
					mtl->glosmap = txt;
			}
		}
		else if ( obj1=="Material" && obj2=="Model" )
		{	Material* mtl = Mtl.lookup(id1);
			Model* mod = M.lookup(id2);
			if ( mtl && mod )
			{	GS_TRACE8 ( "Mtl-Mod: " << mtl->name << " - " << mod->name );
				mod->mtl = mtl;
			}
		}
		else if ( obj1=="Geometry" && obj2=="Model" )
		{	Geom* g = G.lookup(id1);
			Model* m = M.lookup(id2);
			if ( g && m )
			{	GS_TRACE8 ( "Geo-Mod: " << g->name << " - " << m->name );
				m->g = g;
			}
		}
		else if ( obj1=="SubDeformer" && obj2=="Model" )
		{	Deformer* d = Def.lookup(id1);
			Model* m = M.lookup(id2);
			if ( d && m )
			{	GS_TRACE9 ( "SubDef-Mod: " << d->name << " - " << m->name << " (id:"<<(int)(m->g?m->g->gsm:-9)<<')' );
				if ( d->mg ) gsout.fatal("2nd deformer?");
				d->mg = m; // each def i linked to 1 model
				if ( d->mg->g )
				{	d->mg->g->d.push()=d;
				}
			}
		}
		else if ( obj1=="Model" && obj2=="SubDeformer" )
		{	Model* m = M.lookup(id1);
			Deformer* d = Def.lookup(id2);
			if ( m && d )
			{	m->d.push()=d; // joints may be linked to multiple defs
				d->mj = m;
				GS_TRACE9 ( "Mod-SubDef: " << m->name << " - " << d->name << " (n:"<<m->d.size()<<')');
				//Unity supports 4 bones per vertex, to the best of my knowledge.
				//https://forums.autodesk.com/t5/fbx-forum/how-do-you-associate-a-weight-with-a-bone/td-p/4037479
			}
		}
		else
		{	GS_TRACE4 ( "Skipping connection "<<obj1<<" - "<<obj2<<"..." );
			// blend shapes will use: SubDeformer-Deformer, Geometry-SubDeformer
		}
	}
	finalize_models ();
}

void KnFbxImporter::finalize_model ( GsDirs& dirs, int gi, GsModel& m, const Material* mtl )
{
	Texture* dift = mtl->difmap; // only processing here diffuse texture

	GS_TRACE8 ( "Geo "<<gi<< ": " << (t?t->relfname:"-") );

	if ( dift && dift->fname.len()==0 && dift->relfname.len()==0 ) dift=0; // no texture file

	GsString fname;
	//Could define desired mode, but currently using detect_mode() (called later)
	//GsModel::GeoMode geom = GsModel::Faces;
	//GsModel::MtlMode mtlm = GsModel::NoMtl;

	if (dift)
	{
		fname = dift->relfname;
		bool ok = dirs.checkfull ( fname );
		if ( !ok ) { fname=dift->relfname; }

		GsModel::Group* g = m.G.push();
		g->fi = 0;
		g->fn = m.F.size();
		g->mtlname = dift->name;
		g->dmap = new GsModel::Texture(fname);

		m.M.size(1);
		m.M[0]=mtl->mtl;
		//m.set_mode ( GsModel::Faces, GsModel::PerGroupMtl );
	
		m.textured = true;
	}

	if ( !dift )
	{	m.set_one_material ( mtl->mtl );
	}
	m.detect_mode();
	m.compress();
	//m.statistics(gsout);
}

void KnFbxImporter::finalize_models ()
{
	GsDirs d;
	d.basedir_from_filename ( _fbxfname );

	if ( GSM.size()!=GEO.size() ) gsout.fatal("Geo-Mod mismatch!");
	for ( int i=0; i<GEO.size(); i++ )
	{	int mi = GEO[i]->g->gsm;
		if ( mi>=0 )
		{	finalize_model ( d, i, *GSM[mi], GEO[i]->mtl );
		}
	}
}

//================== public: ==================

void KnFbxImporter::add_children ( KnSkeleton* sk, Model* pm, KnJoint* pj )
{
	int i, s=J.size();
	GsQuat q;
	for ( i=0; i<s; i++ )
	{	Model* m=J[i];
		if ( pm==m->parent )
		{	KnJoint* j = sk->add_joint ( KnJoint::TypeQuat, pj, m->name );
			m->j = j;
			if ( !pm ) // this is the root node
			{	j->pos()->thaw();
				j->pos()->value ( m->t.x, m->t.y, m->t.z );
			}
			else
			{	j->offset ( m->t );
			}

			gs_rot ( gsXYZ, q, m->r.x, m->r.y, m->r.z );
			j->rot()->thaw();
			j->rot()->setmode ( KnJointRot::FullMode );
			j->rot ()->value ( q );
			//j->rot()->prerot(q);
			//j->rot ()->value( GsQuat() );
			add_children ( sk, m, j );
			if ( !pm ) return; // there is only one root node
		}
	}
}

bool KnFbxImporter::get_skeleton ( KnSkeleton *sk )
{
	if ( J.empty() ) return false;

	// here we could search for parent nodes in the loaded order
	// as it seems connections come ordered by hierarchy; but to
	// be generic we do a complete recursive search:
	sk->init ();
	add_children ( sk, 0, 0 );

	// Now we check if there are geometry deformers which will
	// define a deformable skin, and if so we build a KnSkin:
	//bool hasskin=false;
	for ( int i=0, s=GSMG.size(); i<s; i++ )
	{	if ( GSMG[i]->d.size()>0 ) // has skin
		{	sk->skin ( new KnSkin );
			get_skin ( sk, sk->skin() );
			break;
		}
	}

	return true;
}

// Transform refers to the global initial position of the node containing the link
// TransformLink refers to global initial position of the link node
void KnFbxImporter::set_influences ( KnSkin* skin, int mid, Deformer* def )
{
	GsModel* gsm = skin->model(mid);
	int vs = gsm->V.size();
	int is = def->i.size();
	for ( int i=0; i<is; i++ )
	{	if ( def->i[i]>=vs ) { gsout.warning("Skipping invalid skin index!"); continue; }
		if ( !def->mj ) { gsout.warning("Skipping skin null joint!"); continue; }
		if ( !def->mj || !def->mj->j ) { gsout.warning("Skipping skin null joint influence!"); continue; }
		GsArray<KnSkin::Influence>& I = skin->influences(mid,def->i[i]);
		GsVec tl ( def->tl[12], def->tl[13], def->tl[14] );
		GsVec t = gsm->V[def->i[i]]-tl;
		if ( def->mj->j && def->w[i]>0 )
			I.push().init ( def->mj->j, (float)def->w[i], t );
	}
}
// https://forums.autodesk.com/t5/fbx-forum/getting-the-local-transformation-matrix-for-the-vertices-of-a/td-p/4190364
// lBoneBindingMatrix.Inverse() * lDstVertex;

bool KnFbxImporter::get_skin ( KnSkeleton *sk, KnSkin *skin )
{
	if ( Def.elements()==0 ) return false;
	skin->init ();
	for ( int i=0, s=GSMG.size(); i<s; i++ )
	{	Geom* g = GSMG[i];
		int ds = g->d.size();
		if ( ds==0 ) continue; // no deformers
		GsModel* gsm = GSM[g->gsm];
		int mid = skin->add_model ( sk, gsm );
		for ( int j=0; j<ds; j++ )
		{	Deformer* def = g->d[j];
			if ( def->i.empty() ) continue;
			set_influences ( skin, mid, def );
		}
	}
	skin->normalize_weights();
	return true;
}

bool KnFbxImporter::get_motion ( KnSkeleton *sk, KnMotion* m )
{
	if ( C.elements()==0 ) return false;

	//check if ( c->values.size()!=K.size() )
	//{	gsout.warning("Animation curve values resized to match number of keytimes!");
	//}
	return true;
}


//================================ EOF =================================================
