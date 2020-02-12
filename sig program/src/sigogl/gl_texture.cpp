/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sigogl/gl_core.h>
# include <sigogl/gl_texture.h>

# include <sig/gs_image.h>

//=================================== GlTexture ====================================

GlTexture::GlTexture ()
{
	id = 0;
	width = height = 0;
	_decl = 0;
}

GlTexture::~GlTexture ()
{
	// id 0 means texture not generated
	if ( id>0 ) glDeleteTextures ( 1, &id );
}

void GlTexture::init ()
{
	if ( id>0 ) glDeleteTextures ( 1, &id );
	id = 0;
	width = height = 0;
}

void GlTexture::data ( const GsImage* img, Settings s )
{
	if ( id==0 ) glGenTextures ( 1, &id );
	glBindTexture ( GL_TEXTURE_2D, id ); 

	width = img->w();
	height = img->h();

	// Parameters: ( target, level, internalFormat, width, height, border, format, type, data )
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, img->w(), img->h(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img->cdata() );

	// ImprNote: starting at 4.5 glTextureParameter() should replace glTexParameter(),
	//			 here could test which function version was loaded and call the correct one.

	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // If the u,v coordinates overflow the range 0,1 the image is repeated
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if ( s==Filtered )
	{	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // linear takes average of nearby texels
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else if ( s==MipMapped )
	{	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST); 
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); 
		glGenerateMipmap (GL_TEXTURE_2D);
	}

	glBindTexture ( GL_TEXTURE_2D, 0 );
}

void GlTexture::data ( const GsBytemap* bmp, Settings s )
{
	if ( id==0 ) glGenTextures ( 1, &id );

	glBindTexture ( GL_TEXTURE_2D, id ); 

	width = bmp->w();
	height = bmp->h();

	// Parameters: ( target, level, internalFormat, width, height, border, format, type, data )
	glTexImage2D ( GL_TEXTURE_2D, 0, GL_RED, bmp->w(), bmp->h(), 0, GL_RED, GL_UNSIGNED_BYTE, bmp->cdata() );

	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // If the u,v coordinates overflow the range 0,1 the image is repeated
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if ( s==Filtered )
	{	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // linear takes average of nearby texels
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else if ( s==MipMapped )
	{	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST); 
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); 
		glGenerateMipmap (GL_TEXTURE_2D);
	}

	glBindTexture ( GL_TEXTURE_2D, 0 );
}
