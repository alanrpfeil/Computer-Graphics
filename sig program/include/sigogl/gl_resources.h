/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_RESOURCES_H
# define GL_RESOURCES_H

/** \file gl_resources.h 
 * Global shared graphics resources, such as shaders, programs and textures.
 */

# include <sigogl/gl_types.h>

class GsVar;
class GsDirs;
class GlProgram;
class GlShader;
class GlTexture;
class GsImage;
class GsFontStyle;
class GsFont;
class GlFont;

//========================== GlResources =======================

/*! GlResources centralizes functionality to load resources from files. 
	All its members are static */
class GlResources
{  public :
	GlResources () {}
   ~GlResources () {}

   public : // Shaders

	/*! Declares the type and location of a shader program and returns its id location in the resources tables */
	static const GlShader* declare_shader ( GLenum type, const char* shadername, const char* filename, const char* predef=0 );

	/*! Returns a previously declared shader, possibly not yet compiled. */
	static const GlShader* get_shader ( const char* shadername );

   public : // Programs

	/*! Declares a program to be built with the given shaders. The number of shaders nshaders must be
		at least 2. If it is >2, additional shader pointers are to be given as additional arguments. */
	static const GlProgram* declare_program ( const char* progname, int nshaders, const GlShader* shader1, const GlShader* shader2, ... );

	/*! Declares a uniform location for the given program. The location indices declared over multiple calls
		have to be continous and start at 0 */
	static void declare_uniform ( const GlProgram* p, int location, const char* unifname );

	/*! Loads and compiles associated shaders, then links the given program. The program has to be
		previously declared as a resource. An error will exit the program with a message sent to the console. */
	static void compile_program ( const GlProgram* prog );

	/*! Loads and compiles (with compile_program()) all programs declared as resources.
		This function will be automatically called when the first window opens. */
	static void compile_programs ();

	/*! Returns a previously declared program. When the program is first accessed, all its
		declared shaders will be loaded, compiled and the program linked. 
		The returned program will always be ready to be used. */
	static const GlProgram* get_program ( const char* progname );

	/*! Returns the number of entries in the program table (some may be null) */
	static int program_table_size ();

   public : // Textures

	/*! Declares the name and location of a texture file to be later used */
	static int declare_texture ( const char* texname, const char* filename );

	/*! Declares the name and data of a texture file to be later used. Pointer img will be shared using ref()/unref() methods. */
	static int declare_texture ( const char* texname, GsImage* img );

	/*! Get the resources id of the texture, or -1 if it has not been declared. */
	static int get_texture_id ( const char* txname );

	/*! Access the texture from its id, loading it and sending it to OpenGL on first access. */
	static const GlTexture* get_texture ( int txid );

	/*! Access the texture, loading it and sending it to OpenGL on first access. */
	static const GlTexture* get_texture ( const char* txname );

   public : // Fonts

	/*! Declares the name and location of a .fnt font file to be later used */
	static const GlFont* declare_font ( const char* fontname, const char* filename, const char* imgfile );

	/*! Access a declared font, loading it on first access, and sending its texture to OpenGL 
		on first access after OpenGL has been initialized. */
	static const GlFont* get_font ( int fontid );

	/*! Access a declared font from its name. */
	static const GlFont* get_font ( const char* fontname );

	/*! Test if a font has been declared, returning the font ir or -1 if not. */
	static int has_font ( const char* fontname );

	/*! Access a declared font as a GsFont in order to access its font metrics.
		The returned font will be a stroke font if fd.fontid<0, otherwise it will be a GlFont. */
	static const GsFont* get_gsfont ( const GsFontStyle& fs );

	/*! Get the resources id of the font, or -1 if it has not been declared. */
	static int get_font_id ( const char* fontname );

   public : // Configuration

	/*! Returns the directory list used to search for resources */
	static GsDirs& dirs ();

	/*! Load the configuration file. Only the first call to this function will
		actually try to load the configuration file, and this function is
		automatically called when the first WsWindow is created.
		The configuration file determination procedure is:
		1. If "sig1st.cfg" exists, the base folder and file name are read from it
		2. If not, base folder and file name come from executable, with ".cfg" extension
		3. If executable name ends with suffix "32" or "64" followed by 2 or 3 characters, the suffix is removed
		4. If no config file is found, will finally try fixed name "sig.cfg" */
	static void load_configuration_file ();

	/*! Get access to the parameters loaded from the configuration file.
		Returns null if varname is not found. */
	static GsVar* configuration ( const char* varname );

	/*! Declares all the default shaders.
		This is automatically called when the first window is opened. */
	static void declare_default_shaders ();

	/*! Prints to gsout all declared objects and loaded parameters. 
		Some of the reported parameters require the OpenGL context to be active. */
	static void print_report ();
};

//================================= End of File ===============================

# endif // GL_RESOURCES_H
