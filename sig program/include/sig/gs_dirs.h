/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_DIRS
# define GS_DIRS

/** \file gs_dirs.h 
 * array of file paths */

# include <sig/gs_string.h>
# include <sig/gs_strings.h>

//============================== GsDirs ===============================

/*! \class GsDirs gs_dirs.h
	\brief array of directories
	This class is based on GsStrings and maintains a list of directories for
	searching files that may be in one of the declared absolute or relative
	directories, in relation to a given base directory. */
class GsDirs : public GsStrings
 { private:
	GsString _basedir; // the basedir of relative filenames (has to have a slash in the end)
	
   public:
	/*! Access to the base dir */
	const GsString& basedir () const { return _basedir; }
	
	/*! Set the basedir and ensures its validity using validate_path(). 
		Returns true if a valid dir was set. Otherwise false is returne and basedir is set to an empty string. */
	bool basedir ( const char* bd );
	
	/*! Set the basedir, extracting it from a filename and ensuring its validity.
		Returns true if a valid dir was set. Otherwise false is returne and basedir is set to an empty string. */
	bool basedir_from_filename ( const char* fname );

	/*! Push the path to the string array and validate it using validate_path().
		If the path is not valid (eg null), or is already in the array, nothing
		is done and -1 is returned. Case sensitive comparison is used.
		In case of success the position of the added path is returned. */
	int push ( const char* path );

	/*! Tries to open an input file:
		If the given filename has an absolute path in it, the method simply
		tries to open it and returns right after. Otherwise:
		- First search in the paths stored in the array.
		  If basedir exists (not null), relative paths in the array
		  are made relative (concatenated) to basedir.
		- Second, try to open filename using only the basedir path.
		- Finally tries to simply open filename.
		Returns true if the file could be open. In such case, the successfull full file
		name can be found in inp.filename(). False is returned in case of failure. */
	bool open ( GsInput& inp, const char* fname );

	/*! Checks if file exists by testing all declared paths using the rules in open().
		Returns false if the file was not found, otherwise returns true */ 
	bool check ( const char* fname );

	/*! Checks if file exists by testing all declared paths using the rules in open().
		Returns false if the file was not found, otherwise fname will contain the 
		succesfull full file name and true is returned */ 
	bool checkfull ( GsString& fname );
};

#endif // GS_DIRS

//============================== end of file ===============================
