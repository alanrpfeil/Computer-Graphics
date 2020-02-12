/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_list.h>

//============================ GsListBase =================================

GsListBase::GsListBase ( GsManagerBase* m )
 {
   _first = 0;
   _cur = 0;
   _man = m;
   _man->ref();
   _elements = 0;
 } 

GsListBase::GsListBase ( const GsListBase& l )
 {
   _first = 0;
   _cur = 0;
   _man = l._man;
   _man->ref();
   _elements = 0;
   insert_list ( l );
 }

GsListBase::GsListBase ( GsListNode* n, GsManagerBase* m )
 {
   _first = _cur = n;
   _man = m;
   _man->ref();
   _elements = 0;
 }

GsListBase::~GsListBase ()
 {
   init ();
   _man->unref();
 } 

void GsListBase::init ()
{
	while (_cur) remove();
}

void GsListBase::elements ( int e )
 {
   if ( e>=0 ) { _elements=e; return; }

   _elements = 0;
   if ( !_first ) return;
   _cur = _first;
   do { _elements++; 
		_cur = _cur->next(); 
	  } while ( _cur!=_first );
 }

void GsListBase::insert_next ( GsListNode *n ) 
 {
   if (!n) return;

   _elements++;
   if ( _cur )
	{ _cur->insert_next(n);
	  _cur = n;
	}
   else 
	{ n->init(); _first = _cur = n; }
 }

GsListNode* GsListBase::insert_next ()
 {
   insert_next ( (GsListNode*)_man->alloc() );
   return _cur;
 }

void GsListBase::insert_prior ( GsListNode* n )
 {
   if (!n) return;

   _elements++;
   if ( _cur )
	{ _cur->insert_prior(n);
	  _cur = n;
	}
   else 
	{ n->init(); _first = _cur = n; }
 }

GsListNode* GsListBase::insert_prior ()
 {
   insert_prior ( (GsListNode*)_man->alloc() );
   return _cur;
 }

void GsListBase::insert_list ( const GsListNode* l )
 { 
   if ( !l ) return;
 
   GsListBase nl (_man);
   const GsListNode* lcur = l;

   do { nl.insert_next ( (GsListNode*)_man->alloc(lcur) );
		lcur = lcur->next();
	  } while ( lcur!=l );

   if ( _cur )
	_cur->splice ( nl.last() ); // join the two lists
   else
	_cur = _first = nl._cur->next();

   nl.leave_data(); // so that elements are not deleted by nl destructor

   _elements += nl._elements;
 }

GsListNode *GsListBase::replace ( GsListNode *n )
 {
   if (!_cur) return 0;
   if ( _first==_cur ) _first=n;
   GsListNode *ret = _cur->replace(n);
   _cur = n;
   return ret;
 }

GsListNode *GsListBase::extract ()
 {
   if (!_cur) return 0;

   GsListNode *origcur = _cur;
   _elements--;

   if ( _first->next()!=_first )
	{ _cur = _cur->remove()->next();
	  if ( _first==origcur ) _first=_cur;
	}
   else _first = _cur = 0;

   return origcur;
 }

void GsListBase::remove ()
{
	void* pt = extract();
	if ( pt ) _man->free(pt);
}

void GsListBase::sort ()
 {
   _cur = _first;
   if ( !_cur || _cur->next()==_cur ) return;

   GsListNode *pos, *min;
   while ( _cur->next() != _first )
	{ min = pos = _cur;

	  do { _cur = _cur->next();
		   if ( _man->compare(_cur,min)<0 ) min=_cur;
		 } while ( _cur!=_first->prior() );

	  if (min!=pos)
	   { if (pos==_first) _first=min;
		 pos->swap(min);
	   }

	  _cur = min->next();
	}

   _cur = _first;
 }

int GsListBase::insort ( GsListNode *n )
 {
   int c=1;

   if ( _cur )
	{ _cur = _first;
	  do { c = _man->compare ( n, _cur );
		   if ( c<=0 ) break;
		   _cur = _cur->next();
		 } while ( _cur!=_first );
	}

   if ( _cur==_first && c<=0 ) _first=n;

   insert_prior(n);

   return c;
 }

bool GsListBase::search ( const GsListNode *n )
 {
   _cur = _first;
   if ( !_cur || !n ) return false;
   int c;

   do { c = _man->compare ( n, _cur );
		if ( c==0 ) return true;
		if ( c<0 ) return false;
		_cur = _cur->next();
	  } while ( _cur!=_first );

   return false;
 }

bool GsListBase::search_all ( const GsListNode *n )
 {
   _cur = _first;
   if ( !_cur || !n ) return false;

   do { if ( _man->compare(n,_cur)==0 ) return true;
		_cur = _cur->next();
	  } while ( _cur!=_first );

   return false;
 }

void GsListBase::take_data ( GsListBase& l )
 {
   if ( this == &l ) return;
   init ();
   _first	= l._first;	l._first	= 0;
   _cur	  = l._cur;	  l._cur	  = 0;
   _elements = l._elements; l._elements = 0;
 }

void GsListBase::take_data ( GsListNode* n )
 {
   if ( _cur==n || _first==n ) return;
   init ();
   _first = _cur = n;
 }

GsListNode* GsListBase::leave_data ()
 {
   GsListNode* n = _first;
   _first = _cur = 0;
   _elements = 0;
   return n;
 }

GsOutput& operator<< ( GsOutput& o, const GsListBase& l )
 { 
   o<<'['<<gspc;
   GsListIteratorBase it(l);
   for ( it.first(); it.inrange(); it.next() )
	{ l._man->output ( o, it.get() );
	  o << gspc;
	}
   return o<<']';
 }

//=========================== GsListIteratorBase ================================

GsListIteratorBase::GsListIteratorBase ( const GsListBase& l )
 { 
   _node = 0;
   _list = &l;
   reset ();
 }

GsListIteratorBase::GsListIteratorBase ( GsListNode* n )
 { 
   _list = 0;
   _node = n;
   reset ();
 }

void GsListIteratorBase::reset ()
 {
   if ( _list )
	{ _cur = _list->cur();
	  _first = _list->first();
	  _last = _list->last();
	}
   else
	{ _cur = _first = _last = _node;
	  if ( _first ) _last = _first->prior();
	}
   _rcode = 0;
 }

bool GsListIteratorBase::inrange ()
 {
   if ( _rcode==3 || !_first ) return false;

   if ( _rcode==1 ) { _rcode=2; if ( _first==_last ) _rcode=3; }
	else
   if ( _rcode==2 && (inlast()||infirst()) ) _rcode=3;

   return true;
 }

//============================== end of file ===============================
