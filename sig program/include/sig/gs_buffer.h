/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_BUFFER_H
# define GS_BUFFER_H

/** \file gs_buffer.h 
 * fast buffer memory management template */

# include <sig/gs.h> 
# include <sig/gs_input.h> 
# include <sig/gs_output.h> 

/*! Allocates memory with sizeofx*newsize bytes using the C function realloc. 
	If newsize is zero, buffer is freed and both buffer and size becomes zero.
	After the function call, size has the value of newsize. No effect if 
	size==newsize. */
void* gs_buffer_size ( void* buffer, int sizeofx, int& size, int newsize );

/*! Inserts dp*sizeofx bytes at i position, moving correctly the buffer contents. */
void* gs_buffer_insert ( void* buffer, int sizeofx, int& size, int i, int dp );

/*! Inserts dp*sizeofx bytes at i position, moving correctly the buffer contents. */
void* gs_buffer_remove ( void* buffer, int sizeofx, int& size, int i, int dp );

/*! Put buffers with same size and copy them. No effect if buffers pointers are equal. */
void* gs_buffer_copy ( void* buffer, int sizeofx, int& size, const void* buffertocp, int sizetocp );

/*! \class GsBuffer gs_buffer.h
	\brief fast buffer memory management template

	All memory management functions of GsBuffer are written using the four gs_buffer
	functions available in this header. The behavior of GsBuffer is very similar to
	that of GsArray, except that GsBuffer does not maintain a buffer capacity. 
	Therefore GsBuffer requires less memory but always needs to re-allocate
	memory when data is manipulated */
template <typename X>
class GsBuffer
 { private:
	void* _data;
	int   _size;
   public:

	/*! Default constructor. */
	GsBuffer () : _data(0), _size(0) {}

	/*! Copy constructor. */
	GsBuffer ( const GsBuffer& b ) : _data(0), _size(0)
	 { _data=gs_buffer_copy(_data,sizeof(X),_size,b._data,b._size); }

	/*! Constructor with a given size. */
	GsBuffer ( int s ) : _data(0), _size(0) { _data=gs_buffer_size(_data,sizeof(X),_size,s); }

	/*! Constructor from a user allocated buffer. See also leave_data(). */
	GsBuffer ( X* pt, int s ) : _data(pt), _size(s) {}

	/*! Destructor frees the buffer. Elements' destructors are not called ! */
   ~GsBuffer () { gs_buffer_size ( _data, sizeof(X), _size, 0 ); }

	/*! Returns true if size()==0; and false otherwise. */
	bool empty () const { return _size==0; }

	/*! Returns the current size of the Buffer. */
	int size () const { return _size; }

	/*! Allows to change the size of the buffer. */
	void size ( int ns ) 
	 { _data=gs_buffer_size(_data,sizeof(X),_size,ns); }

	/*! Sets all elements as x, copying each element using operator = */
	void setall ( const X& x )
	 { int i; for ( i=0; i<_size; i++ ) ((X*)_data)[i]=x; }

	/*! Returns a valid index from an index that may be out of bounds by assuming it
		refers to a circular mapping, ie, it returns index%size() for positive numbers;
		and negative numbers are also correctly mapped. */
	int vid ( int index ) const { if (index<0) index=_size-(-index%_size); return index%_size; }

	/*! Validate a positive index by returning index%size(). */
	int vidpos ( int index ) const { return index%_size; }

	/*! Gets a const reference to the element of index i. Indices start from 0 and must 
		be smaller than size(). No checkings are done to ensure that i is valid. */
	const X& cget ( int i ) const { return ((X*)_data)[i]; }

	/*! Gets a reference to the element of index i. Indices start from 0 and must 
		be smaller than size(). No checkings are done to ensure that i is valid. */
	X& get ( int i ) const { return ((X*)_data)[i]; }

	/*! Sets an element. Operator = is used here. Indices start from 0 and must 
		be smaller than size(). No checkings are done to ensure that i is valid. */
	void set ( int i, const X& x ) { ((X*)_data)[i]=x; }

	/*! Operator version of get(), which returns a non const reference.
		No checkings are done to ensure that i is valid. */
	X& operator[] ( int i ) const { return ((X*)_data)[i]; }

	/*! Operator version of cget(), which returns a const reference.
		No checkings are done to ensure that i is valid. */
	const X& operator() ( int i ) const { return ((X*)_data)[i]; }

	/*! Returns a const pointer of the internal buffer. The internal buffer 
		will always contain a contiguous storage space of size() elements. 
		See also abandon() and adopt() methods. */
	operator const X* () const { return (X*)_data; }

	/*! Returns a non-const pointer for the internal buffer. The internal buffer 
		will always contain a contiguous storage space of size() elements.
		See also abandon() and adopt() methods. */
	X* pt () const { return (X*)_data; }

	/*! Returns a reference to the last element, ie, with index size()-1.
		The array must not be empty when calling this method. */
	X& top () const { return ((X*)_data)[_size-1]; }

	/*! Const version of top() method.
		The array must not be empty when calling this method. */
	const X& ctop () const { return ((X*)_data)[_size-1]; }

	/* Reduces the size of the buffer by one. */
	void pop () { _data=gs_buffer_size(_data,sizeof(X),_size,_size-1); }

	/* Increases the size of the buffer by dp, which has default value 1. */
	void push ( int dp=1 ) { _data=gs_buffer_size(_data,sizeof(X),_size,_size+dp); }

	/* Increases the size of the buffer by one and copy x to the new entry using copy operator. */
	void push ( const X& x ) { _data=gs_buffer_size(_data,sizeof(X),_size,_size+1); top()=x; }

	/*! Inserts dp positions, starting at pos i, moving all data correctly. 
		Parameter i can be between 0 and size(), if i==size(), dp positions are
		appended. */
	void insert ( int i, int dp=1 ) { _data=gs_buffer_insert(_data,sizeof(X),_size,i,dp); }

	/*! Removes dp positions starting from pos i, moving all data correctly;
		dp has a default value of 1. Attention: elements' destructors are not
		called ! */
	void remove ( int i, int dp=1 ) { _data=gs_buffer_remove(_data,sizeof(X),_size,i,dp); }

	/*! Copies all internal data of a to GsBuffer, with fast memcpy() functions,
		so that the operator=() that X might have is not used. This method has no
		effect if a "self copy" is called. */
	void operator = ( const GsBuffer<X>& b )
	 { _data=gs_buffer_copy(_data,sizeof(X),_size,b._data,b._size); }
	
	/*! Makes the given xpt pointer to point to the internal buffer, without
		reallocation; xpt may be null if GsBuffer is empty. 
		The user will then be responsible to free this allocated memory with free().
		After this call, GsBuffer will become an empty (but valid) buffer. */
	void abandon ( X*& xpt, int& size )
	 { xpt=_data; size=_size; _data=0; _size=0; }

	/*! Frees the data of GsBuffer, and then makes GsBuffer be the given buffer b.
		After this, b is set to be a valid empty buffer. */
	void adopt ( GsBuffer<X>& b )
	 { size(0); _data=b._data; _size=b._size; b._data=0; b._size=0; }

	/*! Output all elements of the array.
		Element type X must have its ouput operator << available.
		The output format is [e0 e1 ... en] */
	friend GsOutput& operator<< ( GsOutput& o, const GsBuffer<X>& b )
	 { int i, m=b.size();
	   o << '[';
	   for ( i=0; i<m; i++ ) o  << ' ' << b[i];
	   return o << ' ' << ']';
	 }
	
	/*! Input all elements of the buffer from format [e0 e1 ... en].
		Element type X must have its input operator << available.
		As the size is not known in advance, automatic allocation is
		done using spaces multiple of 128 */
	friend GsInput& operator>> ( GsInput& in, GsBuffer<X>& b )
	 { int i=0;
	   b.size(128);
	   in.get(); // delimiter [
	   while ( in.check()!=GsInput::Delimiter ) 
		{ in>>b[i++]; if(i==b.size()) b.size(b.size()+128); }
	   b.size(i);
	   in.get(); // delimiter ]
	   return in;
	 }
};

//============================== end of file ===============================

#endif // GS_BUFFER_H

