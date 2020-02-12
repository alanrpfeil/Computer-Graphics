/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# ifndef GL_TYPES_H
# define GL_TYPES_H

/** \file gl_types.h 
 * Header for specifying glcorearb types without loading the full original header.
 */

#ifndef GL_VERSION_1_0
typedef void GLvoid;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
#endif

#ifndef GL_VERSION_1_1
typedef float GLclampf;
typedef double GLclampd;
#endif

//================================= End of File ===============================

# endif // GL_TYPES_H
