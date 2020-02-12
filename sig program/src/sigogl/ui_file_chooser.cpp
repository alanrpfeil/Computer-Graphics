/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_scandir.h>

# include <sigogl/ui_file_chooser.h>

//# define GS_USE_TRACE1 // file scanning
//# define GS_USE_TRACE2 // input callback
# include <sig/gs_trace.h>

//================================== UiFileChooser ====================================

static void format ( GsString& flist, GsStrings& dirs, GsStrings& files )
{
	GsString s;
	int i;
	flist.set("");
	for ( i=0; i<dirs.size(); i++ )
	{	s=dirs[i]; remove_path(s); flist<<s<<"/ ";
	}
	if (i>0) flist<<gsnl;
	for ( i=0; i<files.size(); i++ )
	{	s=files[i]; remove_path(s); flist<<s<<gspc;
	}
}

static void read_files ( const char* fstring, GsString& flist, const GsString& curext )
{
	GS_TRACE1 ( "Reading ["<<fstring<<"]" );
	if ( !fstring ) { flist.set(""); return; }

	GsString path ( fstring );
	GsString fname;
	extract_filename ( path, fname );
	if ( !validate_path(path) ) path="./";
	GS_TRACE1 ( "Path:["<<path<<"] Filename:["<<fname<<"]" );

	GsStrings dirs, files, ext;
	if ( curext.len()>0 )
	{	GS_TRACE1 ( "Extension:["<<curext<<"]" );
		ext.push ( curext );
	}
	gs_scandir ( path, dirs, files, ext );
	format ( flist, dirs, files );
}

static void inputcb ( UiInput* inp, const GsEvent& e, char delc, void* udata )
{
	GS_TRACE2 ( "Callback: "<<e.key<<gspc<<delc );

	UiFileChooser* fc = (UiFileChooser*)udata;
	if ( e.character=='/' || delc=='/' )
	{	fc->update_file_list ( inp->value() );
	}

	const char* ext=get_extension ( inp->value() );
	if ( !ext )
	{	if ( fc->curext().len()>0 ) fc->update_file_list ( inp->value() );
	}
	else if ( gs_comparecs(ext,fc->curext())!=0 )
	{	fc->update_file_list ( inp->value() );
	}
}

UiFileChooser::UiFileChooser ( const char* fname, int ev, int x, int y, int mw, int mh )
			  :UiPanel ( 0, Vertical, Top, x, y, mw, mh )
{
	_inp = new UiInput(">",ev,0,0,mw,0,fname); // ImprNote: event not used, should declare callback to retrive events
	_inp->nextspc ( 8 );
	_inp->label().left();
	_inp->callback ( inputcb, this );

	_out = new UiOutput(NULL,0,0,mw,mh); // null label
	_out->label().left();
	_out->color().bg=_inp->color().bg;
	_out->word_wrap ( true );

	update_file_list ( fname );

	UiPanel::add ( _inp );
	UiPanel::add ( _out );
	disable_dragging();
}

void UiFileChooser::update_file_list ( const char* fstring )
{
	get_extension ( fstring, _curext );
	read_files ( fstring, _out->text(), _curext );
}

int UiFileChooser::handle ( const GsEvent& e, UiManager* uim )
{
	return UiPanel::handle(e,uim);
}

//================================ EOF =================================================
