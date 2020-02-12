/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution.
  =======================================================================*/

# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# include <sig/gs.h>
# include <sig/gs_string.h>
# include <sig/gs_scandir.h>

# ifdef GS_MSWIN
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
# include <sys/timeb.h>
# else
# include <unistd.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <dirent.h>
# endif

//================================ scandir ============================================

# ifdef GS_MSWIN

struct dirent {
	long		   d_ino; // 0
	unsigned short d_reclen; // 0
	unsigned short d_namlen; // len of name in d_name
	char		   d_name[FILENAME_MAX];
};

// namelist: array of pointer to dir entries, number is returned
static int scandir(const char *dirname,  dirent ***namelist,
	int (*select)(dirent *),
	int (*compar)(dirent **, dirent **))
{
	char *d;
	WIN32_FIND_DATA find;
	HANDLE h;
	int nDir = 0, NDir = 0;
	struct dirent **dir = 0, *selectDir;
	unsigned long ret;
	char findIn[MAX_PATH*4];

	strcpy ( findIn, dirname );
	d = findIn+strlen(findIn);
	if (d==findIn) *d++ = '.';
	if (*(d-1)!='/' && *(d-1)!='\\') *d++ = '/';
	*d++ = '*';
	*d++ = 0;

	if ( (h=FindFirstFile(findIn, &find))==INVALID_HANDLE_VALUE )
	{
		ret = GetLastError();
		if ( ret!=ERROR_NO_MORE_FILES ) {} // todo: return some error code
		*namelist = dir;
		return nDir;
	}

	do
	{	selectDir=(struct dirent*)malloc(sizeof(struct dirent));
		strcpy(selectDir->d_name, find.cFileName);
		if (!select || (*select)(selectDir))
		{	if (nDir==NDir)
			{	struct dirent **tempDir = (struct dirent **)calloc(sizeof(struct dirent*), NDir+33);
				if (NDir) memcpy(tempDir, dir, sizeof(struct dirent*)*NDir);
				if (dir) free(dir);
				dir = tempDir;
				NDir += 32;
			}
			dir[nDir] = selectDir;
			nDir++;
			dir[nDir] = 0;
		}
		else
		{	free(selectDir);
		}
	} while (FindNextFile(h, &find));

	ret = GetLastError();
	if (ret != ERROR_NO_MORE_FILES) {} // todo: return some error code

	FindClose(h);

	if (compar) qsort ( dir, nDir, sizeof(*dir), (int(*)(const void*, const void*))compar );

	*namelist = dir;
	return nDir;
}

# endif // GS_MSWIN

//================================ scandir ============================================

static int _scandir ( const char *d, dirent ***list,
					  int (*sort)(dirent **, dirent **) )
{
	#ifdef GS_MSWIN
	int n = scandir(d, list, 0, sort);
	#else
	int n = scandir(d, list, 0, (int(*)(const dirent **, const dirent **))sort);
	#endif

	// Some other systems may need special type casts as the ones below:
	//int n = scandir(d, list, 0, (int(*)(const void*,const void*))sort);
	//int n = scandir(d, list, 0, (int(*)(const dirent **, const dirent **))sort);
	//int n = scandir(d, list, 0, (int(*)(dirent **, dirent **))sort);
	//int n = scandir(d, list, 0, (int(*)(void*, void*))sort);

	return n;
}

static void sMake ( GsString& s, const char* dir, const char* file )
{
	s = dir;
	if ( s.lchar()!='/' && s.lchar()!='\\' ) s.append ( "/" );
	s.append ( file );
}

void gs_scandir ( const char* dir, GsStrings& dirs, GsStrings& files, const GsStrings& ext )
{
	dirent **list;
	int n = _scandir ( dir, &list, 0 );
	int i;
	GsString s;

	for ( i=0; i<n; i++ )
	{	if ( list[i]->d_name[0]=='.' ) continue; // "./" or "../"
		sMake ( s, dir, list[i]->d_name );
		if ( gs_isdir(s) )
		{	dirs.push ( s ); }
		else // is file
		{	int j;
			for ( j=0; j<ext.size(); j++ )
			if ( has_extension(s,ext[j]) ) break;
			if ( ext.size()==0 || j<ext.size() ) files.push ( s );
		}
	}

	for ( i=0; i<n; i++ ) free ( (void*)(list[i]) );
	free((void*)list);
}

void gs_scandir ( const char* dir, GsStrings& files, const char* ext )
{
	dirent **list;
	int i, n=_scandir ( dir, &list, 0 );
	GsString s;

	for ( i=0; i<n; i++ )
	{	if ( list[i]->d_name[0]=='.' ) continue; // "./" or "../"
		sMake ( s, dir, list[i]->d_name );
		if ( gs_isdir(s) ) continue;
		if ( ext ) { if ( !has_extension(s,ext) ) continue; }
		files.push ( s );
	}

	for ( i=0; i<n; i++ ) free ( (void*)(list[i]) );
	free((void*)list);
}

void gs_scanfiles ( const char* basedir, GsStrings& files, const GsStrings& ext )
{
	GsStrings dirs;

	gs_scandir ( basedir, dirs, files, ext );

	GsString d;
	while ( dirs.size()>0 )
	{	d = dirs[0];
		if ( d.lchar()!='/' ) d.append("/");
		dirs.remove ( 0 );
		gs_scandir ( d, dirs, files, ext );
	}
}

/*USE THIS FOR TESTING:
# include <sig/gs_scandir.h>
GsStrings dirs, files;
GsStrings ext;
gs_scandir ( "C:/", dirs, files, ext );
int i;
for ( i=0; i<files.size(); i++ )
gsout<<files[i]<<gsnl;
gsout<<"+++++++++++++\n";
for ( i=0; i<dirs.size(); i++ )
gsout<<dirs[i]<<gsnl;
*/

//=== End of File =======================================================================
