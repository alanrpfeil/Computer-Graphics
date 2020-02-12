/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GS_SCANDIR_H
# define GS_SCANDIR_H

# include <sig/gs_strings.h>

// ========================== scandir functions ============================

/*! Scans directory dir, and pushes all found directories and files in
	dirs and files. Strings in ext may contain file extensions to
	filter the files (e.g "cpp" "txt", etc).
	String dir must finish with a slash '/' character.
	File names are returned in their full path names. */
void gs_scandir ( const char* dir, GsStrings& dirs, GsStrings& files, const GsStrings& ext );

/*! Scan filenames in dir, optionally filtered by extension ext (if given) */
void gs_scandir ( const char* dir, GsStrings& files, const char* ext=0 );

/*! Recursively scans and pushes in files all filenames starting at basedir.
	String array ext may contain file extensions to filter the read
	files; extensions are set without the "dot" character.
	File names are returned in their full path names. */
void gs_scanfiles ( const char* basedir, GsStrings& files, const GsStrings& ext );

//============================== end of file ===============================

# endif // GS_SCANDIR_H

