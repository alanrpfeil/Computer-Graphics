/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_table.h>
# include <sig/gs_string.h>
# include <sig/gs_vars.h>
# include <sig/gs_dirs.h>

# include <sigogl/ui_style.h>
# include <sigogl/ws_run.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_resources.h>
# include <sigogl/gl_shader.h>
# include <sigogl/gl_program.h>
# include <sigogl/gl_texture.h>
# include <sigogl/gl_font.h>
# include <sigogl/gl_loader.h>

# include <stdarg.h>

//# define GS_USE_TRACE1 // basic trace
//# define GS_USE_TRACE2 // configuration file
//# define GS_USE_TRACE3 // default shaders
//# define GS_USE_TRACE4 // fonts
# include <sig/gs_trace.h>

# define ShaderTableSize  20
# define ProgramTableSize 15
# define TextureTableSize 18
# define FontTableSize	8

//================================== static data =========================================

# include "gl_predef_shaders.inc"

//# define TEST_SHADERS					// Macro to pre-compile shaders and list loaded resources

static gscbool FreeDeclInfo=1;			// Will free declarion information once a resource is loaded
static gscbool LoadPredefShaders=0;		// When 0 it will only load predefined shaders when shader files not found
static gsbyte NumPredefShadersLoaded=0;	// How many shaders came from gl_predef_shaders.inc
static gsbyte NumSigShadersDeclared=0;	// How many sig shaders were defined by declare_default_shaders()

static GsTablePt<GlShader> ShaderTable;
static GsTablePt<GlProgram> ProgramTable;
static GsTablePt<GlTexture> TextureTable;
static GsTablePt<GlFont> FontTable;
static GsVars Vars;
static GsDirs Dirs;

/*	ImprNote: possible extensions to be implemented:
	- void load_and_compile_all_resources (); // force load to be sure all resources can be found and have no errors
	- void add_resources_directory ( const char *dir ); 
	- void set_log_file ( const char* filename );
	- have keyword in cfg file to force LoadPredefShaders to 1
	- complete/revise configuration file definitions
*/

//================================== static objects =========================================

class GlShaderDecl
{ public :
	GLenum type; GsCharPt name; GsCharPt filename; const char* predef;
	GlShaderDecl ( GLenum t, const char* n, const char* fn, const char* pd )
	{ type=t; name=n; filename=fn; predef=pd; }
};

struct GlUniformDecl
{ int location; const char* unifname; void init () { unifname=0; } }; // uniform names must be static strings

class GlProgramDecl
{ public:
	GsCharPt name; GsArray<GlShader*> shaders; GsArray<GlUniformDecl> uniforms;
	GlProgramDecl ( const char* n )
	{ name=n; }
};

class GlTextureDecl
{ public:
	GsCharPt name; GsCharPt filename; GsImage* image;
	GlTextureDecl ( const char* n, const char* fn )
	{ name=n; filename=fn; image=0; }
	GlTextureDecl ( const char* n, GsImage* i )
	{ name=n; image=i; image->ref(); }
};

class GlFontDecl
{ public:
	GsCharPt name; GsCharPt filename; GlTexture* texture;
	GlFontDecl ( const char* n, const char* fn )
	{ name=n; filename=fn; texture=0; }
};

static void Error ( const char* msg, const char* sn )
{ 
	gsout.fatal ( "Resources Error: %s [%s].", msg, sn );
}

static void _load_and_compile ( GsDirs* d, GlShader* s, GlShaderDecl* sd )
{
	if ( !sd ) Error("Null","GlShaderDecl");
	GS_TRACE1 ( "Loading and compiling ["<<sd->filename<<"]" );
	gscbool loadpredef=LoadPredefShaders;

	if ( !loadpredef || !sd->predef )
	{	GsString fname(sd->filename);
		const char* error=0;
		if ( !d->checkfull(fname) ) error="Could not find ";
		else if ( !s->load(sd->type,fname) ) error="Could not load ";
		if ( error )
		{	gsout<<error<<fname<<"! Switching to predefined shader.\n";
			loadpredef=1;
		}
	}

	else//if ( loadpredef )
	{	if ( !sd->predef ) Error("Missing predefined shader",sd->name);
		s->set(sd->type,sd->predef);
		NumPredefShadersLoaded++;
	}

	if ( !s->compile() ) Error("could not compile",sd->filename);
}

//================================== public functions =========================================

// === Shaders ===

const GlShader* GlResources::declare_shader ( GLenum type, const char* shadername, const char* filename, const char* predef )
{
	if ( ShaderTable.hashsize()==0 ) ShaderTable.init(ShaderTableSize);
	GlShader* s = ShaderTable.insert(shadername); // cannot use filename because a same file may be used for multiple shaders
	if ( !s ) Error("shader name conflict",shadername);
	s->_decl = new GlShaderDecl ( type, shadername, filename, predef );
	return s;
}

const GlShader* GlResources::get_shader ( const char* shadername )
{
	GS_TRACE1 ( "get_shader ["<<shadername<<"]" );
	GlShader* s = ShaderTable.lookup ( shadername );
	if ( !s ) Error("shader not declared",shadername);
	return s;
}

// === Programs ===

const GlProgram* GlResources::declare_program ( const char* progname, int nshaders, const GlShader* shader1, const GlShader* shader2, ... )
{
	if ( ProgramTable.hashsize()==0 ) ProgramTable.init(ProgramTableSize);

	if ( nshaders<2 ) Error ("invalid number of shaders in program",progname);

	GlProgram* p = ProgramTable.insert ( progname );
	if ( !p ) Error("program name conflict",progname);
	GlProgramDecl* i = new GlProgramDecl ( progname );
	p->_decl = i;

	i->shaders.capacity(nshaders);
	i->shaders.size(0);
	i->shaders.push() = (GlShader*)shader1;
	i->shaders.push() = (GlShader*)shader2;

	if ( nshaders==2 ) return p;

	va_list args;
	va_start ( args, shader2 );
	while ( i->shaders.size()<nshaders )
		i->shaders.push() = va_arg ( args, GlShader* ); 
	va_end ( args );

	return p;
}

void GlResources::declare_uniform ( const GlProgram* p, int location, const char* unifname )
{
	if ( !p->_decl ) Error("program not found in uniform declaration",unifname );
	GsArray<GlUniformDecl>& u = p->_decl->uniforms;
	if ( location!=u.size() ) Error("invalid uniform location",unifname);
	u.push().init(); // need init to call unifname constructor
	u.top().location = location;
	u.top().unifname = unifname;
}

void GlResources::compile_program ( const GlProgram* prog )
{
	GS_TRACE1 ( "compile_program" );
	GlProgramDecl* pi = prog->_decl;
	if ( !pi || prog->linked() ) return; // already linked
	GlProgram* p = (GlProgram*) prog;

	for ( int i=0; i<pi->shaders.size(); i++ )
	{	GlShader* s = pi->shaders[i];
		if ( !s->compiled() && s->_decl )
		{	_load_and_compile(&Dirs,s,s->_decl); if (FreeDeclInfo) {delete s->_decl; s->_decl=0;} }
		p->attach ( s );
	}

	if ( !p->link() ) Error("program could not be linked",pi->name);

	if ( pi->uniforms.size()>0 )
	{	GsArray<GlUniformDecl>& u = pi->uniforms;
		p->uniform_locations(u.size());
		for ( int i=0; i<u.size(); i++ )
		{	if ( u[i].location>=u.size() ) Error("uniform location too large",pi->name);
			GS_TRACE1 ( "Getting uniform location ["<<u[i].unifname<<"]" );
			p->uniform_location(u[i].location,u[i].unifname);
			u[i].init(); // free string, if it is dynamically allocated
		}
		u.capacity(0); // free array
	}
   
	if ( FreeDeclInfo ) { delete pi; p->_decl=0; }
}

void GlResources::compile_programs ()
{
	GS_TRACE1 ( "compile_programs" );
	GsTablePt<GlProgram>&tp = ProgramTable;
	for ( int i=0; i<tp.size(); i++ )
	{	if ( tp.key(i) ) compile_program(tp.data(i));
	}
}

const GlProgram* GlResources::get_program ( const char* progname )
{
	GS_TRACE1 ( "get_program ["<<progname<<"]" );
	GlProgram* p = ProgramTable.lookup ( progname );
	if ( !p ) Error("program not found",progname);
	compile_program ( p );
	return p;
}

int GlResources::program_table_size ()
{
	return ProgramTable.size();
}

// === Textures ===

int GlResources::declare_texture ( const char* txname, const char* filename )
{
	if ( TextureTable.hashsize()==0 ) TextureTable.init(TextureTableSize);
	GlTexture* t = TextureTable.insert(txname);
	if ( t )
	{	t->_decl = new GlTextureDecl(txname,filename); }
	else
	{	int lid = TextureTable.lastid();
		if ( lid<0 ) Error("invalid texture name",txname); // null string or some other error
		GlTexture* prevt = TextureTable.data(lid);
		if ( prevt->_decl )
		{	if ( gs_compare(prevt->_decl->filename,filename)!=0 ) // different file, cannot share
			 Error("texture name conflict",txname);
		}
	}
	return TextureTable.lastid();
}

int GlResources::declare_texture ( const char* txname, GsImage* img )
{
	if ( TextureTable.hashsize()==0 ) TextureTable.init(TextureTableSize);
	GlTexture* t = TextureTable.insert(txname);
	if ( t )
	{	t->_decl = new GlTextureDecl(txname,img); }
	else
	{	int lid = TextureTable.lastid();
		if ( lid<0 )
			Error("invalid texture name",txname); // null string or some other error
		else
			Error("texture name conflict",txname);
	}
	return TextureTable.lastid();
}

int GlResources::get_texture_id ( const char* txname )
{
	GS_TRACE1 ( "get_texture_id ["<<txname<<"]" );
	return TextureTable.lookup_index(txname);
}

const GlTexture* GlResources::get_texture ( int txid )
{
	GS_TRACE1 ( "get_texture ["<<txid<<"]" );
	GlTexture* t = TextureTable.data(txid);
	if ( !t ) Error("get_texture()","texture index not found");
	if ( !t->valid() )
	{	GlTextureDecl* td = t->_decl;
		if ( !td ) Error("texture declarion null",TextureTable.key(txid));
		GsString fname;
		GsImage* img;
		if ( td->image )
		{	img = td->image;
			img->ref();
			fname = "(imagedata)";
		}
		else
		{	img = new GsImage;
			img->ref();
			fname = td->filename;
			if ( !Dirs.checkfull(fname) ) Error("texture file not found",td->filename);
			if ( !img->load(fname) ) Error("error loading texture file",fname);
			img->vertical_mirror(); // needed because OpenGL loads pixel data upside-down
		}
		GS_TRACE1 ( "texture ["<<fname<<"] size:"<<img->w()<<'x'<<img->h() );
		if ( !gl_loaded() ) Error("get_texture called before OpenGL loaded",fname);
		t->data ( img, GlTexture::Filtered );
		img->unref();
		if ( FreeDeclInfo ) { delete td; t->_decl=0; }
	}
	return t;
}

const GlTexture* GlResources::get_texture ( const char* txname )
{
	GS_TRACE1 ( "get_texture ["<<txname<<"]" );
	int id = TextureTable.lookup_index(txname);
	if ( id<0 ) Error("texture name not found",txname);
	return get_texture ( id );
}

// === Fonts ===

const GlFont* GlResources::declare_font ( const char* fontname, const char* fntfile, const char* imgfile )
{
	if ( FontTable.hashsize()==0 ) FontTable.init(FontTableSize);

	GlFont* f = FontTable.insert ( fontname );
	if ( !f ) Error("font name conflict",fontname);
	GlFontDecl* fd = new GlFontDecl ( fontname, fntfile );
	f->_decl = fd;
	fd->texture = TextureTable.data( declare_texture ( fontname, imgfile ) );

	return f;
}

const GlFont* GlResources::get_font ( int fontid )
{
	GS_TRACE1 ( "get_font ["<<fontid<<"]" );
   
	static int lfid = -99;
	static const GlFont* lfnt = 0;
	if ( fontid==lfid ) return lfnt; // fast return when there is no font change

	if ( fontid<0 ) // return default font
	{	lfnt = GlFont::default_font(); 
		if ( !lfnt->texture() && gl_loaded()>0 ) // send texture to OpenGL
		{	GS_TRACE4 ( "sending default texture to ogl...");
			GlFont::init_default_font_texture(); 
		}
		if ( lfnt->texture() ) lfid=fontid; 
		return lfnt; 
	}

	if ( FontTable.size()==0 ) Error("get_font()","no font declared!");

	GlFont* f = FontTable.data(fontid);
	if ( !f ) Error("get_font()","font id not valid");
	if ( !f->texture() ) // font is not ready for use while texture not ready
	{	GlFontDecl* fd = f->_decl;
		if ( !fd ) Error("font declarion null",FontTable.key(fontid));
		GS_TRACE4 ( "get_font: initializing texture ["<<(fd->texture->_decl->name)<<"]...");
		if ( !f->loaded() ) // load font file
		{	GsString fname(fd->filename.pt);
			if ( !Dirs.checkfull(fname) ) Error("font file not found",fd->filename);
			GS_TRACE4 ( "get_font: loading texture ["<<fname<<"]...");
			f->load ( fname );
		}
		if ( gl_loaded()>0 ) // load and send texture to OpenGL
		{	GS_TRACE4 ( "get_font: sending texture ["<<(fd->texture->_decl->name)<<"] to ogl...");
			f->texture ( fd->texture->_decl? get_texture(fd->texture->_decl->name) : fd->texture );
			if ( FreeDeclInfo ) { delete fd; f->_decl=0; }
		}
	}
	if ( f->texture() ) { lfid=fontid; lfnt=f; }
	return f;
}

const GlFont* GlResources::get_font ( const char* fontname )
{
	GS_TRACE1 ( "get_font ["<<fontname<<"]" );
	int ind = FontTable.lookup_index ( fontname );
	if ( ind<0 ) Error("font not found",fontname); // perhaps instead of error return default font
	return get_font ( ind );
}

int GlResources::has_font ( const char* fontname )
{
	GS_TRACE1 ( "has_font ["<<fontname<<"]" );
	return FontTable.lookup_index ( fontname );
}

const GsFont* GlResources::get_gsfont ( const GsFontStyle& fs )
{ 
	// This is called all the time by the UI to get access to font metrics
	GS_TRACE1 ( "get_gsfont ["<<fs.fontid<<"]" );
	return fs.fontid<0? GlFont::default_font() : get_font ( fs.fontid );
}

int GlResources::get_font_id ( const char* fontname )
{
	GS_TRACE1 ( "get_font_id ["<<fontname<<"]" );
	return FontTable.lookup_index ( fontname );
}

// === Configuration ===

GsDirs& GlResources::dirs ()
{
	return Dirs;
}

// default file names used:
# define CFGCUSTOM  "sig1st.cfg"
# define DEFCFGFILE "sig.cfg"

void GlResources::load_configuration_file ()
{
	// Load only once:
	if ( Dirs.size()>0 ) return; // configuration already loaded
	GS_TRACE2 ( "Loading configuration..." );

	// Get executable filename:
	GsString filename;
	bool execfilename = false;
	if ( ws_program_argv() ) // make sure filename is properly available
	{	filename=ws_program_argv()[0];
		execfilename = true;
	}
	GS_TRACE2 ( "Executable name is ["<<filename<<']' );

	// Search for base folder and config file:
	// 1. If file CFGCUSTOM exists, base folder and (optionally) file name are read from it
	// 2. If not, 1st file name considered will be executable name with ".cfg" as extension
	// 3. If executable name ends with suffix "32" or "64" followed by 2 or 3 characters, the suffix is removed
	// 4. If the config file is not found, will try fixed name DEFCFGFILE
	// (if changed keep this description the same in file CFGCUSTOM, and in the function declaration)

	// Search for base folder (step 1):
	GsInput in;
	in.commentchar('#');
	bool customloaded=false;
	if ( in.open(CFGCUSTOM) ) // read custom base folder
	{	GS_TRACE2 ( "Found "<<CFGCUSTOM );
		in.gets();
		if ( in.ltoken().len() ) // ok there is a base folder defined
		{	customloaded = true;
			Dirs.basedir(in.ltoken());
			in.get();
			if ( in.ltype()==GsInput::String ) { filename=in.ltoken(); execfilename=false; }
		}
	}
	if ( !customloaded )
	{	GS_TRACE2 ( "Basedir will come from: "<<filename ); // base folder is the executable folder
		Dirs.basedir_from_filename ( filename );
	} 
	GS_TRACE2 ( "Basedir ["<<Dirs.basedir()<<']' );
	if ( execfilename ) // process name from executable (step 2)
	{	remove_path(filename);
		remove_extension(filename); // config file name set same as executable name
		char s[3]; s[2]=0; int l=filename.len();
		if ( l>4 ) // check and remove "32--" or "64--" sufixes (step 3)
		{	s[0]=filename[l-4]; s[1]=filename[l-3];
			if ( gs_comparecs(s,"32")==0 || gs_comparecs(s,"64")==0 ) filename.len(l-4);
		}
		if ( l>5 && l==filename.len() ) // check and remove "32---" or "64---" sufixes (step 3)
		{	s[0]=filename[l-5]; s[1]=filename[l-4];
			if ( gs_comparecs(s,"32")==0 || gs_comparecs(s,"64")==0 ) filename.len(l-5);
		}
		filename.append(".cfg");
	}

	if ( !Dirs.open(in,filename) ) // Config file name not found
	{	if ( !Dirs.open(in,DEFCFGFILE) ) // Try the default fixed name (step 4)
		{	// useful when debuging: gsout.fatal("Could not find [%s] or [%s]!",filename,DEFCFGFILE);
			LoadPredefShaders=1; // At this point we use in-memory pre-defined shaders and font
		}
	}

	if ( in.valid() ) // config file can be loaded
	{	// process configuration:
		GS_TRACE2("Loading configuration...");
		in >> Vars;
		in.close();
	}

	// add folders:
	GsVar* v;
	if ( (v=Vars.get("folders")) )
	{	GS_TRACE2 ( "folders" );
		for ( int i=0; i<v->size(); i++ )
		{	Dirs.push ( v->gets(i) );
		}
	}

	// declare fonts:
	if ( (v=Vars.get("fonts")) )
	{	GS_TRACE2 ( "fonts" );
		GsString fntfile, pngfile;
		for ( int i=0; i<v->size(); i++ )
		{	fntfile=v->gets(i); fntfile<<".fnt";
			pngfile=v->gets(i); pngfile<<".png";
			declare_font ( v->gets(i), fntfile, pngfile );
		}
	}

	// declare textures for later use:
	if ( (v=Vars.get("textures")) )
	{
	}

	// customize default ui which is statically defined:
	if ( (v=Vars.get("uistyle")) )
	{	GS_TRACE2 ( "uistyle" );
		// add here style selection
	}

	if ( (v=Vars.get("uifont")) )
	{	GS_TRACE2 ( "uifont" );
		int fid = get_font_id (v->gets());
		UiStyle::Current().set_font_id(fid);
		UiStyle::Dialog().set_font_id(fid);
	}

	if ( (v=Vars.get("uifonth")) )
	{	GS_TRACE2 ( "uifonth" );
		int h = v->geti();
		UiStyle::Current().set_font_h(h);
		UiStyle::Dialog().set_font_h(h);
	}

	GS_TRACE2 ( "Configuration loaded.\n" );
}

GsVar* GlResources::configuration ( const char* varname )
{ 
	return Vars.get(varname); 
}

// === Default Shaders ===

void GlResources::declare_default_shaders ()
{
	GS_TRACE3 ( "Declare default shaders called" );

	// Declare shaders only once:
	if ( NumSigShadersDeclared>0 ) return;
	int inielems = ShaderTable.elements();

	GS_TRACE3 ( "Declaring default shaders..." );

	// Load configuration, if not already loaded:
	load_configuration_file ();

	// Declare shaders:
	GlResources r; // use this instance just as a namespace access
  
	const GlShader* vs2dcolored = r.declare_shader ( GL_VERTEX_SHADER, "vs2dcolored", "2dcolored.vert", pds_2dcolored_vert );
	const GlShader* vs2dcoloredsc= r.declare_shader( GL_VERTEX_SHADER, "vs2dcoloredsc", "2dcoloredsc.vert", pds_2dcoloredsc_vert );
	const GlShader* vs2dsmooth  = r.declare_shader ( GL_VERTEX_SHADER, "vs2dsmooth", "2dsmooth.vert", pds_2dsmooth_vert );
	const GlShader* vs3dsmooth  = r.declare_shader ( GL_VERTEX_SHADER, "vs3dsmooth", "3dsmooth.vert", pds_3dsmooth_vert );
	const GlShader* vs3dsmoothsc= r.declare_shader ( GL_VERTEX_SHADER, "vs3dsmoothsc", "3dsmoothsc.vert", pds_3dsmoothsc_vert );
	const GlShader* vs3dflat	= r.declare_shader ( GL_VERTEX_SHADER, "vs3dflat", "3dflat.vert", pds_3dflat_vert );
	const GlShader* vs3dgouraud = r.declare_shader ( GL_VERTEX_SHADER, "vs3dgouraud", "3dgouraud.vert", pds_3dgouraud_vert );
	const GlShader* vs3dphong   = r.declare_shader ( GL_VERTEX_SHADER, "vs3dphong", "3dphong.vert", pds_3dphong_vert );
	const GlShader* vs3dphongmc = r.declare_shader ( GL_VERTEX_SHADER, "vs3dphongmc", "3dphongmc.vert", pds_3dphongmc_vert );
	const GlShader* vs3dtextured= r.declare_shader ( GL_VERTEX_SHADER, "vsv3dtextured", "3dtextured.vert", pds_3dtextured_vert );
	const GlShader* fs3dtextured= r.declare_shader ( GL_FRAGMENT_SHADER, "fs3dtextured", "3dtextured.frag", pds_3dtextured_frag );
	const GlShader* fsflat		= r.declare_shader ( GL_FRAGMENT_SHADER, "fsflat", "flat.frag", pds_flat_frag );
	const GlShader* fsgouraud	= r.declare_shader ( GL_FRAGMENT_SHADER, "fsgouraud", "gouraud.frag", pds_gouraud_frag );
	const GlShader* fsphong		= r.declare_shader ( GL_FRAGMENT_SHADER, "fsphong", "phong.frag", pds_phong_frag );
	const GlShader* fsphongmc	= r.declare_shader ( GL_FRAGMENT_SHADER, "fsphongmc", "phongmc.frag", pds_phongmc_frag );
	const GlShader* vshadefunc	= r.declare_shader ( GL_VERTEX_SHADER, "vshadefunc", "shadefunc.glsl", pds_shadefunc_glsl );
	const GlShader* fshadefunc	= r.declare_shader ( GL_FRAGMENT_SHADER, "fshadefunc", "shadefunc.glsl", pds_shadefunc_glsl );

	const GlProgram* p;
	p = r.declare_program ( "2dcolored", 2, vs2dcolored, fsflat );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "zCoord" );

	p = r.declare_program ( "2dcoloredsc", 2, vs2dcoloredsc, fsflat );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "zCoord" );
	r.declare_uniform ( p, 3, "vColor" );

	p = r.declare_program ( "2dsmooth", 2, vs2dsmooth, fsgouraud );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "zCoord" );

	p = r.declare_program ( "3dsmooth", 2, vs3dsmooth, fsgouraud );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );

	p = r.declare_program ( "3dsmoothsc", 2, vs3dsmoothsc, fsgouraud );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "vColor" );

	p = r.declare_program ( "3dflat", 3, vs3dflat, vshadefunc, fsflat );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "lPos" );
	r.declare_uniform ( p, 3, "lInt" );
	r.declare_uniform ( p, 4, "lNum" );
	r.declare_uniform ( p, 5, "mColors" );
	r.declare_uniform ( p, 6, "mParams" );

	p = r.declare_program ( "3dgouraud", 3, vs3dgouraud, vshadefunc, fsgouraud );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "lPos" );
	r.declare_uniform ( p, 3, "lInt" );
	r.declare_uniform ( p, 4, "lNum" );
	r.declare_uniform ( p, 5, "mColors" );
	r.declare_uniform ( p, 6, "mParams" );

	p = r.declare_program ( "3dtextured", 3, vs3dtextured, fs3dtextured, fshadefunc );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "lPos" );
	r.declare_uniform ( p, 3, "lInt" );
	r.declare_uniform ( p, 4, "lNum" );
	r.declare_uniform ( p, 5, "mColors" );
	r.declare_uniform ( p, 6, "mParams" );
	r.declare_uniform ( p, 7, "Mode" ); // LighDev: if uniforms are added before, these ids need update in glr_model.cpp
	r.declare_uniform ( p, 8, "TexId" );

	p = r.declare_program ( "3dphong", 3, vs3dphong, fsphong, fshadefunc );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "lPos" );
	r.declare_uniform ( p, 3, "lInt" );
	r.declare_uniform ( p, 4, "lNum" );
	r.declare_uniform ( p, 5, "mColors" );
	r.declare_uniform ( p, 6, "mParams" );

	p = r.declare_program ( "3dphongmc", 3, vs3dphongmc, fsphongmc, fshadefunc );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "lPos" );
	r.declare_uniform ( p, 3, "lInt" );
	r.declare_uniform ( p, 4, "lNum" );
	r.declare_uniform ( p, 5, "mColors" );
	r.declare_uniform ( p, 6, "mParams" );

	p = r.declare_program ( "dftext", 2,
		r.declare_shader ( GL_VERTEX_SHADER,   "vsdftext", "dftext.vert", pds_dftext_vert ),
		r.declare_shader ( GL_FRAGMENT_SHADER, "fsdftext", "dftext.frag", pds_dftext_frag ) );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "zCoord" );
	r.declare_uniform ( p, 3, "TexId" );

	p = r.declare_program ( "2dtextured", 2, 
		r.declare_shader ( GL_VERTEX_SHADER,   "vs2dtextured", "2dtextured.vert", pds_2dtextured_vert ),
		r.declare_shader ( GL_FRAGMENT_SHADER, "fs2dtextured", "2dtextured.frag", pds_2dtextured_frag ) );
	r.declare_uniform ( p, 0, "vProj" );
	r.declare_uniform ( p, 1, "vView" );
	r.declare_uniform ( p, 2, "zCoord" );
	r.declare_uniform ( p, 3, "Mode" );
	r.declare_uniform ( p, 4, "TexId" );

	NumSigShadersDeclared = ShaderTable.elements()-inielems;

	// Usefull methods to test/debug:
	#ifdef TEST_SHADERS
	compile_programs();
	print_report();
	#endif

	GS_TRACE3 ( "Done.\n" );
}

// === Report ===

static void outentries ( int n ) { if ( n>3 ) gsout<<"Entries: "<<n<<gsnl; }

void GlResources::print_report ()
{
	int i, n;

	gsout<<"\nDefault config files: "<<CFGCUSTOM<<gspc<<DEFCFGFILE<<gsnl;
	gsout<<"Pre-defined shaders loaded from executable: "<<NumPredefShadersLoaded<<gsnl;

	GsTablePt<GlShader>& ts = ShaderTable;
	gsout<<"\nShader Table - collisions:"<<ts.collisions()<< ", longest:"<<ts.longest_entry()<<gsnl;
	for ( i=0,n=0; i<ts.size(); i++ )
	{	if ( !ts.key(i) ) continue;
		gsout<<i<<": "<<ts.key(i)<<": ";
		GlShader* s = ts.data(i);
		if ( !s->compiled() ) gsout<<"not ";
		gsout<<"compiled";
		if ( !s->_decl ) gsout << ".\n"; else gsout << ", file:" << s->_decl->filename << gsnl;
		n++;
	}
	outentries(n);

	GsTablePt<GlProgram>& tp = ProgramTable;
	gsout<<"\nProgram Table - collisions:"<<tp.collisions()<< ", longest:"<<tp.longest_entry()<<gsnl;
	for ( i=0,n=0; i<tp.size(); i++ )
	{	if ( !tp.key(i) ) continue;
		gsout<<i<<": "<<tp.key(i)<<gspc;
		GlProgram* p = tp.data(i);
		GlProgramDecl* pi = tp.data(i)->_decl;
		gsout<<" glid="<<p->id<<": ";
		if ( !p->linked() ) { gsout<<"not "; } gsout<<"linked";
		if ( !pi ) 
		{	gsout << ".\n"; }
		else
		{	gsout << ", uniforms:"<<pi->uniforms.size();
			gsout << ", shaders: ";
			for ( int s=0; s<pi->shaders.size(); s++ )
			{	gsout << (pi->shaders[s]->_decl? pi->shaders[s]->_decl->name.pt:"compiled") << gspc; }
			gsout << gsnl;
		}
		n++;
	}
	outentries(n);

	GsTablePt<GlTexture>& tt = TextureTable;
	gsout<<"\nTexture Table - collisions:"<<tt.collisions()<< ", longest:"<<tt.longest_entry()<<gsnl;
	for ( i=0,n=0; i<tt.size(); i++ )
	{	if ( !tt.key(i) ) continue;
		gsout<<i<<": "<<tt.key(i)<<": ";
		GlTexture* t = tt.data(i);
		if ( !t->valid() ) { gsout<<"not "; } gsout<<"loaded";
		if ( !t->_decl ) gsout << ".\n"; else gsout << gspc << t->_decl->filename << gsnl;
		n++;
	}
	outentries(n);

	GsTablePt<GlFont>& tf = FontTable;
	gsout<<"\nFont Table - collisions:"<<tf.collisions()<< ", longest:"<<tf.longest_entry()<<gsnl;
	for ( i=0,n=0; i<tf.size(); i++ )
	{	if ( !tf.key(i) ) continue;
		gsout<<i<<": "<<tf.key(i)<<": ";
		GlFont* f = tf.data(i);//->res;
		GlFontDecl* d = (GlFontDecl*)tf.data(i)->_decl;
		if ( !f->characters() ) { gsout<<"not "; } gsout<<"loaded";
		if ( !d ) gsout << ".\n"; else gsout << gspc << d->filename << gsnl;
		n++;
	}
	outentries(n);

	gsout<<"\nLoaded Configuration:\n"<<Vars;

	gsout << gsnl;
}

//================================ End of File ========================================
