/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_strings.h>
# include <sig/gs_string.h>
# include <sig/gs_model.h>
# include <sig/gs_dirs.h>

//# define GS_USE_TRACE1	// keyword tracking
//# define GS_USE_TRACE2	// trace specific keywords
//# define GS_USE_TRACE3	// new material
//# define GS_USE_TRACE4	// final stats
# include <sig/gs_trace.h>

# define GETID(n,A) in>>n; if (n>0) n--; else if (n<0) n+=A.size()

static void get_face ( GsInput& in, GsModel& m, int& vc, int& vt, int& vn )
{
	vc = vt = vn = -1;
	GETID(vc,m.V);

	if ( in.check()==GsInput::Delimiter ) // if not had only: vc
	{	in.get(); // get /
		if ( in.check()==GsInput::Number ) // get vt from: vc/vt or vc/vt/vn
		{	GETID(vt,m.T);
		}
		if ( in.check()==GsInput::Delimiter ) // get vn from: vc/vt/vn or vc//vn
		{	in.get(); // get /
			GETID(vn,m.N);
		}
	}
}

static GsColor read_color ( GsInput& in )
{
	float r, g, b;
	in >> r >> g >> b;
	GsColor c(r,g,b);
	return c;
}

/* .mtl materials:
	 Ka: ambient color (r,g,b)
	 Kd: diffuse color (r,g,b)
	 Ks: specular color (r,g,b)
  illum: illumination model: 1 a flat material with no specular, 2 has specular highlights
	 Ns: shininess of the material
d or Tr: the transparency of the material
 map_Ka: file containing a texture map */
static void read_materials ( GsModel& model,
							 GsArray<GsMaterial>& M,
							 GsArray<GsModel::Texture*>& Td,
							 GsStrings& mnames,
							 const GsString& file,
							 const GsStrings& paths )
{
	GsString fullf;
	GsInput in;
	in.lowercase(false);

	for ( int i=0; i<=paths.size(); i++ )
	{	fullf = i<paths.size()? paths[i] : "";
		fullf << file;
		GS_TRACE3 ( "Opening material file ["<<fullf<<"]..." );
		in.init ( fopen(fullf,"rt") );
		if ( in.valid() ) break;
	}
	if ( !in.valid() ) { GS_TRACE3 ( "Could not open!"); return; } // could not get materials
	in.commentchar ('#');

	while ( !in.end() )
	{	in.get();
		if ( in.ltoken()=="newmtl" )
		{	M.push().init();
			Td.push()=0;
			in.get();
			GS_TRACE3 ( "new material: "<<in.ltoken() );
			mnames.push ( in.ltoken() );
		}
		else if ( in.ltoken()=="Ka" )
		{	M.top().ambient = read_color ( in );
		}
		else if ( in.ltoken()=="Kd" )
		{	M.top().diffuse = read_color ( in );
		}
		else if ( in.ltoken()=="Ks" )
		{	M.top().specular = read_color ( in );
		}
		else if ( in.ltoken()=="Ke" ) // not sure if this one exists
		{	M.top().emission = read_color ( in );
		}
		else if ( in.ltoken()=="Ns" )
		{	in >> M.top().shininess;
		}
		else if ( in.ltoken()=="d" || in.ltoken()=="Tr" )
		{	int a = int(in.getf()*255.0f);
			//if ( in.ltoken()[0]=='T' ) a=255-a;
			M.top().diffuse.a = (gsbyte)(GS_BOUND(a,0,255));
		}
		else if ( in.ltoken()=="illum" )
		{	int i = in.geti();
			if ( i==1 ) M.top().specular = GsColor::black;
		}
		else if ( in.ltoken()=="map_Kd" || in.ltoken()=="map_Ka" ) // diffuse texture
		{	GsString txfile;
			in.readline(txfile);
			txfile.trim();
			GsModel::Texture* tx = new GsModel::Texture;
			tx->id=-2;
			tx->fname.set ( txfile );
			Td.top() = tx;
			model.textured = 1;
		}
		else // keyword not supported
		{	in.skipline();
		}
	}
}

static bool process_line ( GsInput& in, GsModel& m, GsArray<int>& Fm,
						   GsStrings& paths, GsStrings& mnames, int& curmtl,
						   GsArray<int>& va, GsArray<int>& ta, GsArray<int>& na,
						   GsArray<GsModel::Texture*>& Td )
{
	in.get();

	GS_TRACE1 ( "Processing: ["<<in.ltoken()<<"] (len="<<in.ltoken().len()<<")" );

	if ( in.ltoken().len()==0 ) return true;

	if ( in.ltoken()=="v" ) // v x y z [w]
	{	GS_TRACE1 ( "v" );
		m.V.push();
		in >> m.V.top();
	}
	else if ( in.ltoken()=="vn" ) // vn i j k
	{	GS_TRACE1 ( "vn" );
		m.N.push();
		in >> m.N.top();
	}
	else if ( in.ltoken()=="vt" ) // vt u v [w]
	{	GS_TRACE1 ( "vt" );
		m.T.push();
		in >> m.T.top();
	}
	else if ( in.ltoken()=="f" ) // f v/t/n v/t/n v/t/n (or v/t or v//n or v)
	{	GS_TRACE1 ( "f" );
		int i=0;
		va.size(0); ta.size(0); na.size(0);
		while ( true )
		{	if ( in.get()==GsInput::End ) break;
			in.unget();
			get_face ( in, m, va.push(), ta.push(), na.push() );
		}
		if ( va.size()<3 ) return false;
		for ( i=2; i<va.size(); i++ ) // triangulate
		{	m.F.push().set ( va[0], va[i-1], va[i] );
			Fm.push() = curmtl;
			if ( ta[0]>=0 && ta[1]>=0 && ta[i]>=0 )
				m.Ft.push().set ( ta[0], ta[i-1], ta[i] );
			if ( na[0]>=0 && na[1]>=0 && na[i]>=0 )
				m.Fn.push().set ( na[0], na[i-1], na[i] );
		}
	}
	else if ( in.ltoken()=="s" ) // smoothing groups not loaded
	{	GS_TRACE1 ( "s" );
		in.get();
		GS_TRACE2 ( "s: "<<in.ltoken() );
	}
	else if ( in.ltoken()=="o" ) // object name
	{	GS_TRACE1 ( "o" );
		in.get();
		m.name = in.ltoken();
	}
	else if ( in.ltoken()=="usemap" ) // usemap name/off
	{	GS_TRACE1 ( "usemap" );
	}
	else if ( in.ltoken()=="usemtl" || in.ltoken()=="g" ) // usemtl name
	{	GS_TRACE1 ( "usemtl" );
		in.get ();
		if ( in.ltype()!=GsInput::End )
		{	int i = mnames.lsearch ( in.ltoken() );
			if ( i>=0 ) curmtl=i;
		}
		GS_TRACE1 ( "curmtl = " << curmtl << " (" << in.ltoken() << ")" );
	}
	else if ( in.ltoken()=="mtllib" ) // mtllib file1 file2 ...
	{	GS_TRACE1 ( "mtllib" );
		GsString token, file;
		while ( in.check()==GsInput::String )
		{	in.readline(token);
			token.trim();
			extract_filename ( token, file );
			paths.push ( token );
			GS_TRACE1 ( "new path: "<<paths.top() );
			read_materials ( m, m.M, Td, mnames, file, paths );
		}
	}

	return true;
}

bool GsModel::load_obj ( const char* file )
{
	GsInput in ( fopen(file,"r") );
	if ( !in.valid() ) return false;

	in.commentchar ( '#' );
	in.lowercase ( false );

	GsString path=file;
	GsString fname;
	extract_filename(path,fname);
	GsStrings paths;
	paths.push ( path );
	GS_TRACE1 ( "First path:" << path );
	int curmtl = -1;

	init ();
	name = fname;
	remove_extension ( name );

	GsString line;
	GsStrings mtlnames;
	GsArray<int> Fm; // materials per face
	GsArray<int> v(0,8), t(0,8), n(0,8); // buffers
	GsArray<GsModel::Texture*> Td; // diffuse textures per material

	GsInput lineinp;
	lineinp.commentchar('#');
	while ( in.readline(line)>=0 )
	{	lineinp.init ( line );
		if ( !process_line(lineinp,*this,Fm,paths,mtlnames,curmtl,v,t,n,Td) ) return false;
	}

	define_groups ( Fm, &mtlnames );
	for ( int i=0; i<G.size(); i++) G[i]->dmap = Td[i]; 

	order_transparent_materials ();
	validate();
	compress ();

	#ifdef GS_USE_TRACE4
	statistics ( gsout );
	#endif

	GS_TRACE4("Ok!");

	return true;
}

//============================ EOF ===============================
