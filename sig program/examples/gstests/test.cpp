/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>

# include <sig/gs_array.h>
# include <sig/gs_string.h>
# include <sig/gs_input.h>
# include <sig/gs_mat.h>

extern void test_types ();
extern void test_random ();
extern void test_mat ();
extern void test_matn ();
extern void test_euler ();
extern void test_vars ();
extern void test_heap ();
extern void test_table ();
extern void test_slotmap ();
extern void test_string ();
extern void test_structures ();
extern void test_timer ();
extern void test_array ();
extern void test_arraylist ();
extern void test_graph ();
extern void test_grid ();
extern void test_list ();

struct FuncDesc { void (*func) (); const char* name; } FD[] =
{	{ test_types,	"types" },
	{ test_random,	"random" },
	{ test_timer,	"timer" },
	{ test_string,	"string" },
	{ test_vars,	"vars" },
	{ test_mat,		"mat" },
	{ test_matn,	"matn" },
	{ test_euler,	"euler" },
	{ test_grid,	"grid" },
	{ test_array,	"array" },
	{ test_graph,	"graph" },
	{ test_list,	"list" },
	{ test_heap,	"heap" },
	{ test_table,	"table" },
	{ test_slotmap, "slotmap" },
	{ test_structures, "structures" },
	{ test_arraylist, "arraylist" },
	{ 0, 0 } };

int main ( int argc, char** argv )
{
	int i;
	const char* test = 0; //test="types";
	GsString s(test);

	if ( !test )
	{	for ( i=0; FD[i].name; i++ ) gsout<<gspc<<FD[i].name<<gsnl;
		gsout << "\nEnter test name: ";
		s.input();
	}

	if ( s.len()==0 ) return 0;

	for ( i=0; FD[i].name; i++ )
		if (s==FD[i].name) { FD[i].func(); break; }

	if ( !FD[i].name ) { gsout<<"unknown test!\n"; return 0; }

	gsout.pause ( "\n<Press Enter to Exit>" );

	return 0;
}
