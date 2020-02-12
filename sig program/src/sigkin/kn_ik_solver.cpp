/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigkin/kn_ik_solver.h>

//=============================== KnIkSolver ===============================

KnIkSolver::KnIkSolver ()
{
	orbitsearch=true;
	_coldet=0;
}

KnIk::Result KnIkSolver::solve ()
{
	if ( orbitsearch )
		return KnIk::solve(goal,osearch,_coldet);
	else
		return KnIk::solve(goal,osearch.init,_coldet);
}

//======================================= EOF =====================================

