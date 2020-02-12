/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SN_COLOR_SURF_H
# define SN_COLOR_SURF_H

/** \file sn_color_surf.h 
 * color interpolation in a mesh
 */

# include <sig/sn_model.h>

/*! \class SnColorSurf sn_color_surf.h
	\brief model color interpolation in a mesh

	Performs color interpolation among mesh vertices. */
class SnColorSurf : public SnModel
{  public :
	static const char* class_name; //<! Contains string SnColorSurf
	enum Mode { Illuminated, Colored };

   private :
	gsuint _gridnx;

   protected :

	/*! Constructor for derived classes */
	SnColorSurf ( const char* classname );

   public :

	/*! Constructor */
	SnColorSurf ();

	/*! Destructor */
   ~SnColorSurf ();

	/*! Init model V, N, and M arrays with size nv ready for manual insertion of values */
	void init_arrays ( int nv );

	/*! Utility to create a grid of vertices */
	void make_grid ( int nx, int ny );

	/*! Access to vertex or material index from grid coordinates */
	gsuint gid ( int i, int j ) { return _gridnx*j + i; }

	/*! Automatically compute smooth normals */
	void make_normals () { model()->smooth(-1); }

	/*! A mode has to be set before first render for correct results.
		It cannot be later modified because of the different number of required internal buffers. */
	void set_mode ( Mode m )
	 { if ( m==Illuminated ) 
		model()->set_mode(GsModel::Smooth,GsModel::PerVertexMtl); 
	   else 
		model()->set_mode(GsModel::Faces,GsModel::PerVertexColor); 
	 }
};

//================================ End of File =================================================

# endif // SN_COLOR_SURF_H
