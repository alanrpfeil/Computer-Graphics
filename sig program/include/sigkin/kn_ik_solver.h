/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef KN_IK_SOLVER_H
# define KN_IK_SOLVER_H

# include <sigkin/kn_ik.h>
# include <sigkin/kn_coldet.h>

/*! KnIkSolver provides a generic interface for IK solvers, and
	ans as standalone it directly calls methods of KnIk */
class KnIkSolver : public KnIk
{  public:
	GsMat goal;
	KnIkOrbitSearch osearch;
	bool orbitsearch;

   protected:
	KnColdet* _coldet;

   public:
	KnIkSolver ();
	void coldet ( KnColdet* cd ) { updrefs(_coldet,cd); }
	void fixed_orbit ( float oang ) { orbitsearch=false; osearch.init=oang; }
	void search_orbit () { orbitsearch=true; }
	virtual bool standardik () { return true; }
	virtual bool init ( KnIk::Type t, KnJoint* end ) { return KnIk::init(t,end); }
	virtual void solve_rot_goal ( bool b ) { KnIk::solve_rot_goal(b); }
	virtual void set_local () { KnIk::set_local(goal); }
	virtual KnIk::Result solve ();
	virtual void apply_values ( const float values[7] ) { KnIk::apply_values(values); }
	virtual void apply_last_result () { KnIk::apply_last_result(); }
};

//======================================= EOF =====================================

# endif // KN_IK_SOLVER_H
