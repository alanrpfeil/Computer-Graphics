/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_FBX_IMPORTER_H
# define KN_FBX_IMPORTER_H

# include <sig/gs_vars.h>
# include <sig/gs_table.h>
# include <sig/gs_string.h>

class KnSkeleton;
class KnMotion;
class KnJoint;
class KnSkin;

//================================ KnFbxImporter ===================================

/*! A fbx loader */
class KnFbxImporter : public GsShareable
{  protected :
	struct Entry { gsuint id; GsString name, type; Entry(){id=0;} };
	struct Geom;
	struct Anim;
	struct Material;
	struct Deformer;

	//=== skeleton info ===
	struct Model : public Entry
	{	GsVec t, r; Model* parent; Anim* anim; KnJoint* j; Material* mtl; Geom* g; GsArray<Deformer*> d;
		Model() { parent=0; anim=0; j=0; mtl=0; g=0; }
	};
	GsTablePt<Model> M;  // all parsed models
	GsArray<Model*> J;   // models representing joints
	GsArray<Model*> GEO; // models representing geometries

	//=== anim info ===
	struct Curve
	{	gsuint id; GsArray<double> times; GsArray<double> values; 
		Curve() { id=0; }
	};
	struct Anim
	{	gsuint id; char type; gsuint modelid; Curve *cx, *cy, *cz;
		Anim() { id=0; type='R'; modelid=0; cx=cy=cz=0; }
	};
	GsTablePt<Anim> A;  // all parsed animation nodes
	GsTablePt<Curve> C; // all parsed animation curves

	//=== geometry info ===
	struct Geom : public Entry { int gsm; GsArray<Deformer*> d; Geom(){gsm=-1;} };
	struct Texture : public Entry { GsString fname, relfname; };
	struct Material : public Entry { GsMaterial mtl; Texture* glosmap, *specmap, *opacmap, *normap, *difmap;
									 Material() { glosmap=specmap=opacmap=normap=difmap=0; } };
	GsTablePt<Texture> Txt;  // all parsed textures
	GsTablePt<Material> Mtl; // all parsed materials
	GsTablePt<Geom> G;		 // all parsed geometries
	GsArrayRef<GsModel> GSM; // gsmodels created
	GsArray<Geom*> GSMG;	 // for each element in GSM here goes the pointer to its Geom

	//=== deformers info ===
	struct Deformer : public Entry { GsArray<int> i; GsArray<double> w, t, tl, fw; Model *mg, *mj; Deformer(){mg=mj=0;} };
	GsTablePt<Deformer> Def;  // all parsed deformers

	//=== internal data ===
	GsArray<double> _buffer; // internal buffer
	GsArray<int> _vi, _ni, _ti; // internal buffers
	GsString _fbxfname;

   public :

	/*! Constructor */
	KnFbxImporter ();

	/*! Destructor is public but pay attention to the use of ref()/unref() */
	virtual ~KnFbxImporter ();

	/*! Clears all internal data */
	void init ();

	/*! Loads a .fbx file in ASCII format, returns true if data was loaded and false if error.
		Parameter options is a string where each letter, in any order, defines:
		s: simplify names by removing any namespaces
		If a null string is given, the default option string considered is: "s". */
	bool load ( const char* filename, const char* options=0 );

	/*!	Builds in sk the skeleton information loaded from the most recent fbx file loaded.
		Returns true if the skeleton was built, or false if no skeleton data was loaded. */
	bool get_skeleton ( KnSkeleton *sk );

	/*!	Access to all models built from the most recent fbx file loaded. */
	const GsArrayRef<GsModel>& models () { return  GSM; }

   protected : // loading of motions not yet implemented:
	int num_motions () { return 0; }
	bool get_motion ( KnSkeleton *sk, KnMotion* m );

   protected :
	bool get_skin ( KnSkeleton *sk, KnSkin *skin );
	void read_entry ( GsInput& lin, Entry* e, GsString& idst, bool simpnames );
	void read_mesh ( GsInput& fin, Geom* g );
	void read_properties ( GsInput& fin, Model* m );
	void read_anim ( GsInput& lin, GsInput& fin );
	void read_curve ( GsInput& lin, GsInput& fin );
	void read_texture ( GsInput& fin, Texture* t );
	void read_material ( GsInput& fin, Material* m );
	void read_deformer ( GsInput& fin, Deformer* d );
	void read_connections ( GsInput& fin );
	void add_children ( KnSkeleton* sk, Model* pm, KnJoint* pj );
	void finalize_model ( GsDirs& d, int gi, GsModel& m, const Material* mtl );
	void finalize_models ();
	void set_influences ( KnSkin* skin, int mid, Deformer* d );
};

//================================ End of File =================================================

# endif  // KN_FBX_IMPORTER_H
