/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <string.h>
# include <sig/sn_planar_objects.h>
# include <sigogl/ui_label.h>
# include <sigogl/ui_manager.h>
# include <sigogl/gl_resources.h>
# include <sigogl/gl_texture.h>
# include <sigogl/gl_font.h>

//================================== UiLabel =========================================

UiLabel::UiLabel ()
		:_fs ( UiStyle::Current().font.label )
{
	_accel = -1;
	_x = _y = 0;
	_w = _h = 0;
	_align = Left;
}

UiLabel::~UiLabel ()
{
}

//UiKeyDev:
// -search_fkey() should become "search_cmdkey" and search for all combinations, for ex "Ctrl+A" etc
// -formatting should align cmd keys independently from label
// -the other option is to declare using GsEvent enums
// -could use: 	int search ( const char* st, int i=0 ) const;
	/*! Returns the index of the first occurrence of str, or -1 if not found.
		The search starts at index i, which has default value of 0.
		In case of empty strings or st larger than GsString, -1 is returned.
		Case-insensitve comparison is performed. */
// -all Alt, Ctrl, Shift combinations will be declared here:
//  void add_cmd_key ( int key, int cmd, gscbool alt=0, gscbool ctrl=0, gscbool shift=0 );

int UiLabel::search_fkey () const
{
	int i, len=(int)strlen(_text);
	for ( i=0; i<len; i++ )
	{	if ( _text[i]=='F' && _text[i+1]>='0' && _text[i+1]<='9') break;
	}
	if ( i==len ) return 0;
	int n = _text[i+1]-'0';
	int n2 = _text[i+2];
	if ( n2>='0' && n2<='9' ) n=(n*10)+(n2-'0');
	return n;
}

float UiLabel::base () const 
{
	const GsFont* f = GlResources::get_gsfont ( _fs );
	return f->top()*f->hscaling(_fs); 
}

# define SORTEXT(s) (const char*)(s? (const char*)s:(const char*)_text)

void UiLabel::accelrange ( float& a, float& b, const char* st ) const
{
	if ( _accel<0 ) { a=b=0; return; }
	const GsFont* f = GlResources::get_gsfont ( _fs );
	GsString s ( SORTEXT(st) );
	s[_accel+1]=0;
	GsVec2 v = f->text_size ( _fs, s );
	a = 0;
	b = v.x;
	if ( _accel>0 )
	{	s[_accel]=0;
		v = f->text_size ( _fs, s );
		a = v.x;
	}
}

void UiLabel::set_size ( const char* s )
{
	const GsFont* f = GlResources::get_gsfont ( _fs );
	GsVec2 size = f->text_size ( _fs, s );
	_w = int(size.x+0.5f);
	_h = int(size.y+0.5f);
}

void UiLabel::set ( const char* l, bool chkaccel )
{
	_accel = -1;
	if ( !l || !l[0] ) { _text.set(0); set_size(" "); return; }
	if ( !chkaccel ) { _text.set(l); set_size(_text); return; }

	int i, len=(int)strlen(l);
	if ( len>16 ) len=16; // do not search long labels fully
	for ( i=0; i<len; i++ )
	{	if ( l[i]=='&' ) { _accel=i; break; }
	}

	if ( _accel<0 )
	{	_text.set ( l );
	}
	else
	{	GsString s(l);
		s.remove ( i, 1 );
		_text.set ( s );
	}
	set_size ( _text );
}

void UiLabel::size ( int s )
{
	if ( s<=0 ) { _text.set(0); return; }
	_text.renew ( s+1 );
}

void UiLabel::copy_spacing ( const UiLabel& l )
{
	_x = l._x;
	_y = l._y;
	_w = l._w;
	_h = l._h;
}

void UiLabel::draw ( SnPlanarObjects* pobs, float x, float y, GsColor c, float* xmax, const char* s ) const
{
	const GlFont* f = GlResources::get_font(_fs.fontid);
	pobs->start_group ( SnPlanarObjects::Masked, f->texture()->id );
	f->output ( _fs, SORTEXT(s), x+_x, y+_y, c, pobs, 0, xmax, 'w', _accel );
}

GsOutput& operator<< ( GsOutput& o, const UiLabel& l )
{
	o << "Label \""<<l._text<<"\":"<<
	" fontid:"<<l._fs.fontid<<
	" rect:" << GsRect(l._x,l._y,l._w,l._h) <<
	" accel:" << l._accel <<
	" align:" << l._align <<
	gsnl;
	return o;
}

//================================ End of File =================================================
