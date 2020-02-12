/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef SA_EVENT_H
# define SA_EVENT_H

/** \file sa_event.h 
 * propagates events
 */

# include <sig/sa_action.h>
# include <sig/gs_event.h>

/*! \class SaEvent sa_event.h
	\brief propagates events in a scene

	sends an event to the scene graph */
class SaEvent : public SaAction
 { protected :
	GsEvent _ev;
	int _result;
	struct Hit { GsEvent ev; SnEditor* ed; float t; };
	GsArray<Hit> _hits;

   public :
	SaEvent ( const GsEvent& e ) { _ev=e; _result=0; }
	GsEvent& get () { return _ev; }
	int result () const { return _result; }
	void apply ( SnNode* n );

   protected : 
	virtual bool editor_apply ( SnEditor* ed ) override;
};

//================================ End of File =================================================

# endif  // SA_EVENT_H

