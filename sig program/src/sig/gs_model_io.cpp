/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>
# include <iostream>

# include <sig/gs_model.h>
# include <sig/gs_dirs.h>
# include <sig/gs_strings.h>

//# define GS_USE_TRACE1 // IO
# include <sig/gs_trace.h>

//=================================== GsModel =================================================

bool GsModel::load ( const char* fname )
{
	if ( !fname || fname[0]==0 ) return false;
	GsInput in;
	bool ret;

	GsString fn=fname;
	if ( has_extension(fn,"m") )
	{	if ( !in.open(fname) ) return false;
		ret = load ( in );
	}
	else if ( has_extension(fn,"obj") )
	{	ret = load_obj(fname);
	}
	else if ( has_extension(fn,"iv")||has_extension(filename,"wrl") )
	{	ret = load_iv(fname);
	}
	else if ( has_extension(fn,"3ds") )
	{	ret = load_3ds(fname);
	}
	else
	{	if ( !in.open(fname) ) return false;
		ret = load ( in );
	}
	if ( ret ) GsModel::filename.adopt(fn);
	return ret;
}

bool GsModel::load ( GsInput &in )
{
	if ( !in.valid() ) return false;
	in.commentchar ( '#' ); // ensure proper comment style

	// check signature
	in.get();
	GsString s = in.ltoken();
	if ( s=="newgeo" ) { in.get(); in.get(); s=in.ltoken(); } // just skip this entry
	if ( s!="GsModel" ) return false;

	// materials per face and independent names were supported in old format:
	GsArray<int> Fm;
	GsStrings mtlnames;

	// clear arrays and set culling to true
	int i;
	init ();
	GsPrimitive* hasprim=0;
	GsString mtlpath;

	while ( true )
	{ 
		in.get();
		if ( in.ltype()==GsInput::End ) break;
  
		// parse old srm format as well:
		if ( in.ltoken()=="<" )
		{	in.get();
			if ( in.ltoken()=="/" ) { in.get(); in.get(); continue; }
			s=in.ltoken();
			in.get();
		}
		else
		{	s = in.ltoken();
		}
		GS_TRACE1 ( "[" << s << "] keyword found..." );

		if ( s=="end" ) // stop reading data
		{	break;
		}
		else if ( s=="culling" ) // read culling state
		{	in >> i; 
			culling = i? true:false;
		}
		else if ( s=="name" ) // read name (is a GsInput::String type )
		{	in.get();
			name = in.ltoken();
		}
		else if ( s=="vertices" ) // read vertices: x y z
		{	V.size(in.geti());
			for ( i=0; i<V.size(); i++ ) // fscanf call is equiv to: in >> V[i];
			{	if ( fscanf ( in.filept(), "%f %f %f", &V[i].x, &V[i].y, &V[i].z )!=3 ) return false; }
		}
		else if ( s=="faces" ) // read F: a b c
		{	F.size(in.geti());
			for ( i=0; i<F.size(); i++ ) // fscanf call is equiv to: in >> F[i];
			{	if ( fscanf ( in.filept(), "%d %d %d", &F[i].a, &F[i].b, &F[i].c )!=3 ) return false;
				F[i].validate();
			}
		}
		else if ( s=="normals" ) // read N: x y z
		{	N.size(in.geti());
			for ( i=0; i<N.size(); i++ ) // fscanf call is equiv to: in >> N[i];
			{	if ( fscanf ( in.filept(), "%f %f %f", &N[i].x, &N[i].y, &N[i].z )!=3 ) return false; }
		}
		else if ( s=="fnormals" ) // read Fn: a b c
		{	Fn.size(in.geti());
			for ( i=0; i<Fn.size(); i++ ) // fscanf call is equiv to: in >> Fn[i];
			{	if ( fscanf ( in.filept(), "%d %d %d", &Fn[i].a, &Fn[i].b, &Fn[i].c )!=3 ) return false; }
		}
		else if ( s=="ftextcoords" ) // read Ft: a b c
		{	Ft.size(in.geti());
			for ( i=0; i<Ft.size(); i++ ) // fscanf call is equiv to: in >> Ft[i];
			{	if ( fscanf ( in.filept(), "%d %d %d", &Ft[i].a, &Ft[i].b, &Ft[i].c )!=3 ) return false; }
		}
		else if ( s=="fmaterials" ) // read Fm: i
		{	Fm.size(in.geti());
			for ( i=0; i<Fm.size(); i++ ) // fscanf call is equiv to: in >> Fm[i];
			{	if ( fscanf ( in.filept(), "%d", &Fm[i] )!=1 ) return false; }
		}
		else if ( s=="groups" ) // read material groups
		{	int gsize = in.geti();
			for ( i=0; i<gsize; i++ )
			{	G.push ();
				G.top()->fi = in.geti();
				G.top()->fn = in.geti();
				in.get(); 
				if ( in.ltype()==GsInput::String ) G.top()->mtlname = in.ltoken(); // copy operator used
			}
		}
		else if ( s=="textcoords" ) // read T: u v
		{	T.size(in.geti());
			for ( i=0; i<T.size(); i++ ) // fscanf call is equiv to: in >> T[i];
			{	if ( fscanf ( in.filept(), "%f %f", &T[i].x, &T[i].y )!=2 ) return false; }
		}
		else if ( s=="materials" ) // read M: mtls
		{	M.size(in.geti());
			for ( i=0; i<M.size(); i++ ) in >> M[i];
		}
		else if ( s=="mtlnames" ) // read materials
		{	GsString buf1, buf2;
			mtlnames.capacity ( 0 ); // clear all
			mtlnames.size ( M.size() ); // realloc
			while ( true )
			{	if ( in.get()!=GsInput::Number ) { in.unget(); break; }
				i = in.ltoken().atoi();
				in.get();
				mtlnames.set ( i, in.ltoken() );
			}
			if ( mtlnames.size()>0 && in.filename() && has_extension(mtlnames[i],"mt") )
			{	mtlpath=in.filename();
				extract_filename(mtlpath,s);
			}
		}
		else if ( s=="primitive" )
		{	hasprim = new GsPrimitive;
			in >> *(hasprim);
		}
		else if ( s=="vertices_per_face" ) in.unget("faces");
		else if ( s=="normals_per_face" ) in.unget("fnormals");
		else if ( s=="materials_per_face" ) in.unget("fmaterials");
		else if ( s=="material_names" ) in.unget("mtlnames");
		else if ( s=="textures" ) gsout.fatal("GsModel Load: Textures not yet supported!");
	}

	if ( hasprim )
	{	GsString* s = 0;
		if ( V.size()==0 ) // note that make_primitive() calls init()
		{	if ( name.len()>0 ) s = new GsString(name);
			make_primitive ( *hasprim );
			if ( s ) { name = *s; delete s; }
			delete hasprim;
		}
		else
		{	primitive = hasprim; }
	}

	if ( mtlpath.len()>0 )
	{	GsInput minp;
		for ( i=0; i<mtlnames.size(); i++ )
		{	if ( i>=M.size() ) break; // protection
			s = mtlpath;
			s.append ( mtlnames[i] );
			if ( minp.open(s) ) minp >> M[i]; // GsInput will take care of closing the file
			else gsout.warning ( "Could not load shared material [%s]!", s.pt() );
		}
	}

	if ( N.size()>0 && Fn.size()!=F.size() ) Fn=F;
	if ( Fm.size() ) define_groups ( Fm, mtlnames.size()==M.size()? &mtlnames:0 );

	compress ();
	detect_mode ();
	GS_TRACE1 ( "OK.\n" );

	return true;
}

bool GsModel::save ( const char* fname )
{
	filename = fname;

	if ( has_extension(filename,"iv") )
	{	return save_iv(fname);
	}
	else
	{	GsOutput out;
		if ( !out.open(fname) ) return false;
		return save ( out );
	}
}

bool GsModel::save ( GsOutput &o ) const
{
	int i, s;

	// save header as a comment
	o << "# SIG Toolkit\n\n";

	// save signature
	o << "GsModel\n\n";

	// save name
	if ( name.len()>0 )
	{	o << "name ";
		name.safewrite(o);
		o << gsnl << gsnl;
	}

	// save culling state
	if ( !culling )
	{	o << "culling 0\n\n"; }

	// save primitive shape information
	if ( primitive )
	{	o << "primitive\n" << *primitive << gsnl; }

	// save vertices (V)
	if ( V.size() ) 
	{	o << "vertices " << V.size() << gsnl;
		for ( i=0; i<V.size(); i++ ) o << V[i] << gsnl;
		o << gsnl;
	}

	// save faces (F)
	if ( F.size() )
	{	o << "faces " << F.size() << gsnl;
		for ( i=0; i<F.size(); i++ ) o << F[i] << gsnl;
		o << gsnl;
	}

	// save normals (N)
	if ( N.size() )
	{	o << "normals " << N.size() << gsnl;
		for ( i=0; i<N.size(); i++ ) o << N[i] << gsnl;
		o << gsnl;
	}

	// save normals per face (Fn)
	if ( Fn.size() )
	{	o << "fnormals " << Fn.size() << gsnl;
		for ( i=0; i<Fn.size(); i++ ) o << Fn[i] << gsnl;
		o << gsnl;
	}

	// save textures ids per face (Ft)
	if ( Ft.size() )
	{	o << "ftextcoords " << Ft.size() << gsnl;
		for ( i=0; i<Ft.size(); i++ ) o << Ft[i] << gsnl;
		o << gsnl;
	}

	// save texture coordinates (T)
	if ( T.size() )
	{	o << "textcoords " << T.size() << gsnl;
		for ( i=0; i<T.size(); i++ ) o << T[i] << gsnl;
		o << gsnl;
	}

	// save materials (M)
	if ( M.size() )
	{	o << "materials " << M.size() << gsnl;
		for ( i=0; i<M.size(); i++ ) o << M[i] << gsnl;
		o << gsnl;
	}

	// save material groups
	if ( G.size() )
	{	o << "groups " << G.size() << gsnl; 
		for ( i=0, s=G.size(); i<s; i++ )
		{	Group& g = *G[i];
			o << g.fi << gspc << g.fn << (g.mtlname? g.mtlname:";") << gsnl;
		}
		o << gsnl;
	}

	// done.
	return true;
}

//================================ End of File =================================================
