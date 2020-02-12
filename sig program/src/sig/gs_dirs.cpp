/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_dirs.h>

//# define GS_USE_TRACE1 // open 
# include <sig/gs_trace.h>

//====================== GsDirs ==========================

bool GsDirs::basedir ( const char* path )
{
	_basedir = path;
	bool b = validate_path ( _basedir );
	if ( !b ) _basedir="";
	return b;
}

bool GsDirs::basedir_from_filename ( const char* fname )
{
	_basedir = fname;
	int c = remove_filename ( _basedir );
	if ( c<=1 ) // no dir in fname
	{ _basedir=""; return false; }
   else
	{ validate_path ( _basedir ); return true; }
}

int GsDirs::push ( const char* path )
{
	// validate path:
	GsString spath(path);
	if ( !validate_path(spath) ) return -1;

	// check if already there:
	for ( int i=0; i<size(); i++ )
	{ if ( spath==get(i) ) return -1; 
	}
	
	// ok, push it:
	GsStrings::push ( spath );
	return size()-1;
}

bool GsDirs::open ( GsInput& inp, const char* fname )
{
	// If absolute path, just try to open it:
	if ( gs_absolute(fname) )
	{	inp.open(fname);
		return inp.valid();
	}
 
	// Otherwise search in the paths:
	GsString fullfile;

	// first search in the declared paths:
	for ( int i=0; i<size(); i++ )
	{	fullfile = "";
		if ( _basedir && !gs_absolute(get(i)) ) fullfile << _basedir;
		fullfile << get(i);
		fullfile << fname;
		GS_TRACE1("Trying: ["<<fullfile<<']');
		inp.open ( fullfile );
		if ( inp.valid() ) return true; // found
	}
	
	// try only in basedir path:
	if ( _basedir.len() )
	{	fullfile = _basedir;
		fullfile << fname;
		GS_TRACE1("Trying: ["<<fullfile<<']');
		inp.open ( fullfile );
	}
	
	// try only filename, ie in cur folder:
	if ( !inp.valid() )
	{	GS_TRACE1("Trying: ["<<fname<<']');
		inp.open ( fname );
	}
   
	// return:
	return inp.valid();
}

bool GsDirs::check ( const char* fname )
{
	GsInput in;
	return open(in,fname);
}

bool GsDirs::checkfull ( GsString& fname )
{
	GsInput in;
	if ( open(in,fname) )
	{	fname = in.filename();
		return true;
	}
	else
	{	return false;
	}
}

//=========================== EOF ===============================
