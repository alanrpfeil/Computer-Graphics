/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>
# include <string.h>
# include <ctype.h>

# include <sig/gs_input.h>
# include <sig/gs_string.h>

//# define GS_USE_TRACE1 //Parser
//# define GS_USE_TRACE2 //Init/Open
# include <sig/gs_trace.h>

# define ISINVALID	_type==(gsbyte)TypeInvalid
# define ISFILE		_type==(gsbyte)TypeFile
# define ISSTRING	_type==(gsbyte)TypeString
# define CHKDATA	if ( !_data ) _data = new Data
# define INITDATA	if ( _data ) _data->init()

//=============================== GsInput =================================

struct GsInput::Data
{	GsArray<char> ungetstack; // unget buffer of chars
	GsString	  ltoken;	 // buffer with the last token read
	gsbyte		  ltype;	  // last token type read
	void init () { ungetstack.size(0); ltoken.set(""); ltype=(gsbyte)End; }
};

void GsInput::_init ( char c )
{
	_data = 0;
	_cur.f = 0;
	_curline = 1;
	_comchar = c;
	_type = (gsbyte) TypeInvalid;
	_lowercase = 0; // false
	_lnumreal = 0; // false
	_maxtoksize = 128;
	_filename = 0;
}

GsInput::GsInput ( char com )
{
	GS_TRACE2 ("Default Constructor");
	_init ( com );
}

GsInput::GsInput ( FILE *file, char com )
{ 
	GS_TRACE2 ("File Constructor");
	_init ( com );
	if ( file )
	{	_cur.f = file;
		_type = (gsbyte) TypeFile;
	}
}

GsInput::GsInput ( const char* filename, const char* mode, char com )
{ 
	GS_TRACE2 ("File Constructor");
	_init ( com );
	FILE* file = fopen(filename,mode);
	if ( file )
	{	_cur.f = file;
		_type = (gsbyte) TypeFile;
	}
}

GsInput::~GsInput ()
{
	close (); // close frees _filename
	delete _data;
}

void GsInput::init ( const char *buff ) 
{
	GS_TRACE2 ("Init string");
	close ();
	if ( buff )
	{	_cur.s = buff;
		_type = (gsbyte) TypeString;
	}
}

void GsInput::init ( FILE *file ) 
{
	GS_TRACE2 ("Open file");
	close ();
	if ( file )
	{	_cur.f = file;
		_type = (gsbyte) TypeFile;
	}
}

bool GsInput::open ( const char* filename, char m )
{
	GS_TRACE2 ("Open filename: "<<filename);
	FILE* file = fopen ( filename, m=='b'? "rb":"r" );
	init ( file );
	if ( file ) gs_string_set ( _filename, filename );
	return file? true:false;
}

void GsInput::close ()
{
	INITDATA;
	if ( ISFILE ) fclose ( _cur.f );
	_curline = 1;
	_type = (gsbyte) TypeInvalid;
	gs_string_set ( _filename, 0 );
}

void GsInput::abandon ()
{ 
	_type = (gsbyte) TypeInvalid;
	close ();   
}

bool GsInput::end ()
{
	if ( _data ) { if (_data->ungetstack.size()>0) return false; }

	if ( ISFILE ) return feof(_cur.f)? true:false;

	if ( ISSTRING ) return *(_cur.s)? false:true;

	return true;
}

int GsInput::readchar () // comments not handled, unget yes
{
	int c = -1;

	CHKDATA;

	if ( _data->ungetstack.size()>0 ) 
	{	c = _data->ungetstack.pop();
	}
	else
	{	if ( ISFILE ) { c=fgetc(_cur.f); }
		else if ( ISSTRING ) c = *_cur.s? *_cur.s++ : -1;
	}

	if ( c=='\n' ) _curline++;

	if ( _lowercase && c>='A' && c<='Z' ) c = c-'A'+'a';

	return c;
}

int GsInput::readline ( GsString& buf )
{
	int c;

	char st[2]; st[1]=0;
	buf.len(0);
	do { c = readchar();
		 if ( c==EOF ) break;
		 st[0] = (char)c;
		 buf.append(st);
	   } while ( c!='\n' );

	return c;
}

void GsInput::readall ( GsArray<gsbyte>& buf )
{
	if ( ISFILE )
	{
		long start = ftell(_cur.f);
		fseek ( _cur.f, 0, SEEK_END );
		int size = (int)(ftell(_cur.f)-start);
		fseek ( _cur.f, start, SEEK_SET );
		buf.size ( size );
		int rs = (int)fread ( (void*)(&buf[0]), sizeof(char), (size_t)size, _cur.f );
		if ( rs!=size ) buf.size ( size ); // if _cur.f was open as a text file adjust len to address CRNL convertions
	}
	else if ( ISSTRING )
	{
		int len = (int)strlen(_cur.s);
		buf.sizecap ( len, len+1 );
		strcpy ( (char*)&buf[0], _cur.s );
		_cur.s += buf.size();
	}
}

void GsInput::readall ( GsString& buf )
{
	if ( ISFILE )
	{
		long start = ftell(_cur.f);
		fseek ( _cur.f, 0, SEEK_END );
		int size = (int)(ftell(_cur.f)-start);
		fseek ( _cur.f, start, SEEK_SET );
		buf.len ( size ); // will add 0 at the end
		size = (int)fread ( (void*)(&buf[0]), sizeof(char), (size_t)size, _cur.f );
		buf.len ( size ); // if _cur.f was open as a text file adjust len to address CRNL convertions
	}
	else if ( ISSTRING )
	{
		buf.set ( _cur.s );
		_cur.s += buf.len();
	}
}

void GsInput::skipline ()
{
	int c;
	do { c = readchar();
	   } while ( c!=EOF && c!='\n' );
}

GsInput::TokenType GsInput::check ()
{ 
	// skip white spaces ang get 1st char:
	int c;
	do
	{	c = readchar();
		if ( c==_comchar ) do { c=readchar(); } while ( c!=EOF && c!='\n' );
		if ( c=='\n' ) _curline++;
		if ( c==EOF ) return End;
	} while ( isspace(c) );

	// check if delimiter preceeding number:
	if ( (c=='.'||c=='+'||c=='-') && isdigit(_peekbyte()) ) { unget(c); return Number; }

	// check other cases:
	unget(c);
	if ( isdigit(c) ) return Number;
	if ( isalpha(c) || c=='"' || c=='_' ) return String;
	return Delimiter;
}

static char getescape ( char c )
{
	switch ( c )
	{	case 'n' : return '\n';
		case 't' : return '\t';
		case '\n': return '\\';
		default  : return c;
	}
}

GsInput::TokenType GsInput::get ()
{
	return get ( check() );
}

GsInput::TokenType GsInput::get ( TokenType type )
{
	_data->ltype = type;
	_data->ltoken.len(0);

	if ( type==End )
	{
		GS_TRACE1 ( "Got End..." );
	}
	else if ( type==String )
	{
		GsString &s = _data->ltoken;
		s.len ( _maxtoksize );
		int i, c = readchar();
		if ( c=='"' ) // inside quotes mode
		{	GS_TRACE1 ( "Got String between quotes..." );
			for ( i=0; i<_maxtoksize; i++ )
			{	c = readchar();
				if ( c=='\\' ) c=getescape(readchar());
				if ( c==EOF || c=='"' ) break;
				s[i]=c;
			}
		}
		else // normal mode
		{	GS_TRACE1 ( "Got String..." );
			for ( i=0; i<_maxtoksize; i++ )
			{	if ( c==EOF ) break;
				if ( !isalnum(c) && c!='_' ) { unget(c); break; }
				s[i] = c;
				c = readchar();
			}
		}
		s.len(i);
	}
	else if ( type==Number )
	{	GS_TRACE1 ( "Got Number..." );
		GsString& s = _data->ltoken;
		s.len ( _maxtoksize );
		bool pnt=false, exp=false;
		int i, c = readchar();
		s[0]=c; // we know the 1st is part of a number (can be +/-)
		for ( i=1; i<_maxtoksize; i++ )
		{	c = readchar();
			if ( !isdigit(c) )
			{	if ( c=='e' ) c='E';
				if ( !pnt && c=='.' ) pnt=true;
				else if ( pnt && c=='.' ) break;
				else if ( !exp && c=='E' ) exp=pnt=true;
				else if ( (c=='+'||c=='-') && s[i-1]=='E' ); // ok
				else { unget(c); break; }
			}
			s[i]=c; 
		}
		s.len(i);
		_lnumreal = pnt? 1:0;
	}
	else // Delimiter
	{	GS_TRACE1 ( "Got Delimiter..." );
		_data->ltoken.len (1);
		_data->ltoken[0] = readchar();
	} 

	GS_TRACE1 ( "Token: "<<_data->ltoken );
	return type;
}

char GsInput::getc ()
 {
   get();
   return _data->ltoken[0];
 }

const GsString& GsInput::gets ()
 {
   get();
   return _data->ltoken;
 }

const GsString& GsInput::getfilename ()
 {
   GsString fname = gets();
   if ( getc()=='.' ) fname << '.' << gets(); else unget(); 
   _data->ltoken = fname;
   return _data->ltoken;
 }

int GsInput::geti ()
 {
   return get()==Number? _data->ltoken.atoi():0;
 }

long GsInput::getl()
{
   return get()==Number? _data->ltoken.atol():0; 
}

float GsInput::getf ()
 {
   return get()==Number? _data->ltoken.atof():0;
 }

const GsString& GsInput::ltoken() const
 {
   CHKDATA;
   return _data->ltoken;
 }

GsInput::TokenType GsInput::ltype() const
 { 
   CHKDATA;
   return (TokenType)_data->ltype;
 }

void GsInput::unget ()
 {
   if ( !_data ) return;
   unget ( gspc );
   unget ( _data->ltoken );
 }

void GsInput::unget ( const char* s )
 {
   if ( !s ) return;
   for ( int i=(int)strlen(s)-1; i>=0; i-- ) unget ( s[i] );
 }

void GsInput::unget ( char c )
 {
   CHKDATA;
   _data->ungetstack.push() = c;
   if ( c=='\n' ) _curline--;
 }

bool GsInput::skip ( int n )
 {
   while ( n-- )
	{ if ( get()==End ) return false;
	}
   return true;
 }

bool GsInput::skipto ( const char *name, TokenType type )
{
	while ( get()!=End )
	{	if ( _data->ltype==(gsbyte)type )
		{	if ( _data->ltoken==name ) return true; }
	}
	return false;
}

//============================= operators ==================================

GsInput& operator>> ( GsInput& in, GsString& s ) 
 { 
   s = in.gets();
   return in;
 }

GsInput& operator>> ( GsInput& in, char* st ) 
 { 
   strcpy ( st, in.gets() );
   return in; 
 }

//============================ End of File =================================


