/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_GRID_H
# define GS_GRID_H

/** \file gs_grid.h 
 * Manages data in a n-D regular grid */

# include <sig/gs_vec.h>
# include <sig/gs_vec2.h>
# include <sig/gs_array.h>
# include <sig/gs_output.h>
# include <sig/gs_manager.h> 

/*! Describes how to subdivide one dimensional axis.
	This class is required to set up the decomposition in GsGrid */
class GsGridAxis
{  public :
	int   segs; // number of segments to divide this axis
	float min;  // minimum coordinate
	float max;  // maximum coordinate

   public :
	GsGridAxis ( int se, float mi, float ma )
	{	segs=se; min=mi; max=ma; }

	void set ( int se, float mi, float ma )
	{	segs=se; min=mi; max=ma; }

	friend GsOutput& operator<< ( GsOutput& o, const GsGridAxis& a )
	{	return o << a.segs << gspc << a.min << gspc << a.max; }

	friend GsInput& operator>> ( GsInput& i, GsGridAxis& a )
	{	return i >> a.segs >> a.min >> a.max; }
};

/*! \class GsGridBase gs_grid.h
	\brief n-D regular grid base class

	GsGridBase has methods to translate n dimensional grid coordinates
	into a one dimension bucket array coordinate. Use the derived GsGrid
	template class to associate user data to the grid. */
class GsGridBase
 { private :
	GsArray<GsGridAxis> _axis;
	GsArray<int> _size; // subspaces sizes for fast bucket determination
	GsArray<float> _seglen; // axis segments lengths for fast cell determination
	int _cells;

   public :

	/*! Constructs a grid of given dimension (dim) and number of
		segments in each axis (ns). The number of cells will be ns^dim.
		Axes are normalized in [0,1] */
	GsGridBase ( int dim=0, int ns=0 );

	/*! Initializes the grid with given dimension and number of
		segments in each axis. The number of cells will be 	ns^dim.
		Axes are normalized in [0,1] */
	void init ( int dim, int ns );

	/*! Destroy the existing grid and initializes a new one according
		to the descriptions in the desc array. */
	void init ( const GsArray<GsGridAxis>& axis_desc );

	const GsArray<GsGridAxis>& axis_desc() const { return _axis; }

	float max_coord ( int axis ) const { return _axis[axis].max; }
	float min_coord ( int axis ) const { return _axis[axis].min; }
	int segments ( int axis ) const { return _axis[axis].segs; }
	float seglen ( int axis ) const { return _seglen[axis]; }

	/*! Returns the number of dimensions of the grid */
	int dimensions () const { return _axis.size(); }

	/*! Returns the total number of cells in the grid */
	int cells () const { return _cells; }

	/*! Get the index of a cell from its coordinates. 2D version only. */
	int cell_index ( int i, int j ) const { return _size[1]*j + i; }

	/*! Get the index of a cell from its coordinates. 3D version only. */
	int cell_index ( int i, int j, int k ) const { return _size[2]*k + _size[1]*j + i; }

	/*! Get the index of a cell from its coordinates. Multidimensional version */
	int cell_index ( const GsArray<int>& coords ) const;

	/*! Get the cell coordinates from its index. 2D version. */
	void cell_coords ( int index, int& i, int& j ) const;

	/*! Get the cell coordinates from its index. 3D version. */
	void cell_coords ( int index, int& i, int& j, int& k ) const;

	/*! Get the cell coordinates from its index. Multidimensional version. */
	void cell_coords ( int index, GsArray<int>& coords ) const;

	/*! Get the lower and upper coordinates of the Euclidian 2D box of cell (i,j). */
	void cell_boundary ( int i, int j, GsPnt2& a, GsPnt2& b ) const;

	/*! Get the lower and upper coordinates of the Euclidian 3D box of cell (i,j,k). */
	void cell_boundary ( int i, int j, int k, GsPnt& a, GsPnt& b ) const;

	/*! Get the indices of all cells intersecting with the given box.
		If a box boundary is exactly at a grid separator, only the cell
		with greater index is considered. Indices are just pushed to
		array cells, which is not emptied before being used */
	void get_intersection ( GsPnt2 a, GsPnt2 b, GsArray<int>& cells ) const;

	/*! 3D version of get_intersection() */
	void get_intersection ( GsPnt a, GsPnt b, GsArray<int>& cells ) const;

	/*! Returns the index of the cell containing a, or -1 if a is outside the grid */
	int get_point_location ( GsPnt2 a ) const;

	/*! Returns the index of the cell containing a, or -1 if a is outside the grid */
	int get_point_location ( GsPnt a ) const;
};

/*! \class GsGrid gs_grid.h
	\brief n-D regular grid template class

	GsGrid defines automatic type casts to the user type.
	WARNING: GsGrid is designed to efficiently store large
	amount of cells and it uses GsArray<X>, which implies
	that no constructors or destructors of type X are called.
	The user must initialize and delete data properly if needed. */
template <class X>
class GsGrid : public GsGridBase
 { private :
	GsArray<X> _data;

   public :
	GsGrid ( int dim=0, int ns=0 ) : GsGridBase ( dim, ns )
	 { _data.size(GsGridBase::cells()); }

	void init ( int dim, int ns )
	 { GsGridBase::init ( dim, ns );
	   _data.size(GsGridBase::cells());
	 }

	void init ( const GsArray<GsGridAxis>& axis_desc )
	 { GsGridBase::init ( axis_desc );
	   _data.size(GsGridBase::cells());
	 }

	void setall ( const X& x ) { _data.setall(x); }

	/*! Returns the cell of given index, that should be in 0<=i<cells() */
	X& get ( int index ) { return _data[index]; }

	/*!Const cell access */
	const X& cget ( int index ) const { return _data[index]; }

	/*! Returns the cell of given 2D coordinates */
	X& get ( int i, int j )
	 { return _data[GsGridBase::cell_index(i,j)]; }

	/*! Const access given 2D coordinates */
	const X& cget ( int i, int j ) const
	 { return _data[GsGridBase::cell_index(i,j)]; }

	/*! Returns the cell of given index, that should be in 0<=i<cells() */
	X& operator[] ( int index ) { return _data[index]; }

	/*! Returns the cell of given 2D coordinates */
	X& operator() ( int i, int j )
	 { return _data[GsGridBase::cell_index(i,j)]; }

	/*! Returns the cell of given 3D coordinates */
	X& operator() ( int i, int j, int k )
	 { return _data[GsGridBase::cell_index(i,j,k)]; }

	/*! Returns the cell of given n-D coordinates */
	X& operator() ( const GsArray<int>& coords )
	 { return _data[GsGridBase::cell_index(coords)]; }
};

//============================== end of file ===============================

# endif // GS_GRID_H
