/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_MODEL_H
# define GS_MODEL_H

/** \file gs_model.h
 * 3d model with triangular faces
 */

class SnCylinder;
class SnSphere;
class GsImage;
class GsPolygon;

# include <sig/gs_box.h>
# include <sig/gs_vec.h>
# include <sig/gs_mat.h>
# include <sig/gs_quat.h>
# include <sig/gs_line.h>
# include <sig/gs_string.h>
# include <sig/gs_strings.h>
# include <sig/gs_material.h>
# include <sig/gs_primitive.h>

# include <sig/gs_shareable.h>

/*! \class GsModel gs_model.h
	\brief a model composed of triangular faces

	GsModel keeps arrays of vertices, normals, texture coordinates,
	materials, and of additional information.
	These arrays are public and can be directly manipulated by the user.
	It is the user responsibility to maintain all indices in a coherent way.*/
class GsModel : public GsShareable
{ public :
	/*! The triangular face keeps 3 indices to the associated vertex information */
	struct Face 
	{	int a, b, c;
		Face () {}
		Face ( int i, int j, int k ) { a=i; b=j; c=k; }
		void set ( int i, int j, int k ) { a=i; b=j; c=k; }
		void validate () { if (a<0) a=-a; if (b<0) b=-b; if (c<0) c=-c; }
		friend GsOutput& operator<< ( GsOutput& o, const Face& f ) { return o<<f.a<<gspc<<f.b<<gspc<<f.c; }
		friend GsInput& operator>> ( GsInput& i, Face& f ) { return i>>f.a>>f.b>>f.c; }
	};

	/*! Texture information */
	struct Texture
	{	int id;				//!< the texture index in GlResources, or <0 if not yet loaded
		unsigned int oglid; //!< if >0 then this OpenGL id will be used, otherwise image will come from GlResources
		GsCharPt fname;		//!< texture filename as read from model file
		Texture ( const char* file=0 ) : fname(file) { id=-1; oglid=0; }
	};

	/*! The group structure keeps material and texture information for group of faces */
	struct Group
	{	int fi, fn;			//!< the initial face index and the number of faces in this group
		GsCharPt mtlname;	//!< name of the material used by this group
		Texture* dmap;		//!< diffuse color map, or null pointer if none
		Group ( int i=0, int n=0 ) { fi=i; fn=n; dmap=0; }
		Group ( const Group& g ) { dmap=0; copyfrom(g); }
	   ~Group () { delete dmap; }
		void copyfrom ( const Group& g );
	};

	// Arrays containing the model data:
	GsArray<GsPnt>		V;  //!< List of vertex coordinates
	GsArray<GsVec>		N;  //!< List of normals
	GsArray<Face>		F;  //!< Triangular faces indices to V
	GsArray<Face>		Fn; //!< Indices to the normals in N (size is 0 or F.size())
	GsArray<GsPnt2>		T;  //!< List of texture coordinates (indexed by Ft, or T.size() must be equal to V.size())
	GsArray<Face>		Ft; //!< Indices to the texture coords in T
	GsArray<GsMaterial> M;  //!< List of materials
	GsArrayPt<Group>	G;  //!< Groups of faces with same material and texture (size is 0 or M.size())

	// Depending on how the data is organized, the model will be classified in geometry and material modes:

	/*! The geometry mode specifies the basic state and how the normals array should be interpreted */
	enum GeoMode { Empty,  //!< No F or V arrays defined, the model is empty
				   Faces,  //!< F and V are defined, N is empty, model will be flat but N will remain empty
				   Flat,   //!< F, V, and N defined, normals defined per face and N.size()==F.size()
				   Smooth, //!< F, V, and N defined, normals defined per vertex and N.size()==V.size()
				   Hybrid  //!< F, V, N, and Fn defined, normals indexed by indices in Fn: Fn.size()==F.size() (or as in Smooth mode)
				 };

	/*! The material mode specifies how the materials should be interpreted 
		If the model has a texture, the model has to be in PerGroupMtl mode. */
	enum MtlMode { NoMtl,		  //!< No materials defined: M.size()==G.size()==0
				   PerGroupMtl,   //!< Material per group: G.size()>0 and G.size()==M.size()
				   PerVertexMtl,  //!< Material per vertex: V.size()==M.size(), G.size()==0 (diffuse color per vertex, M[0] shared for other components)
				   PerVertexColor,//!< Same as PerVertexMtl, but N not used and only diffuse non-illuminated colors used per vertex
				   PerFaceMtl	  //!< Material per face: F.size()==M.size(), G.size()==0 (diffuse color per vertex, M[0] shared for other components)
				 };

	/*! May contain any desired name for the model. */
	GsString name;

	/*! Will contain the filename of the last file loaded or saved.
		This information is not saved in the model file itself, but 
		maintained as load and save methods are called. */
	GsString filename;

	/*! The primitive pointer will only be valid (not null) if this model is representing
		a primitive created by the make_primitive() method */
	GsPrimitive* primitive;

	/*! To be set to true (the default) if back face culling should be applied, and false otherwise */
	gscbool culling;

	/*! To be set to true if a texture map has been defined in the per-group materials.
		Only Hybrid PerGroupMtl mode will handle textures. */
	gscbool textured;

   private:
	gscenum _geomode; // modes have to bet set with mode() so that basic checks can take place
	gscenum _mtlmode; 
	void construct();

   public :

	/*! Constructor lets all internal arrays as empty and culling is set to true */
	GsModel () { construct(); }

	/*! Copy constructor */
	GsModel ( const GsModel& m ) { construct(); *this=m; }

	/*! Virtual Destructor */
	virtual ~GsModel ();

	/*! Set the mode of the model and perform basic checks if the internal arrays may correspond
		to the requested mode. Fatal error occurs if there is an important mismatch. */
	void set_mode ( GeoMode g, MtlMode m );

	/*! Will discover and set the mode of the model according to the size of the arrays. 
		Fast basic error checking, with fatal consequences, are performed. See also validate(). */
	void detect_mode ();

	GeoMode geomode () const { return (GeoMode)_geomode; } //!< returns the geometry mode
	MtlMode mtlmode () const { return (MtlMode)_mtlmode; } //!< returns the material mode

	/*! Returns true if the model has no faces, and false otherwise */
	bool empty () const { return F.empty(); }

	/*! Sets to an empty model. Mode is updated and used memory is freed. */
	void init ();

	/*! Sends to the output summary information about the model. Char sep is used to separate the information. */
	void statistics ( GsOutput& o, char sep=gsnl );

	/*! Copy operator */
	void operator = ( const GsModel& m );

	/*! Compress all internal array buffers. */
	void compress ();

	/*! Ensure data arrays have coherent sizes, and if not, will set the illegal ones to
		have size 0 and will then update the mode by calling detect_mode(). */
	void validate ();

  public : // construction functions:
	
	/*! Add faces, materials and normals of m to GsModel. The two models have to have no
		materials or materials defined per group. Primitive information, if any, is lost. */
	void add_model ( const GsModel& m );

	/*! Clear materials and then set M and G so that all triangles use the same material m,
		with full model compression (if last param is true) and MtlMode updated to PerGroupMtl. */
	void set_one_material ( const GsMaterial& m, const char* name=0, bool comp=true );

	/*! Clear material names, and the M and Fm array (with compression) */
	void clear_materials ();

	/*! Clear the T and Ft array (with compression. */
	void clear_texture_arrays ();

	/*! Empty G array and adjust MtlMode (with compression) */
	void clear_groups ();

	/*! Organize materials per groups in G from per-face material indices in array M */
	void define_groups ( const GsArray<int>& Fm, const GsStrings* mtlnames=0 );

	/*! Put transparent materials last in the G and M arrays */
	void order_transparent_materials ();

	/*! Removes redundant normals, which are very close or equal to each other. 
		Only applicable if Fn.size()>0 (Hybrid GeoMode). */
	void remove_redundant_normals ( float prec=gstiny );

	/*! Check and remove redundant vertices */
	void merge_redundant_vertices ( float prec=gstiny );

	/*! Clear the N and Fn arrays, with compression (if last param is true), then adjust mode. */
	void flat ( bool comp=true );

	/*! Generates normals for smooth surface respecting given crease angle in radians.
		GeoMode will become Smooth or Hybrid. Redundant normals are optimized and arrays compressed.
		If the crease angle is <=0, it is not considered, and smooth per-vertex normals are created,
		in which case the GeoMode will be Smooth. */
	void smooth ( float crease_angle=GS_TORAD(35.0f) );

  public : // query functions:

	/*! Returns 3F/2, which is the number of edges for "well connected" manifold meshes using Euler's formula */
	int euleredges () const { return 3*F.size()/2; }
  
	/*! Count and return the mean number of edges adjacent to a vertex in the model,
		by counting the aerage number of faces adjacent to a vertex. */
	float count_mean_vertex_degree ();

	/*! Returns the number of common vertices between the two faces given by their indices. */
	int common_vertices_of_faces ( int i, int j );

	/*! Calculates the bounding box of this model. */
	void get_bounding_box ( GsBox &box ) const;

	/*! Allocates with operator new an array va of V.size() where va[i] is an array containing
		information of the edges around V[i], such that: 
		V[i],V[va[i].b] forms an edge around V[i], adjacent to face F[va[i].a] with 3rd vertex V[va[i].c].
		The returned pointer must be freed by the user with operator delete[]. */
	GsArray<Face>* get_edges_per_vertex();

	/*! Allocates with operator new an array of V.size() where entry i has another array
		with indices to V for each edge around V[i]. The returned pointer must be freed
		by the user with operator delete[]. */
	GsArray<int>* get_edges();

	/*! Makes E to be an array containing the indices of the model edges,
		without any repetition; ie only one instance per edge.
		The number of edges in the model will thus be E.size()/2 */
	void get_edges ( GsArray<int>& E );

	/*! Returns the index of the face intersecting with the line, or -1 if
		no face is found. In case several intersections are found, the closest
		to line.p1 is returned */
	int pick_face ( const GsLine& line ) const;

	/*! Sequentially stores, for all faces, the 3 vertices of each face in the given array. */
	void get_vertices_per_face ( GsArray<GsVec>& fv ) const;

	/*! Sequentially stores, for all faces in Ft, the 3 vertex coordinates of each face in the given array. */
	void get_texcoords_per_face ( GsArray<GsVec2>& ftc ) const;

	/*! Sequentially stores, for all faces, the indices of the materials in the given array. */
	void get_materials_per_face ( GsArray<int>& fm ) const;

	/*! Sequentially stores in fn, for all faces, the 3 normals for each vertex of each face in the model. 
		If Fn.size()>0 its indices are used to retrieve the normals per face from N; if Fn.size()==0,
		then, in case N.size()==V.size() the normals in N are used, otherwise, flat normals are computed
		and returned. The returned array will always contain F.size()*3 normals. */
	void get_normals_per_face ( GsArray<GsVec>& fn ) const;

	/*! Computes the normals of all faces and store them with the given number of repetitions in the given array. */
	void get_flat_normals_per_face ( GsArray<GsVec>& fn, int repspernormal=1 ) const;

	/*! Calculates and returns the normalized normal of the given face index. */
	GsVec face_normal ( int f ) const;

	/*! Calculates and returns the center point of face f. */
	GsVec face_center ( int f ) const;

  public : // Geometry processing methods:

	/*! Inverts faces orientations by swaping b and c indices, 
		and does the same to the normals. */
	void invert_faces ();

	/*! Multiply all normals in N by -1 */
	void invert_normals ();

	/*! Translate the position of each vertex of the model.
		For primitive models, the primitive center is also translated.  */
	void translate ( const GsVec &tr );

	/*! Scale each vertex of the model. 
		For primitive models, each primitive radius is also scaled.  */
	void scale ( float factor );

	/*! Translates so that the bounding box center becomes (0,0,0).
		Primitive models correctly handled. */
	void centralize ();

	/*! Multiply array V and N by the given transformation matrix.
		N is transformed without translation and with renormalization.
		If parameter primtransf is true, only the rotation and translation
		in mat are aplied to the model and the primitive. If false,
		the current primitive information, if any, is lost. */
	void transform ( const GsMat& mat, bool primtransf=false );

	/*! Apply rotation in quaternion q to all vertices and normals.
		For primitive models, the primitive orientation is also rotate.  */
	void rotate ( const GsQuat& q );

	/*! Centralizes and scale to achieve maxcoord. */
	void normalize ( float maxcoord );

   public : // IO functions :

	/*! Checks the extension to be "obj" or "3ds", calling the apropiate importer,
		or otherwise it will load a GsModel in .m (or old .srm) format.
		The given filename is stored and can be accessed later on
		with filename() */
	bool load ( const char* filename );

	/*! Reads a .m format (old .srm format also supported). 
		Method in.filename() is needed for shared materials to work. */
	bool load ( GsInput& in );

	/*! This method imports a model in .obj format. If the import
		is succesfull, true is returned, otherwise false is returned. */
	bool load_obj ( const char* file );

	/*! This method imports a model in .3ds format. If the import
		is succesfull, true is returned, otherwise false is returned. */
	bool load_3ds ( const char* file );

	/*! This method imports a model in .iv or .wrl format. If the import
		is succesfull, true is returned, otherwise false is returned.
		Only the most common subset of commands are supported. */
	bool load_iv ( const char* file );

	/*! If the extension in file name is "iv" the model is exported in 
		.iv format, otherwise save the model in the .m format.
		The given filename is stored with method filename() */
	bool save ( const char* fname );

	/*! Save GsModel in the .m format.  */
	bool save ( GsOutput& o ) const;

	/*! Export model in Open Inventor .iv format */
	bool save_iv ( const char* file );

   public : // Make functions :

	/*! Make a model by sweeping the 2D polygonal cross section p placed at plane (z=0) in the
		direction of vector v. Polygon p must be given in CCW orientation. 
		If parameter bot is true (the default) the bottom of the 3D model (at z=0) is triangulated
		and filled with the triangles; otherwise it is not filled. Equivalently, parameter top specifies
		if the top face is to be filled with triangles. For a complete model both need to be true. */
	void make_sweep ( const GsPolygon& p, const GsVec& v, bool bot=true, bool top=true );

	/*! Make an "open tube" shape */
	void make_tube (  const GsPnt& a, const GsPnt& b, float ra, float rb, int nfaces, bool smooth );

	/*! Make a box shape */
	void make_box ( const GsBox& b );

	/*! Make a sphere shape */
	void make_sphere ( const GsPnt& c, float r, int nfaces, bool smooth );

	/*! Make an ellipsoid shape, based on make_sphere */
	void make_ellipsoid ( const GsPnt& c, float r, float ratio, int nfaces, bool smooth );

	/*! Make a cylinder shape */
	void make_cylinder ( const GsPnt& a, const GsPnt& b, float ra, float rb, int nfaces, bool smooth );

	/*! Make a capsule shape */
	void make_capsule (  const GsPnt& a, const GsPnt& b, float ra, float rb, int nfaces, bool smooth );

	/*! Make a primitive shape as described by GsPrimitive.
		By calling this method the parameters of the primitive are stored in 
		the model in member primitive, in that way the shape can be reconstructed
		with a different resolution, saved as a primitive, etc.
		Parameter mtlchoice can have 3 values:
		UsePrimMtl : model set as PerGroupMtl with single primitive material (default option),
		UseModelMtl : model set as PerGroupMtl with existing model M[0] material,
		UseNoMtl : no material is used and model is set to NoMtl mode. */
	enum MakePrimitiveMtlChoice { UsePrimMtl, UseModelMtl, UseNoMtl };
	void make_primitive ( const GsPrimitive& p, MakePrimitiveMtlChoice mtlchoice=UsePrimMtl );
};

//================================ End of File =================================================

# endif // GS_MODEL_H
