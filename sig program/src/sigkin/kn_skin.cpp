/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>

# include <sig/sn_model.h>
# include <sigkin/kn_skin.h>
# include <sigkin/kn_skeleton.h>
# include <sigkin/kn_joint.h>

//# define GS_USE_TRACE1 // Constr Destr
//# define GS_USE_TRACE2
# include <sig/gs_trace.h>

//============================= KnSkin ============================

KnSkin::KnSkin ()
{
	GS_TRACE1 ( "Constructor" );
	_skeleton = 0;
}

KnSkin::~KnSkin ()
{
	GS_TRACE1 ( "Destructor" );
	init ();
}

void KnSkin::init ()
{
	_infs.init();
	_models.init();
	if ( _skeleton ) { _skeleton->unref(); _skeleton=0; }
}

int KnSkin::add_model ( KnSkeleton* sk, GsModel* m )
{
	if ( _skeleton!=sk ) { updrefs(_skeleton,sk); }
	if ( !m ) m = new GsModel;
	//int vs = m->V.size();
	int ms = _models.size();
	_models.push(m); // m will be referenced by _models array
	_infstart.push() = _infs.size(); // starting point for new influences
	int nis = _infs.size()+m->V.size();
	_infs.size ( nis );
	return ms;
}

//bool KnSkin::load ( KnSkeleton* skel, const char* filename, const char* basedir )
//{
//	init ();
//	GsInput in;
//	GsDirs paths;
//	if ( basedir ) paths.basedir ( basedir );
//	else paths.basedir_from_filename ( filename );
//
//	// check if there is a mesh skin file to load:
//	GsString filest(filename);
//	remove_extension(filest);
//	filest.append(".m");
//	if ( !paths.checkfull(filest) )
//	{	remove_extension(filest);
//		filest.append(".obj");
//		if ( !paths.checkfull(filest) ) return false; // no mesh file found
//	}
//
//	// load skin mesh:
//	if ( !model()->load(filest) ) return false; // error in loading mesh
//
//	// load weights:
//	remove_extension(filest);
//	filest.append(".w");
//	if ( !paths.checkfull(filest) ) return false; // no weights file found
//	if ( !in.open(filename) ) return false; // error opening .w file
//	in.commentchar ( '#' );
//	in.lowercase ( false );
//
//	// check signature, type, etc:
//	in.get();
//	if ( in.ltoken()=="KnSkinWeights" )
//	{	/* ok .w format */ }
//	else
//	{	gsout.warning("Unknown skinning weights file"); return false; }
//
//	GsQuat q;
//	GsMat ml, mi, mv;
//	skel->init_values();
//	skel->update_global_matrices();
//	_skeleton = skel;
//	_skeleton->ref();
//	_intn = model()->V.size()==model()->N.size();
//
//	while (true)
//	{	in.get();
//		if ( in.end() ) break;
//
//		if ( in.ltype()==GsInput::Number )
//		{ 
//			int vid = atoi ( in.ltoken() );
//			if ( vid!=SV.size() ) gsout<<"skin: skin vertex id mismatch\n";
//			int n = in.geti();
//			Weight* w = new Weight[n];
//			SV.push();
//			SV.top().n = n;
//			SV.top().w = w;
//			for ( int i=0; i<n; i++ )
//			{	in.get();
//				if ( in.ltype()==GsInput::Delimiter ) in.get(); // skip delimiter
//				if ( in.ltype()!=GsInput::String ) // missing weight
//				{ in.unget(); w[i].j=0; w[i].w=0; gsout<<"skin: missing weight\n"; break; }
//				w[i].j = skel->joint(in.ltoken());
//				if ( !w[i].j ) gsout<<"skin: unknown joint name: "<<in.ltoken()<<gsnl;
//				w[i].w = in.getf();
//				if ( w[i].j )
//				{	mv.translation ( model()->V[vid] );
//					mi = w[i].j->gmat().inverse();
//					ml.mult ( mv, mi );
//					decompose ( ml, w[i].q, w[i].v );
//					if ( _intn )
//					{	//mi.setrans ( GsVec::null );
//						//w[i].n = model()->N[vid] * mi; // put normal in local coords
//						//mv.translation ( model()->N[vid] );
//						/*mat2quat ( w[i].j->gmat(), q );
//						q = (w[i].q * q);
//						w[i].nq = q.inverse();
//						w[i].n = w[i].nq.apply ( model()->N[vid] );*/
//					} 
//				}
//			}
//		}
//		else if ( in.ltype()==GsInput::String && in.ltoken()=="end"  )
//		{	break;
//		}
//		if ( SV.size()==model()->V.size() ) break;
//	}
//
//   SV.compress();
//   if ( SV.size()!=model()->V.size() ) gsout.warning("Number of skin vertices differs from skinning weights");
//   return true;
//}

void KnSkin::normalize_weights ()
{
	for ( int vi=0, vs=_infs.size(); vi<vs; vi++ ) // for all vertices
	{	GsArray<KnSkin::Influence>& I = *_infs[vi];
		int is=I.size();
		float wsum = 0;
		for ( int k=0; k<is; k++ ) wsum+=I[k].w;
		for ( int k=0; k<is; k++ ) I[k].w /= wsum;
		I.compress();
	}
}

void KnSkin::update ()
{
	if ( !_skeleton ) return;
	_skeleton->update_global_matrices();

	GsPnt p, wp, ni, wn;
	GsQuat q;

	int ms = _models.size();
	for ( int mi=0; mi<ms; mi++ ) // for all models
	{	GsArray<GsPnt>& V = _models[mi]->V;
		int vs = V.size();
		int ci = _infstart[mi];
		for ( int vi=0; vi<vs; vi++ ) // for all vertices
		{	GsArray<KnSkin::Influence>& I = *_infs[ci+vi];
			int is = I.size();
			if ( is==1 ) // optimize for 1 weight case
			{	Influence& i = I[0];
				if ( !i.j ) break; // protection
				V[vi] = (i.j->gmat()*i.v) * i.w;
			}
			else // generic case
			{	wn = GsPnt::null;
				wp = GsPnt::null;
				for ( int k=0; k<is; k++ )
				{	Influence& i = I[k];
					if ( !i.j ) continue; // protection
					wp += (i.j->gmat()*i.v) * i.w;
				}
				V[vi] = wp;
			}
		}
	}
}

// is such a version useful?
	/*! Assumes skeleton is attached and has up to date global matrices and updates
		the skin vertices of skin model index mi. No validity tests on mi are performed. */
//	void update ( int mi );

//============================= EOF ===================================
