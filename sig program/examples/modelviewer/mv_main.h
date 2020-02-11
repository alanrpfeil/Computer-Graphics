/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/
 
# ifndef MV_MAIN_H
# define MV_MAIN_H

class MvScene;
class MvWindow;

struct MvApp
{	MvWindow* window;
	MvScene* scene;
	MvApp ();
};

extern MvApp* App;

# endif // MV_MAIN_H
