/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_QUEUE_H
# define GS_QUEUE_H

/** \file gs_queue.h 
 * Template for a queue based on GsArray. */

# include <sig/gs_array.h> 

/*! \class GsQueue gs_queue.h
	\brief Queue based on two GsArray 

	GsQueue implements a FIFO queue using two internal GsArray objects,
	one for elements being inserted, and the other for elements to output.
	Both GsArray work as stacks. Note that GsQueue is oriented for speed,
	and does not honors constructors or destructors of its elements X, see
	GsArray documentation for additional comments on this. To overcome
	this limitation, use GsQueue with pointers. */
template <typename X>
class GsQueue
{  private :
	GsArray<X> _in;
	GsArray<X> _out;
   public :

	/*! Constructor receives the capacity for both input and output arrays. */
	GsQueue ( int c=0 ) : _in(0,c), _out(0,c) {}

	/*! Copy constructor. */
	GsQueue ( const GsQueue& q ) : _in(q._in), _out(q._out) {}

	/*! Returns true if the queue is empty, false otherwise. */
	bool empty () const { return _in.empty() && _out.empty(); }

	/*! Returns the number of elements in the queue. */
	int size () const { return _in.size()+_out.size(); }

	/*! Make the queue empty. Attention: elements are not visited or maintained for deallocation. */
	void init () { _in.size(0); _out.size(0); }

	/*! Compress both input and output arrays. */
	void compress () { _in.compress(); _out.compress(); }

	/*! Returns a reference to the first element; the queue must not be empty. */
	X& first () { return _out.empty()? _in[0]:_out.top(); }

	/*! Returns a reference to the last element; the queue must not be empty. */
	X& last () { return _in.empty()? _out[0]:_in.top(); }

	/*! Access method, index 0 corresponds to the first element, and index
		size()-1, corresponds to the last element of the queue, i must obey 0<=i<size() */
	X& get ( int i ) { return i>=_out.size()? _in[i-_out.size()]:_out[_out.size()-1-i]; }

	/*! const version of get() */
	const X& cget ( int i ) const { return i>=_out.size()? _in[i-_out.size()]:_out[_out.size()-1-i]; }

	/*! same as get() */
	X& operator[] ( int i ) { return get(i); }

	/*! same as cget() */
	const X& operator() ( int i ) const { return cget(i); }

	/*! Inserts an element at the end of the queue. */
	X& insert () { return _in.push(); }

	/*! Removes and returns a reference to the first element of the queue;
		the queue must not be empty. */
	X& remove ()
	{	if ( _out.empty() ) flush();
		return _out.pop();
	}

	/*! Empties the input stack, correctly moving its elements to the output stack. */
	void flush ()
	{	if ( _in.empty() ) return;
		_out.insert ( 0, _in.size() );
		for ( int i=0; _in.size()>0; i++ ) _out[i]=_in.pop();
	}

	/*! Makes this queue be the same as q, which becomes an empty queue. */
	void adopt ( GsQueue<X>& q )
	{	_in.adopt ( q._in );
		_out.adopt ( q._out );
	}

	/*! Outputs all elements of the queue in format [e[0] e[1] ... e[size()-1]]. */
	friend GsOutput& operator<< ( GsOutput& o, const GsQueue<X>& q )
	{	o << '[' << gspc;
		for ( int i=0, s=q.size(); i<s; i++ ) o << q.cget(i) << gspc;
		return o << ']';
	}
};

//============================== end of file ===============================

#endif // GS_QUEUE_H
