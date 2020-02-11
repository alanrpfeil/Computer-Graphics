/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_grid.h>

void out ( GsGridBase& g )
 {
   gsout << "Dim:   " << g.dimensions() << gsnl;
   gsout << "Cells: " << g.cells() << gsnl;
   gsout << gsnl;
 }

void test4x4 ()
 {
   GsGridBase g;
   int cell;
   GsArray<int> c;

   g.init ( 4, 3 );
   c.size(4);

   int i, j, k, t;

   for ( t=0; t<3; t++ )
	for ( k=0; k<3; k++ )
	 for ( j=0; j<3; j++ )
	  for ( i=0; i<3; i++ )
		{ c[0]=i; c[1]=j; c[2]=k; c[3]=t; 
		  cell = g.cell_index(c);
		  gsout << i << "," << j << "," << k << "," << t << ": "
				 << cell << gsnl;
		}

   out ( g );
 }

struct Data
{	int index;
	double data;
};

void test_grid ()
 {
   GsGrid<Data> g;
   GsArray<GsGridAxis> ax;

   ax.size(2);
   ax[0].min = 1.0f;
   ax[0].max = 5.0f;
   ax[0].segs = 4;
   ax[1].min = 1.0f;
   ax[1].max = 2.0f;
   ax[1].segs = 3;

   g.init ( ax );

   int idx, i, j;

   for ( idx=0; idx<g.cells(); idx++ )
	g[idx].index = idx;

   for ( idx=0; idx<g.cells(); idx++ )
	{ g.cell_coords(idx,i,j);
	  gsout << i << "," << j << ": " << idx << gsnl;
	}

   GsPnt2 a, b;
   for ( idx=0; idx<g.cells(); idx++ )
	{ g.cell_coords(idx,i,j);
	  g.cell_boundary(i,j,a,b);
	  gsout << i << "," << j << ": ";
	  gsout << a << "; " << b << gsnl;
	}

   GsArray<int> cells;
   a.set ( 1.0f, 1.0f );
   b.set ( 3.0f, 2.0f );
   g.get_intersection ( a, b, cells );
   gsout<<"Intersection: ";
   for ( idx=0; idx<cells.size(); idx++ )
	gsout<<cells[idx]<<gspc;
   gsout<<gsnl;

   out ( g );
 }
