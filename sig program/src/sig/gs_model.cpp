/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>
# include <iostream>

# include <sig/gs_model.h>
# include <sig/gs_quat.h>
# include <sig/gs_dirs.h>
# include <sig/gs_table.h>
# include <sig/gs_strings.h>

//# define GS_USE_TRACE1 // groups
//# define GS_USE_TRACE2 // Validation of normals materials, etc
//# define GS_USE_TRACE3 // validate
//# define GS_USE_TRACE4 // add_model
//# define GS_USE_TRACE5 // smooth
//# define GS_USE_TRACE6 // smooth more info
//# define GS_USE_TRACE7 // normals per face
# include <sig/gs_trace.h>

//========================== GsModel::Group ======================================

void GsModel::Group::copyfrom ( const Group& g )
{
	fi = g.fi;
	fn = g.fn;
	mtlname = g.mtlname;
	if ( !g.dmap )
	{	delete dmap; dmap=0; }
	else
	{	if ( !dmap ) dmap = new Texture;
		*dmap = *g.dmap; // copy operator
	}
};

//=================================== GsModel =================================================

void GsModel::construct() // private
{
	primitive = 0;
	culling = 1;
	textured = 0;
	_geomode = Empty;
	_mtlmode = NoMtl;
}

GsModel::~GsModel ()
{
	init ();
}

static void Error ( const char* s ) { gsout.fatal ( "GsModel: %s!", s ); }

void GsModel::set_mode ( GeoMode g, MtlMode m )
{
	_geomode=g;
	_mtlmode=m;

	if ( g==Empty ) return;
	if ( V.empty() || F.empty() ) Error("V or F Empty!");
	if ( g==Faces ) {}
	else if ( g==Flat && N.size()!=F.size() ) Error("Not Flat!");
	else if ( g==Smooth && N.size()!=V.size() ) Error("Not Smooth!");
	else if ( g==Hybrid && (F.size()!=Fn.size()&&N.size()!=V.size()) ) Error("Not Hybrid!");

	if ( m==NoMtl ) return;
	if ( M.empty() ) Error("M empty!");
	if ( m==PerGroupMtl )
	{	if ( M.size()<G.size() ) Error("Invalid G size!");
	}
	else if ( m==PerVertexMtl )
	{	if ( M.size()<V.size() ) Error("Invalid M size!");
		if ( N.size()<V.size() ) Error("Invalid N size!");
	}
	else if ( m==PerVertexColor )
	{	if ( M.size()<V.size() ) Error("Invalid M size!");
	}
	else if ( m==PerFaceMtl )
	{	if ( M.size()<F.size() ) Error("Invalid M size!");
	}
}

void GsModel::detect_mode ()
{
	_geomode=Empty;
	_mtlmode=NoMtl;

	if ( F.empty() || V.empty() ) return;

	// arrays sizes do not uniquely determine modes, the order of the tests determines priorities
	if ( N.empty() ) { _geomode=Faces; }
	else if ( Fn.size()==F.size() ) { _geomode=Hybrid; }
	else if ( N.size()==V.size() ) { _geomode=Smooth; }
	else if ( N.size()==F.size() ) { _geomode=Flat; }
	else Error ( "Invalid GeoMode" );

	if ( M.empty() ) { _mtlmode=NoMtl; }
	else if ( M.size()==G.size() ) { _mtlmode=PerGroupMtl; }
	else if ( M.size()==V.size() ) { _mtlmode=PerVertexMtl; }
	else if ( M.size()==F.size() ) { _mtlmode=PerFaceMtl; }
	else Error ( "Invalid MtlMode" );

	if ( _mtlmode!=PerGroupMtl && G.size()>0 ) Error ( "Invalid G.size()!" );
}

void GsModel::init ()
{
	M.capacity ( 0 );
	V.capacity ( 0 );
	N.capacity ( 0 );
	T.capacity ( 0 );
	F.capacity ( 0 );
	Fn.capacity ( 0 );
	Ft.capacity ( 0 );

	clear_groups ();

	name.set(0);
	filename.set(0);

	delete primitive;
	primitive=0;

	culling = 1;
	_geomode = Empty;
	_mtlmode = NoMtl;
}

void GsModel::statistics ( GsOutput& o, char sep )
{
	o << "GsModel ["<<name<<"]: "<<sep;
	o << "V size:"<<V.size()<<" N size:"<<N.size()<<sep;
	o << "F size:"<<F.size()<<" Fn size:"<<Fn.size()<<sep;
	o << "T size:"<<T.size()<<" Ft size:"<<Ft.size()<<sep;
	o << "M size:"<<M.size()<<" G size:"<<G.size()<<sep;
	o << "Modes: GM="<<_geomode<<" MM="<<_mtlmode<<sep;
}

void GsModel::operator = ( const GsModel& m )
{
	M = m.M;
	V = m.V;
	N = m.N;
	T = m.T;
	F = m.F;
	Fn = m.Fn;
	Ft = m.Fn;
	G = m.G;

	name = m.name;
	filename = m.filename;

	culling = m.culling;
	_geomode = m._geomode;
	_mtlmode = m._mtlmode;

	if ( primitive ) { delete primitive; primitive=0; }
	if ( m.primitive )
	{	primitive = new GsPrimitive;
		*primitive = *m.primitive;
	}
}

void GsModel::compress ()
{
	M.compress();
	V.compress();
	N.compress();
	T.compress();
	F.compress();
	Fn.compress();
	Ft.compress();
	G.compress();
	name.compress();
	filename.compress();
}

void GsModel::validate ()
{
	GsArray<int> iarray;
	GsStrings sarray;

	int fsize = F.size();
	int vsize = V.size();
	int gsize = G.size();

	// check if the model is empty:
	if ( fsize==0 || vsize==0 )
	{	GS_TRACE3("No Faces!");
		init ();
		compress ();
		return;
	}

	// check size of Fn:
	if ( N.size()==0 )
	{	Fn.size(0);
	}
	else if ( Fn.size()!=fsize )
	{	Fn.size(0);
		GS_TRACE3("Fn mismatch!");
		if ( N.size()!=fsize && N.size()!=vsize )
		{	N.size(0);
			GS_TRACE3("N mismatch!");
		}
	}

	// check validity of T:
	if ( T.size()>0 && Ft.empty() )
	{	T.size(0);
		GS_TRACE3("Texture coords mismatch!");
	}

	// check groups:
	if ( gsize )
	{	if ( gsize!=M.size() )
		{	clear_groups();
			GS_TRACE3("Group mismatch!");
		}
		else
		{	for ( int i=0; i<gsize; i++ )
			{	if ( G[i]->fi+G[i]->fn>fsize )
				{	G[i]->fn = fsize-G[i]->fi;
					GS_TRACE3("Group range fixed!");
				}
			}
		}
	}

	// check M size:
	if ( M.size()!=fsize && M.size()!=vsize && M.size()!=G.size() )
	{	M.size ( 0 );
		GS_TRACE3("Materials mismatch!");
	}

	detect_mode ();
	GS_TRACE3("Validate finished.");
}

void GsModel::add_model ( const GsModel& m )
{
	if ( _mtlmode!=NoMtl && _mtlmode!=PerGroupMtl && m.mtlmode()!=NoMtl && m.mtlmode()!=PerGroupMtl )
		gsout.fatal("incompatible modes add_model()!");

	// data: M, V, N, T, F, Fn, Ft, G
	int origm = M.size();
	int origv = V.size();
	int orign = N.size();
	int origt = T.size();
	int origf = F.size();
	int origfn = Fn.size(); // not used
	int origft = Ft.size();
	int origg = G.size();

	int i;
	int mfsize = m.F.size();
	int mvsize = m.V.size();
	int mnsize = m.N.size();
	GS_TRACE4 ( "add_model: init" );

	if ( mvsize==0 || mfsize==0 ) return;
	if ( origv==0 || origf==0 ) { init(); *this=m; return; }

	// add vertices and faces:
	GS_TRACE4 ( "add_model: geometry" );
	V.size ( origv+mvsize );
	for ( i=0; i<mvsize; i++ ) V[origv+i] = m.V.get(i);
	F.size ( origf+mfsize );
	for ( i=0; i<mfsize; i++ )
	{	const Face& f = m.F.get(i);
		F[origf+i].set ( f.a+origv, f.b+origv, f.c+origv );
	}

	// add the normals:
	GS_TRACE4 ( "add_model: normals" );
	if ( mnsize>0 )
	{	int mfnsize = m.Fn.size();
		if ( orign==0 && origfn==0 ) // normals will follow format of m
		{	if ( mfnsize>0 )
			{	smooth(); orign=N.size(); origfn=Fn.size(); }
			else
			{	smooth(-1.0f); orign=N.size(); }
		}
		N.size ( orign+mnsize );
		for ( i=0; i<mnsize; i++ ) N[orign+i] = m.N.get(i);

		if ( mfnsize>0 )
		{	Fn.size ( origfn+mfnsize );
			for ( i=0; i<mfnsize; i++ )
			{	const Face& f = m.Fn.get(i);
				Fn[origfn+i].set ( f.a+origfn, f.b+origfn, f.c+origfn );
			}
		}
	}

	// add the materials:
	GS_TRACE4 ( "add_model: materials" );
	if ( m.M.size()>0 )
	{	M.size ( origm+m.M.size() );
		for ( i=0; i<m.M.size(); i++ ) M[origm+i] = m.M(i);
	}

	// add the groups:
	GS_TRACE4 ( "add_model: groups" );
	if ( m.G.size()>0 )
	{	G.size ( origg+m.G.size() );
		for ( i=0; i<m.G.size(); i++ )
		{	G[origg+i]->copyfrom ( *m.G[i] );
			G[origg+i]->fi+=origf;
		}
	}

	// add text coords:
	GS_TRACE4 ( "add_model: text coords" );
	if ( m.Ft.size()>0 )
	{	Ft.size ( origft+m.Ft.size() );
		for ( i=0; i<m.Ft.size(); i++ ) Ft[origft+i] = m.Ft(i);
	}

	// add texture coordinates
	if ( m.T.size()>0 )
	{	T.size ( origt+m.T.size() );
		for ( i=0; i<m.T.size(); i++ ) V[origt+i] = m.T(i);
	}

	// the model will for sure not be a primitive anymore
	if ( primitive )
	{	delete primitive; primitive=0; }

	GS_TRACE4 ( "add_model: ok." );
}

void GsModel::set_one_material ( const GsMaterial& m, const char* name, bool comp )
{
	clear_groups();
	M.size(1);
	M[0] = m;
	G.push ( new Group( 0, F.size() ) );
	if ( name ) G.top()->mtlname = name;
	if ( comp ) compress ();
	_mtlmode = PerGroupMtl;
}

void GsModel::clear_materials ()
{
	M.size (0);
	clear_groups ();
	compress ();
}

void GsModel::clear_texture_arrays ()
{
	T.size(0);
	T.compress();
	Ft.size(0);
	Ft.compress();
}

void GsModel::clear_groups ()
{
	G.init();
	G.compress();
	if ( M.empty() ) _mtlmode=NoMtl;
}

void GsModel::define_groups ( const GsArray<int>& Fm, const GsStrings* mtlnames )
{
	GS_TRACE1 ( "Defining groups ("<<M.size()<<","<<Fm.size()<<")");
	clear_groups();
	if ( M.empty() && Fm.empty() ) return;

	GS_TRACE1 ( "Organizing faces...");
	// First organize faces per material:
	bool allinvalid = true;
	GsArray<int>* FA = new GsArray<int>[M.size()];
	for ( int f=0; f<Fm.size(); f++ )
	{	int m = Fm[f];
		if ( m<0 || m>=M.size() ) { Fm[f]=0; continue; } // wrong index
		if ( FA[m].empty() ) FA[m].capacity(64);
		FA[m].push()=f;
		allinvalid = false;
	}
	if ( allinvalid ) { delete[] FA; return; }

	// Now create groups, skiping non-used materials:
	GsArray<Face> nF(0,F.size()); //!< Triangular faces indices to V
	GsArray<Face> nFn(0,Fn.size()); //!< Indices to the normals in N (size can be<F.size())
	GsArray<Face> nFt(0,Ft.size()); //!< Indices to the texture coords in T (size can be<F.size())
	GsArray<GsMaterial> nM(0,M.size());

	GS_TRACE1 ( "Adding groups...");
	for ( int m=0; m<M.size(); m++ )
	{	GsArray<int>& A = FA[m];
		GS_TRACE1 ( "References to material "<<m<<": "<<A.size());
		if ( A.empty() ) continue;
		nM.push()=M[m];
		G.push ( new Group ( nF.size(), A.size() ) );
		if (mtlnames) G.top()->mtlname.set ( mtlnames->get(0) ); // copy operator used
		GS_TRACE1 ( "Reindexing group "<<(G.size()-1)<<"...");
		for ( int i=0; i<A.size(); i++ )
		{	nF.push()=F[A[i]];
			if ( Fn.size() ) nFn.push()=Fn[A[i]];
			if ( Ft.size() ) nFt.push()=Ft[A[i]];
		}
	}
	GS_TRACE1 ( "Adopting new arrays...");
	G.compress();
	M.adopt ( nM );
	F.adopt ( nF );
	if ( Fn.size() ) Fn.adopt ( nFn );
	if ( Ft.size() ) Ft.adopt ( nFt );

	delete[] FA;
	GS_TRACE1 ( "Done.");
}

void GsModel::order_transparent_materials ()
{
	if ( M.empty() || G.empty() ) return;
	GS_TRACE1 ( "Ordering transparent materials...");
	GsMaterial tmpm;
	int minj; gsbyte mina;
	int gend = G.size();
	while ( gend>1 )
	{	mina=255; minj=-1;
		for ( int j=0; j<gend; j++ ) 
		{	if ( M[j].diffuse.a<mina ) { mina=M[j].diffuse.a; minj=j; } }
		GS_TRACE1 ( "Index found: "<<minj );
		if ( minj<0 ) break; // no transparent material found
		culling = 0; // turn culling off for transparent objects
		gend--;
		GS_SWAPT(M[minj],M[gend],tmpm);
		G.swap(minj,gend);
	}
	GS_TRACE1 ( "Done.");
	// Note: for correct results when rendering from a given point of view V, the transparent
	// faces would need to be sorted and rendered from back to front with respect to V
}

void GsModel::remove_redundant_normals ( float prec )
 {
   if ( Fn.empty() ) return;

   int fsize = F.size();
   int nsize = N.size();

   if ( nsize==0 || Fn.size()!=fsize )
	{ N.size(0);
	  Fn.size(0);
	  _mtlmode = NoMtl;
	}
   else if ( nsize==1 )
	{ // nothing to test, only 1 normal
	}
   else
	{ // remove references to duplicated normals
	  int i, j, k;
	  GsArray<int> iarray;
	  prec = prec*prec;
	  iarray.size ( nsize );
	  iarray.setall ( -1 );
	  for ( i=0; i<nsize; i++ ) 
	   { for ( j=i+1; j<nsize; j++ ) 
		  { if ( iarray[j]>=0 ) continue;
			if ( dist2(N[i],N[j])<prec )
			 { GS_TRACE2 ( "Detected normal "<<i<<" close to "<<j );
			   iarray[j]=i;
			   for ( k=0; k<fsize; k++ ) // replace references to j by i
				{ if ( Fn[k].a==j ) Fn[k].a=i;
				  if ( Fn[k].b==j ) Fn[k].b=i;
				  if ( Fn[k].c==j ) Fn[k].c=i;
				}
			 }
		  }
	   }

	  int toadd = 0;
	  for ( i=0; i<iarray.size(); i++ ) 
	   { if ( iarray[i]>=0 )
		  { GS_TRACE2 ( "Detected unused normal "<<i );
			toadd++;
			iarray[i]=-1; // unused mark is now -1
		  }
		 else
		  iarray[i] = toadd;
	   }

	  for ( i=0; i<fsize; i++ ) // update indices
	   { Fn[i].a -= iarray[Fn[i].a];
		 Fn[i].b -= iarray[Fn[i].b];
		 Fn[i].c -= iarray[Fn[i].c];
	   }

	  for ( i=0,j=0; i<iarray.size(); i++ ) // compress N
	   { if ( iarray[i]<0 )
		  { N.remove(j); }
		 else
		  { j++; }
	   }
	}
 }

void GsModel::merge_redundant_vertices ( float prec )
 {
   prec = prec*prec;
   
   int fsize = F.size();
   int vsize = V.size();
   int i, j;

   // build iarray marking replacements:
   GsArray<int> iarray;
   iarray.size ( vsize );
   for ( i=0; i<vsize; i++ ) iarray[i]=i;
   
   for ( i=0; i<vsize; i++ )
	for ( j=0; j<vsize; j++ )
	 { if ( i==j ) break; // keep i < j
	   if ( dist2(V[i],V[j])<prec ) // equal
		{ iarray[j]=i;
		}
	 }

   // fix face indices:
   for ( i=0; i<fsize; i++ )
	{ F[i].a = iarray[ F[i].a ];
	  F[i].b = iarray[ F[i].b ];
	  F[i].c = iarray[ F[i].c ];
	}

   // compress indices:   
   int ind=0;
   bool newv;
   for ( i=0; i<vsize; i++ )
	{ newv = iarray[i]==i;
	  V[ind] = V[i];
	  iarray[i] = ind;
	  if ( newv ) ind++;
	}
   V.size ( ind );

   // fix face indices again:
   for ( i=0; i<fsize; i++ )
	{ F[i].a = iarray[ F[i].a ];
	  F[i].b = iarray[ F[i].b ];
	  F[i].c = iarray[ F[i].c ];
	}
 }

void GsModel::flat ( bool comp )
{
	N.size(0);
	Fn.size(0);
	if ( comp ) compress ();
	_geomode = F.empty()||V.empty()? Empty:Faces;
}

// This is 3 times faster than with global sorting method, could still try hash tables
void GsModel::smooth ( float crease_angle )
{
	GS_TRACE5("Starting smooth...");
	if ( F.empty() ) return;
	int i, vsize=V.size();

	// Get array of edges:
	GsArray<Face>* va = get_edges_per_vertex();
	GsArray<GsVec> na; // flat normals per face

	// Compute flat normals for all faces:
	na.size ( F.size() );
	for ( i=0; i<F.size(); i++ ) na[i] = face_normal ( i );

	// First pass interpolates face normals around each vertex:
	GS_TRACE5("1st pass...");
	Fn.size(0);
	N.size ( vsize );
	for ( i=0; i<vsize; i++ )
	{	GsArray<Face>& ea = va[i];
		GsVec nsum (GsVec::null);
		for ( int j=0; j<ea.size(); j++ )
			nsum += na[ea[j].a]; // ImprNote: normals could be weighted by face areas
		N[i] = nsum / (float)ea.size();
		N[i].normalize();
	}

	// second pass will solve crease angles:
	if ( crease_angle>0 )
	{	GS_TRACE5("2nd pass...");
		GsArray<float> ang;
		// Initialize Fn with the smooth normals per vertex:
		Fn.size(F.size());
		for ( i=0; i<Fn.size(); i++ ) Fn[i]=F[i];
		// Make normals per vertex:
		for ( i=0; i<vsize; i++ )
		{	GsArray<Face>& ea = va[i];
			// build angle array and search for a "crease angled edge":
			int j, ini=-1, size=ea.size();
			ang.size(size);
			for ( j=0; j<size; j++ )
			{	ang[j] = ::angle( na[ea[j].a], na[ea[(j+1)%size].a] );
				if ( ini<0 && ang[j]>crease_angle ) ini=j+1;
			}
			// smooth groups of normals starting from creased angle:
			if ( ini>=0 )
			{	GS_TRACE6("Vertex "<<i<<'/'<<vsize<<": crangleind="<<ini<<'/'<<size);
				int ei, est, ncount=0, esize=size+ini; // so that we can start from any index
				GsVec nsum = GsVec::null;
				est=ini;
				for ( ei=ini; ei<esize; ei++ )
				{	j = ei%size;
					nsum += na[ea[j].a];
					ncount++;
					if ( ang[j]>crease_angle ) // add normal and re-start
					{	for ( int k=est; k<=ei; k++ )
						{	Face& fn = Fn[ea[k%size].a];
							if ( i==fn.a ) fn.a = N.size();
							else if ( i==fn.b ) fn.b = N.size();
							else fn.c = N.size();
						}
						N.push() = nsum / (float)ncount;
						N.top().normalize();
						nsum = GsVec::null;
						ncount=0; est=ei+1;
					}
				}
			}
		}
	}

	if ( N.size()==vsize ) // result is one normal per vertex
	{	GS_TRACE5("Smooth normals set per vertex.");
		Fn.size(0);
		_geomode = Smooth;
	}
	else // normals per face per vertex
	{	GS_TRACE5("Optimizing normals...");
		_geomode = Hybrid;
		remove_redundant_normals ();
	}

	// Finalize:
	delete[] va;
	compress ();
	GS_TRACE5("Done.");
}

float GsModel::count_mean_vertex_degree ()
{
	int i;
	if ( F.empty() ) return 0.0f;

	GsArray<int> vi(V.size());
	vi.setall(0);

	for ( i=0; i<F.size(); i++ )
	{	vi[F[i].a]++; vi[F[i].b]++; vi[F[i].c]++; }

	double k=0;
	for ( i=0; i<vi.size(); i++ ) k += double(vi[i]);
	return float( k/double(vi.size()) );
}

int GsModel::common_vertices_of_faces ( int i1, int i2 )
{
	int i, j, c=0;
	int *f1 = &(F[i1].a);
	int *f2 = &(F[i2].a);
	for ( i=0; i<3; i++ )
	{	for ( j=0; j<3; j++ )
		{	if ( f1[i]==f2[j] ) c++;
		}
	}
	return c;
}

void GsModel::get_bounding_box ( GsBox &box ) const
{
	box.set_empty ();
	if ( primitive ) { primitive->get_bounding_box ( box ); return; } // faster via primitive
	if ( V.empty() ) return;
	int i, s=V.size();
	for ( i=0; i<s; i++ ) box.extend ( V[i] );
}

static int fcompare ( const GsModel::Face *f1, const GsModel::Face *f2 )
{
	return f1->b-f2->b;
}

GsArray<GsModel::Face>* GsModel::get_edges_per_vertex()
{
	if ( F.empty() ) return 0;
	int i, j, k;
	int vsize = V.size();
	int fsize = F.size();

	// Allocate array per vertex:
	GsArray<Face>* va = new GsArray<Face>[V.size()];

	// Get slight improvements by reducing re-allocations per vertex:
	for ( i=0; i<vsize; i++ ) va[i].capacity(8);

	// Add unique edges per vertex, respecting orientation, and with face info:
	for ( i=0; i<fsize; i++ )
	{	const Face& f=F[i];
		va[f.a].uniqinsort ( Face(i,f.b,f.c), fcompare );
		va[f.b].uniqinsort ( Face(i,f.c,f.a), fcompare );
		va[f.c].uniqinsort ( Face(i,f.a,f.b), fcompare );
	}

	// Sort edges per vertex by adjacency:
	for ( i=0; i<vsize; i++ )
	{	GsArray<Face>& ea = va[i];
		int max = ea.size()-1;
		for ( j=1; j<max; j++ )
		{	for ( k=j; k<=max; k++ )
			{	if ( ea[j-1].c==ea[k].b ) break; }
			if ( k<=max ) // found
			{	Face tmp; GS_SWAP(ea[j],ea[k]); }
			else 
			{	// It does happen to not find correct adjancency when loading external meshes
			}
		}
	}

	// Done
	return va;
}

GsArray<int>* GsModel::get_edges()
{
	if ( F.empty() ) return 0;

	// The approach here is to ensure uniqueness by sorting the edges per vertex,
	// this is faster than global tree sorting after about 700 vertices.
	// Allocate array per vertex:
	GsArray<int>* va = new GsArray<int>[V.size()];
	// Revise to use: GsArrayPt<GsArray<int>> va passed as referenced argument

	// Get slight improvements by reducing re-allocations per vertex:
	int i, s;
	for ( i=0, s=V.size(); i<s; i++ ) va[i].reserve(8);

	// Add unique edges per vertex:
	// Note: tests indicated uniqpush() about 2x faster than uniqinsort()
	int min, max;
	for ( i=0, s=F.size(); i<s; i++ )
	{	const Face& f=F[i];
		GS_MIN_MAX(f.a,f.b,min,max); va[min].uniqpush ( max, gs_compare );
		GS_MIN_MAX(f.b,f.c,min,max); va[min].uniqpush ( max, gs_compare );
		GS_MIN_MAX(f.c,f.a,min,max); va[min].uniqpush ( max, gs_compare );
	}

	// Done
	return va;
}

void GsModel::get_edges ( GsArray<int>& E )
{
	E.size(0);
	if ( F.empty() ) return;

	// Get array of edges:
	GsArray<int>* va = get_edges();

	// Put result in linear array E:
	E.reserve ( F.size()+V.size()-2 ); // E=F+V-2 estimation if a polyhedron
	for ( int i=0, is=V.size(); i<is; i++ )
	{	for ( int j=0, js=va[i].size(); j<js; j++ )
		{	E.push() = i;
			E.push() = va[i][j];
		}
	}

	// delete va array:
	delete[] va;
}

int GsModel::pick_face ( const GsLine& line ) const
{
	float t, u, v;
	int closestf=-1;
	float closestt;

	for ( int i=0; i<F.size(); i++ )
	{	const Face& f = F[i];
		if ( line.intersects_triangle ( V[f.a], V[f.b], V[f.c], t, u, v ) )
		{	if ( closestf<0 || t<closestt ) { closestf=i; closestt=t; }
		}
	}
   return closestf;
}

void GsModel::normalize ( float maxcoord )
{
	GsVec p; GsBox box;

	get_bounding_box(box);

	p = box.center() * -1.0;
	translate ( p );

	box+=p;
	GS_SETPOS(box.a.x); GS_SETPOS(box.a.y); GS_SETPOS(box.a.z);
	GS_SETPOS(box.b.x); GS_SETPOS(box.b.y); GS_SETPOS(box.b.z);

	p.x = GS_MAX(box.a.x,box.b.x);
	p.y = GS_MAX(box.a.y,box.b.y);
	p.z = GS_MAX(box.a.z,box.b.z);

	float maxactual = GS_MAX3(p.x,p.y,p.z);

	scale ( maxcoord/maxactual );  // Now we normalize to get the desired radius
}

void GsModel::get_vertices_per_face ( GsArray<GsVec>& fv ) const
{
	int n=0, fs=F.size();
	fv.size ( fs*3 );
	for ( int f=0; f<fs; f++ ) 
	{	fv[n++] = V[F[f].a];
		fv[n++] = V[F[f].b];
		fv[n++] = V[F[f].c];
	}
}

void GsModel::get_texcoords_per_face ( GsArray<GsVec2>& ftc ) const
{
	int n=0, s=Ft.size();
	if ( s>0 ) // tx coordinates defined with Ft
	{	ftc.size ( s*3 );
		for ( int f=0; f<s; f++ ) 
		{	ftc[n++] = T[Ft[f].a];
			ftc[n++] = T[Ft[f].b];
			ftc[n++] = T[Ft[f].c];
		}
	}
	else if ( T.size()==V.size() ) // tx coordinates given per vertex
	{	s = F.size();
		ftc.size ( s*3 );
		for ( int f=0; f<s; f++ ) 
		{	ftc[n++] = T[F[f].a];
			ftc[n++] = T[F[f].b];
			ftc[n++] = T[F[f].c];
		}
	}
	else
	{	ftc.size(0); // no coordinates correctly defined
	}
}

void GsModel::get_materials_per_face ( GsArray<int>& fm ) const
{
	fm.size ( F.size() );
	fm.setall ( 0 ); 
	for ( int g=0, gs=G.size(); g<gs; g++ )
	{	for ( int f=0, fs=G[g]->fn; f<fs; f++ )
		{	fm[ G[g]->fi+f ] = g; }
	}
}

void GsModel::get_normals_per_face ( GsArray<GsVec>& fn ) const
{
	int f, n=0, fs=F.size();
	fn.size ( fs*3 );
	if ( _geomode==Hybrid && Fn.size()==fs ) // mixed normals
	{	GS_TRACE7("Retrieving HYBRID face normals...");
		for ( f=0; f<fs; f++ )
		{	fn[n++] = N[Fn[f].a];
			fn[n++] = N[Fn[f].b];
			fn[n++] = N[Fn[f].c];
		}
	}
	else if ( (_geomode==Smooth||_geomode==Hybrid) && N.size()==V.size() ) // smooth normals
	{	GS_TRACE7("Retrieving SMOOTH face normals...");
		for ( f=0; f<fs; f++ )
		{	fn[n++] = N[F[f].a];
			fn[n++] = N[F[f].b];
			fn[n++] = N[F[f].c];
		}
	}
	else if ( _geomode==Flat && N.size()==F.size() ) // normals per face
	{	GS_TRACE7("Retrieving FLAT face normals...");
		for ( f=0; f<fs; f++ )
		{	fn[n++] = N[f];
			fn[n++] = N[f];
			fn[n++] = N[f];
		}
	}
	else // flat normals
	{	GS_TRACE7("Computing FLAT face normals...");
		get_flat_normals_per_face(fn,3);
	}
}

void GsModel::get_flat_normals_per_face ( GsArray<GsVec>& fn, int repspernormal ) const
{ 
	int fs=F.size();
	fn.sizeres ( 0, fs*repspernormal );
	GsVec normal;
	for ( int f=0; f<fs; f++ ) 
	{	normal = face_normal(f);
		for ( int r=0; r<repspernormal; r++ ) 
			fn.push() = normal;
	}
}

GsVec GsModel::face_normal ( int f ) const
{
	GsVec n; 
	const Face& fac = F[f];
	n.cross ( V[fac.b]-V[fac.a], V[fac.c]-V[fac.a] ); 
	n.normalize(); 
	return n; 
}

GsVec GsModel::face_center ( int f ) const
{ 
	GsVec c; 
	const Face& fac = F[f];
	return ( V[fac.a] + V[fac.b] + V[fac.c] ) / 3.0f;
}

void GsModel::invert_faces ()
{
	int i, s, tmp;
	for ( i=0, s=F.size(); i<s; i++ ) GS_SWAP ( F[i].b, F[i].c );
	for ( i=0, s=Fn.size(); i<s; i++ ) GS_SWAP ( Fn[i].b, Fn[i].c );
	for ( i=0, s=Ft.size(); i<s; i++ ) GS_SWAP ( Ft[i].b, Ft[i].c );
}

void GsModel::invert_normals ()
{
	for ( int i=0, s=N.size(); i<s; i++ ) N[i]*=-1.0;
}

void GsModel::translate ( const GsVec &tr )
{
	int i, s=V.size();
	for ( i=0; i<s; i++ ) V[i]+=tr;
	if ( primitive ) { primitive->center += tr; }
}

void GsModel::scale ( float factor )
{
	int i, s=V.size();
	for ( i=0; i<s; i++ ) V[i]*=factor;
	if ( primitive ) { primitive->ra*=factor; primitive->rb*=factor; primitive->rc*=factor; }
}

void GsModel::centralize ()
{
	GsBox box;
	get_bounding_box(box);
	GsVec v = box.center() * -1.0;
	translate ( v );
}

void GsModel::transform ( const GsMat& mat, bool primtransf )
{
	int i, size;

	if ( primtransf )
	{	GsQuat q(mat);
		GsVec t(mat.e14,mat.e24,mat.e34);
		rotate ( q );
		translate ( t );
		return;
	}

	size = V.size();
	for ( i=0; i<size; i++ ) V[i] = mat * V[i];

	size = N.size();
	if ( size )
	{	GsMat m = mat;
		m.setrans ( 0, 0, 0 ); // remove translation before applying to N
		for ( i=0; i<size; i++ )
		{	N[i]= m*N[i]; N[i].normalize(); }  // MatChange: affects here
	}

	// will no longer be a primitive:
	if ( primitive ) { delete primitive; primitive=0; }
}
 
void GsModel::rotate ( const GsQuat& q )
{
	int i, s;
	for ( i=0, s=V.size(); i<s; i++ ) V[i] = q.apply(V[i]);
	for ( i=0, s=N.size(); i<s; i++ ) N[i] = q.apply(N[i]);

	if ( primitive ) { primitive->orientation = q * primitive->orientation; }
}

//================================ End of File =================================================
