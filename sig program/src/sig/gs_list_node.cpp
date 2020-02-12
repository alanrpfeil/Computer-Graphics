/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_list_node.h>

//=============================== GsListNode ================================

GsListNode *GsListNode::remove_next ()
 { 
   GsListNode *n = _next;  
   _next = _next->_next; 
   _next->_next->_prior = this;
   return n;
 }

GsListNode *GsListNode::remove_prior ()
 { 
   GsListNode *n = _prior; 
   _prior = _prior->_prior; 
   _prior->_prior->_next = this;
   return n;
 }

GsListNode *GsListNode::remove ()
 { 
   _next->_prior = _prior; 
   _prior->_next = _next; 
   return this; 
 }

GsListNode *GsListNode::replace ( GsListNode *n )
 { 
   _next->_prior = n;
   _prior->_next = n;
   n->_next  = _next;
   n->_prior = _prior;
   return this;
 } 

GsListNode *GsListNode::insert_next ( GsListNode *n )
 { 
   n->_next = _next;
   n->_next->_prior = n;
   n->_prior = this;
   _next = n;
   return n;
 } 

GsListNode *GsListNode::insert_prior ( GsListNode *n )
 { 
   n->_prior = _prior;
   _prior->_next = n;
   n->_next = this;
   _prior = n;
   return n;
 } 

void GsListNode::splice ( GsListNode *n ) 
 { 
   _next->_prior = n;
   n->_next->_prior = this;
   GsListNode *nxt=_next;
   _next = n->_next;
   n->_next = nxt;

/* This is the same as:
   GsListNode *tmp;
   GS_SWAP ( _next->_prior, n->_next->_prior );
   GS_SWAP ( _next,		 n->_next		 ); */
 }

void GsListNode::swap ( GsListNode *n )
 { 
   GsListNode *nn=n->_next, *p=_prior;

   if ( _next!=n ) { _prior=n->_prior; n->_next=_next; } else { _prior=n; n->_next=this;  }
   if ( nn!=this ) { _next=nn;		 n->_prior=p;	} else { _next=n;  n->_prior=this; }

   _prior->_next=this;
   _next->_prior=this;
   n->_prior->_next=n;
   n->_next->_prior=n;
 }

//============================== end of file ===============================

