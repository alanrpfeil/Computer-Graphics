/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file gs_matn.h 
 * n-dimensional matrix */

# ifndef GS_MATN_H
# define GS_MATN_H

# include <sig/gs_buffer.h>

class GsInput;
class GsOutput;

/*! \class GsMatn gs_matn.h
	\brief resizeable, n-dimensional matrix of double elements

	GsMatn is a resizeable, n-dimensional matrix of elements of
	type double and encapsulates several matrix operations.
	Attention: indices are considered to start from index 0. */
class GsMatn
 { private :
	GsBuffer<double> _data;
	int _lin, _col;
   public :

	/*! Default Constructor */
	GsMatn ();

	/*! Copy Constructor */
	GsMatn ( const GsMatn &m );

	/*! Constructor with given size, elements are not initialized */
	GsMatn ( int m, int n );

	/*! Destructor */
   ~GsMatn () { }

	/*! Old elements are not preserved when realloc is done. */
	void size ( int m, int n );

	/*! returns the size of the matirx, ie, lin*col. */
	int size () const { return _data.size(); }

	/*! resize also copies the old values to the new resized matrix. */
	void resize ( int m, int n );

	/*! Resizes this GsMatn and copy the contents of the submatrix
		[li..le,ci..ce] of m to this GsMatn. */
	void submat ( const GsMatn &m, int li, int le, int ci, int ce );

	/*! Copies the submatrix [li..le,ci..ce] of m to this GsMatn, without 
		reallocation, and putting the submatrix starting at coordinates 
		[l,c] of this GsMatn. */
	void submat ( int l, int c, const GsMatn &m,  int li, int le, int ci, int ce );

	/*! Resizes GsMatn as a column vector containing the data of the 
		s column of m. */
	void column ( const GsMatn &m, int s );

	/*! Copies the data of the column mc of m to the column c of GsMat. */
	void column ( int c, const GsMatn &m, int mc );

	int lin () const { return _lin; }
	int col () const { return _col; }
	void zero () { _data.setall(0); }
	void identity ();
	void transpose ();
	void swaplines ( int l1, int l2 );
	void swapcolumns ( int c1, int c2 );
	void setall ( double val ) { _data.setall(val); }
	void random ( double inf, double sup );
	void random ( float inf, float sup );

	/*! Returns the (lin*col)-dimensional vector norm. */
	double norm () const;

	operator const double* () const { return _data; }

	/*! indices start from 0. */
	double& operator [] ( int p ) { return _data[p]; }

	/*! indices start from 0. */
	double& operator () ( int i, int j ) { return _data[_col*i+j]; }

	void set ( int p, double r ) { _data[p]=r; }
	void set ( int i, int j, double r ) { _data[_col*i+j]=r; }

	double get ( int p ) const { return _data[p]; }
	double get ( int i, int j ) const { return _data[_col*i+j]; }

	void add ( const GsMatn& m1, const GsMatn& m2 );
	void sub ( const GsMatn& m1, const GsMatn& m2 );

	/*! Makes GsMatn be m1*m2. This method works properly with calls like 
		a.mult(a,b), a.mult(b,a), or a.mult(a,a). */
	void mult ( const GsMatn& m1, const GsMatn& m2 );

	void abandon ( GsBuffer<double>& buf );
	void adopt ( GsMatn &m );
	void adopt ( GsBuffer<double>& buf, int m, int n );

	void operator =  ( const GsMatn& m );
	void operator += ( const GsMatn& m );
	void operator -= ( const GsMatn& m );
	void operator *= ( float s );

	friend GsOutput& operator << ( GsOutput &o, const GsMatn &m );

	/*! Returns the norm of the matrix a-b. */
	friend double dist ( const GsMatn &a, const GsMatn &b );

	/* Transforms a into its encoded LU decomposition. The L and U are returned
	   in the same matrix a. L can be retrieved by making L=a and then setting 
	   all elements of the diagonal to 1 and those above the diagonal to 0. U is 
	   retrieved by making U=a, and then setting all elements below the diagonal
	   to 0. Then the multiplication LU will give a', where a' is a with some
	   row permutations. The row permutation history is stored in the returned
	   int array, that tells, beginning from the first row, the other row index
	   to swap sequentially the rows. This swap sequence will transform a to a'. 
	   If pivoting is set to false, then no pivoting is done and a' will be equal
	   to a. Parameter d returns +1 or -1, depending on whether the number of 
	   row interchanges was even or odd, respectively. The function returns null
	   if a is singular. See Numerical Recipes page 46. */
	friend const int* ludcmp ( GsMatn &a, double *d=0, bool pivoting=true );

	/*! Returns the explicit LU decomposition of a. Here we decompose the result
		of the encoded ludcmp version (without pivoting), returning the exact 
		l and u matrices so that lu=a.*/
	friend bool ludcmp ( const GsMatn &a, GsMatn &l, GsMatn &u );

	/*! Solves the linear equations ax=b. Here a is a n dimension square matrix,
		given in its LU encoded decomposition, b is a n dimensional column vector,
		that will be changed to return the solution vector x. indx is the row 
		permutation vector returned by ludcmp. */
	friend void lubksb ( const GsMatn &a, GsMatn &b, const int *indx );

	/*! Solve the system of linear equations ax=b using a LU decomposition. 
		Matrix a is changed to its encoded LU decomposition, and matrix b
		will be changed to contain the solution x. */
	friend bool lusolve ( GsMatn &a, GsMatn &b );

	/*! Same as the other lusolve(), but here const qualifiers are respected,
		what implies some extra temporary buffers allocation deallocation. */ 
	friend bool lusolve ( const GsMatn &a, const GsMatn &b, GsMatn &x );

	/*! Will change a to its encoded LU decomposition and return in inva
		the inverse matrix of a. */
	friend bool inverse ( GsMatn &a, GsMatn &inva );

	/*! Will put in a its inverse. */
	friend bool invert ( GsMatn &a );

	/* Returns the determinant of a using the LU decomposition method. The 
	   given matrix a is transformed into the result of the ludcmp() function.
	   See Numerical Recipes page 49. */
	friend double det ( GsMatn &a );

	/*! Solve the system ax=b, using the Gauss Jordan method. */
	friend bool gauss ( const GsMatn &a, const GsMatn &b, GsMatn &x );
};

//============================== end of file ===============================

# endif // GS_MATN_H
