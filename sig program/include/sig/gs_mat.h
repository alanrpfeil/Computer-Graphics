/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_MAT_H
# define GS_MAT_H

/** \file gs_mat.h 
 * A 4x4 matrix */

# include <sig/gs_vec.h>

/*! \class GsMat gs_mat.h
	\brief A 4x4 matrix

	GsMat is a 4x4 matrix used to store geometric transformations.
	Unlike early versions of OpenGL, the matrix is stored in 
	line-major format, which is closer to standard mathematical notation.
	For ex., the translation component is the last column (m[3],m[7],m[11]). */
class GsMat
{  public :
	/*! A union is used so that the elements of the matrix can
		be accessed either with array e, or by floats e11, e12, etc */
	union { struct { float e11, e12, e13, e14,
						   e21, e22, e23, e24,
						   e31, e32, e33, e34,
						   e41, e42, e43, e44; };
			float e[16];
		 };

	static const GsMat null;		//!< A null matrix
	static const GsMat id;			//!< An identity matrix
	enum InitializeMode { NoInit }; //!< Used by the "no init" constructor

   public :

	/*! Default constructor initializes GsMat as identity. */
	GsMat () { set(id.e); }

	/*! Copy constructor. */
	GsMat ( const GsMat& m ) { set(m.e); }

	/*! Constructor without initialization. GsMat values are simply not
		initialized when declaring GsMat m(GsMat::NoInit) */
	GsMat ( InitializeMode /*m*/ ) {}

	/*! Constructor from a float[16] pointer. */
	GsMat ( const float* p ) { set(p); }

	/*! Constructor from a double[16] pointer. */
	GsMat ( const double* p ) { set(p); }

	/*! Constructor from 16 values. */
	GsMat ( float a, float b, float c, float d, float e, float f, float g, float h, 
			float i, float j, float k, float l, float m, float n, float o, float p );

	/*! Copy operator. */
	void operator= ( const GsMat& m ) { set(m.e); }

	/*! float pointer type cast operator */
	operator float*() { return e; }

	/*! const float pointer type cast operator */
	operator const float*() const { return e; }

	/*! Returns a float pointer to the GsMat element with given index. */
	float* pt ( int i ) { return &e[i]; }

	/*! Returns a const float pointer to the GsMat element with given index. */
	const float* cpt ( int i ) const { return &e[i]; }

	/*! Access to an element of the matrix given an index from 0 to 15,
		ie, considering the matrix as a vector of 16 elements.
		No chekings are done on the index range. Implemented inline. */
	float& operator[] ( int i ) { return e[i]; }
	
	/*! operator() is the const version of operator[] */
	float operator() ( int i ) const { return e[i]; }

	/*! get() is the same as operator[], implemented inline */
	float& get ( int i ) { return e[i]; }

	/*! cget() is the same as operator(), implemented inline */
	float cget ( int i ) const { return e[i]; }

	/*! Set element i in [0,15], no checkings done on i; implemented inline */
	void set ( int i, float v ) { e[i]=v; }

	/*! Permits access to an element of the matrix using line and column
		indices that have to be in {0,1,2,3}. No index checking done. */
	float& get ( int i, int j ) { return e[i*4+j]; }

	/*! const version of get(i,j), implemented inline */
	float cget ( int i, int j ) const { return e[i*4+j]; }

	/*! Sets all elements of GsMat from the given float[16] pointer. */
	void set ( const float *p );

	/*! Sets all elements of GsMat from the given double[16] pointer. */
	void set ( const double *p );

	/*! Sets the four elements of 1st line. Implemented inline. */
	void setl1 ( float x, float y, float z, float w ) { e[0]=x; e[1]=y; e[2]=z; e[3]=w; }
	
	/*! Sets the four elements of 2nd line. Implemented inline. */
	void setl2 ( float x, float y, float z, float w ) { e[4]=x; e[5]=y; e[6]=z; e[7]=w; }
	
	/*! Sets the four elements of 3rd line. Implemented inline. */
	void setl3 ( float x, float y, float z, float w ) { e[8]=x; e[9]=y; e[10]=z; e[11]=w; }
	
	/*! Sets the four elements of 4th line. Implemented inline. */
	void setl4 ( float x, float y, float z, float w ) { e[12]=x; e[13]=y; e[14]=z; e[15]=w; }

	/*! Sets the first three elements of 1st line. Implemented inline. */
	void setl1 ( const GsVec& v ) { e[0]=v.x; e[1]=v.y; e[2]=v.z; }
	
	/*! Sets the first three elements of 2nd line. Implemented inline. */
	void setl2 ( const GsVec& v ) { e[4]=v.x; e[5]=v.y; e[6]=v.z; }
	
	/*! Sets the first three elements of 3rd line. Implemented inline. */
	void setl3 ( const GsVec& v ) { e[8]=v.x; e[9]=v.y; e[10]=v.z; }
	
	/*! Sets the first three elements of 4th line. Implemented inline. */
	void setl4 ( const GsVec& v ) { e[12]=v.x; e[13]=v.y; e[14]=v.z; }

	/*! Sets the four elements of column 1. Implemented inline. */
	void setc1 ( float x, float y, float z, float w ) { e[0]=x; e[4]=y; e[8]=z; e[12]=w; }
	
	/*! Sets the four elements of column 2. Implemented inline. */
	void setc2 ( float x, float y, float z, float w ) { e[1]=x; e[5]=y; e[9]=z; e[13]=w; }
	
	/*! Sets the four elements of column 3. Implemented inline. */
	void setc3 ( float x, float y, float z, float w ) { e[2]=x; e[6]=y; e[10]=z; e[14]=w; }
	
	/*! Sets the four elements of column 4. Implemented inline. */
	void setc4 ( float x, float y, float z, float w ) { e[3]=x; e[7]=y; e[11]=z; e[15]=w; }

	/*! Sets the first three elements of column 1. Implemented inline. */
	void setc1 ( const GsVec& v ) { e[0]=v.x; e[4]=v.y; e[8]=v.z; }
	
	/*! Sets the first three elements of column 2. Implemented inline. */
	void setc2 ( const GsVec& v ) { e[1]=v.x; e[5]=v.y; e[9]=v.z; }
	
	/*! Sets the first three elements of column 3. Implemented inline. */
	void setc3 ( const GsVec& v ) { e[2]=v.x; e[6]=v.y; e[10]=v.z; }
	
	/*! Sets the first three elements of column 4. Implemented inline. */
	void setc4 ( const GsVec& v ) { e[3]=v.x; e[7]=v.y; e[11]=v.z; }

	/*! Returns true if all elements are equal to 0.0, false otherwise. */
	bool isnull () const { return *this==null; }

	/*! Returns true if the matrix is identical to GsMat::id, false otherwise. */
	bool isid () const { return *this==id;   }

	/*! Makes GsMatrix be a null matrix (all elements are zero). */
	void zero () { *this=null; }

	/*! Round elements according to given precision, for ex 0.01 for 2 decimal digits */
	void round ( float prec );

	/*! Makes GsMatrix be an identity matrix. */
	void identity () { *this=id; }

	/*! Makes GsMatrix to receive the transposed of the line-major matrix stored in p */
	void getranspose ( const float *p );

	/*! Transpose GsMatrix. */
	void transpose ();

	/*! Transpose the 3x3 sub matrix. */
	void transpose3x3 ();

	/*! Makes GsMat be a translation transformation of tx,ty,tz */
	void translation ( float tx, float ty, float tz );

	/*! Make GsMat be a translation transformation of v. */
	void translation ( const GsVec& v ) { translation(v.x,v.y,v.z); }

	/*! Set the translation fields of the matrix to be tx, ty, tz */
	void setrans ( float tx, float ty, float tz ) { e[3]=tx; e[7]=ty; e[11]=tz; }

	/*! Set the translation fields of the matrix to be v.x, v.y, v.z */
	void setrans ( const GsVec& v ) { setrans(v.x,v.y,v.z); }

	/*! Get the translation fields of the matrix and put in tx, ty, tz */
	void getrans ( float& tx, float& ty, float& tz ) const { tx=e[3]; ty=e[7]; tz=e[11]; }

	/*! Get the translation fields of the matrix and put in v.x, v.y, v.z */
	void getrans ( GsVec& v ) const { getrans(v.x,v.y,v.z); }

	/*! Pre-multiplies GsMat with a translation matrix of translation v */
	void lcombtrans ( const GsVec& v );

	/*! Pre-multiplies GsMat with a translation matrix of translation v,
		but assuming GsMat is affine, i.e., its 4th line is 0,0,0,1 */
	void lcombtransaff ( const GsVec& v );

	/*! Pos-multiplies GsMat with a translation matrix constructed with v */
	void rcombtrans ( const GsVec& v );

	/*! Pos-multiplies GsMat with a scaling matrix of factors sx, sy and sz */
	void rcombscale ( float sx, float sy, float sz );

	/*! Pos-multiplies GsMat with a scaling matrix of factors sx, sy and sz */
	void lcombscale ( float sx, float sy, float sz );

	/*! Makes GsMat be a scaling transformation matrix */
	void scaling ( float sx, float sy, float sz );

	/*! Makes GsMat be a scaling transformation matrix */
	void scaling ( float s ) { scaling(s,s,s); }

	/*! Makes GsMat be a rotation transformation around x axis. The given parameters 
		are the sine and cosine of the desired angle of rotation. */
	void rotx ( float sa, float ca );

	/*! Makes GsMat be a rotation transformation around y axis. The given parameters 
		are the sine and cosine of the desired angle of rotation. */
	void roty ( float sa, float ca );

	/*! Makes GsMat be a rotation transformation around z axis. The given parameters 
		are the sine and cosine of the desired angle of rotation. */
	void rotz ( float sa, float ca );

	/*! Makes a rotation transformation around x axis of given angle in radians. */
	void rotx ( float radians );

	/*! Makes a rotation transformation around y axis of given angle in radians. */
	void roty ( float radians );

	/*! Makes a rotation transformation around z axis of given angle in radians. */
	void rotz ( float radians );

	/*! Rotation around an axis of given angle, given by its sinus and cosinus.
		The given axis of rotation vec is not required to be normalized. */
	void rot ( const GsVec& vec, float sa, float ca );

	/*! Rotation around an axis given an angle in radians.
		The given axis of rotation vec is not required to be normalized. */
	void rot ( const GsVec& vec, float radians );

	/*! Gives the rotation matrix that rotates one vector to another.
		Vectors are not required to be normalized. */
	void rot ( const GsVec& from, const GsVec& to );

	/*! Set GsMat to be the rigid transformation matrix that maps the three
		given vertices into the plane xy. After transformed, v1 will go to 
		the origin and v2 will be along the X axis. */
	void projxy ( GsVec v1, GsVec v2, GsVec v3 );

	/*! Set the matrix to be the OpenGL glut-like camera transformation; however,
		the result is in line-major format and fovy parameter is in radians.
		Note: znear and zfar must be positive. */
	void perspective ( float fovy, float aspect, float znear, float zfar );

	/*! Set the matrix to the OpenGL gluLookAt camera transformation; however,
		the result is in line-major format. */
	void lookat ( const GsVec& eye, const GsVec& center, const GsVec& up );

	/*! Set the transformation for mapping normalized screen coordinates to 
		window coordinates, for a window of given width and height. 
		Parameter m sets a marging of m pixels in the mapping, so that
		extreme coordinates are not clipped. 
		Equivalent to ortho(-xm,w+xm,h+ym,-ym,-1,1). */
	void orthowin ( float w, float h, float xm, float ym );

	/*! Set the matrix to the OpenGL glOrtho matrix, but result is in line-major format. */
	void ortho ( float left, float right, float bottom, float top, float near, float far );

	/*! Fast invertion by direct calculation, no loops, no gauss, no pivot searching, 
		but with more numerical errors. The result is returned in the 'inv' parameter. */
	void inverse ( GsMat& inv ) const;

	/*! Returns the inverse in a new matrix returned by value, callinf the inverse(GsMat&) method*/
	GsMat inverse () const  { GsMat inv(NoInit); inverse(inv); return inv; }

	/*! Makes GsMat to be its inverse, calling the inverse() method. */
	void invert () { *this=inverse(); }

	/*! Fast 4x4 determinant by direct calculation, no loops, no gauss. */
	float det () const;

	/*! Fast 3x3 determinant by direct calculation. */
	float det3x3 () const;

	/*! Considers the matrix as a 16-dimensional vector and returns its norm raised
		to the power of two. */
	float norm2 () const;

	/*! Considers the matrix as a 16-dimensional vector and returns its norm. */
	float norm () const;

	/*! Set GsMat to be the result of the multiplication of m1 with m2.
		This method is safe if one of the given parameters is equal to 'this'. */
	void mult ( const GsMat& m1, const GsMat& m2 );

	/*! Set GsMat to be the result of the multiplication of affine matrices m1 and m2,
		i.e., with 4th line 0,0,0,1. GsMat must be different than m1 and m2. */
	void multaff ( const GsMat& m1, const GsMat& m2 );

	/*! Sets GsMat to be the addition of m1 with m2. */
	void add ( const GsMat& m1, const GsMat& m2 );

	/*! Sets GsMat to be the difference m1-m2. */
	void sub ( const GsMat& m1, const GsMat& m2 );

	/*! Distance between two matrices, considering them as a 16-dimensional vector. */
	friend float dist ( const GsMat& a, const GsMat& b );

	/*! Distance between two matrices raised to two, considering them as a 16-dimensional vector. */
	friend float dist2 ( const GsMat& a, const GsMat& b );

	/*! Returns true if dist2(*this,m)<=ds*ds, and false otherwise. */
	friend bool next ( const GsMat& a, const GsMat& b, float ds ) { return dist2(a,b)<=ds*ds; }

	/*! Operator to multiply GsMat by a scalar. */
	void operator *= ( float r );

	/*! Operator to (right) multiply GsMat by another GsMat m. */
	void operator *= ( const GsMat& m );

	/*! Operator to add to GsMat another GsMat. */
	void operator += ( const GsMat& m );

	/*! Operator to multiply a GsMat to a scalar, returning another GsMat. */
	friend GsMat operator * ( const GsMat& m, float r );

	/*! Operator to multiply a scalar to a GsMat, returning another GsMat. */
	friend GsMat operator * ( float r, const GsMat& m ) { return m*r; }

	/*! Operator to multiply a GsMat to a GsVec, returning another GsVec. */
	friend GsVec operator * ( const GsMat& m,  const GsVec& v  );

	/*! Operator to multiply a GsVec to a GsMat, returning another GsVec. */
	friend GsVec operator * ( const GsVec& v,  const GsMat& m  );

	/*! Operator to multiply two GsMats, returning another GsMat. */
	friend GsMat operator * ( const GsMat& m1, const GsMat& m2 )
			{ GsMat m(GsMat::NoInit); m.mult(m1,m2); return m; }

	/*! Operator to add two GsMats, returning another GsMat. */
	friend GsMat operator + ( const GsMat& m1, const GsMat& m2 );

	/*! Operator to compute the difference of two GsMats, returning another GsMat. */
	friend GsMat operator - ( const GsMat& m1, const GsMat& m2 );

	/*! Comparison operator to check if two GsMats are (exactly) equal. */
	friend bool operator == ( const GsMat& m1, const GsMat& m2 );

	/*! Comparison operator to check if two GsMat are different. */
	friend bool operator != ( const GsMat& m1, const GsMat& m2 );

	/*! Outputs 16 floats, 4 elements per line. */
	friend GsOutput& operator<< ( GsOutput& o, const GsMat& m );

	/*! Reads 16 float numbers from the input. */
	friend GsInput&  operator>> ( GsInput& in, GsMat& m );
};

//============================== end of file ===============================

# endif // GS_MAT_H
