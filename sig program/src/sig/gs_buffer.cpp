/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <stdlib.h>
# include <string.h>

# include <sig/gs_buffer.h>

//=== GsBuffer =======================================================================

# define BUF(i) ((char*)buffer)+(sizeofx*(i))

void* gs_buffer_size ( void* buffer, int sizeofx, int& size, int newsize )
 { 
   if ( size==newsize ) return buffer;
   size = newsize;
   if ( size==0 )
	{ if (buffer) { free(buffer); buffer=0; } }
   else
	{ buffer = realloc ( buffer, (size_t)(sizeofx*size) ); }
   return buffer;
 }

void* gs_buffer_insert ( void* buffer, int sizeofx, int& size, int i, int dp )
 { 
   size += dp;
   buffer = realloc ( buffer, (size_t)(sizeofx*size) );
   if ( i<size-dp )
	 memmove ( BUF(i+dp), BUF(i), sizeofx*(size-dp-i) ); // ok with overlap
   return buffer; 
 }

void* gs_buffer_remove ( void* buffer, int sizeofx, int& size, int i, int dp )
 { 
   /* void *memmove( void *dest, const void *gsc, size_t count );
	  If some regions of the source area and the destination overlap,
	  memmove ensures that the original source bytes in the overlapping
	  region are copied before being overwritten. */
   if ( i<size-dp ) memmove ( BUF(i), BUF(i+dp), sizeofx*(size-(i+dp)) );
   return gs_buffer_size ( buffer, sizeofx, size, size-dp );
 }

void* gs_buffer_copy ( void* buffer, int sizeofx, int& size, const void* buffertocp, int sizetocp )
 { 
   if ( buffer==buffertocp ) return buffer;
   buffer = gs_buffer_size ( buffer, sizeofx, size, sizetocp );
   /* void *memcpy( void *dest, const void *gsc, size_t count );
	  If the source and destination overlap, memcpy function does not
	  ensure that the original source bytes in the overlapping region
	  are copied before being overwritten. Use memmove to handle
	  overlapping regions. */
   if ( buffer ) memcpy ( buffer, buffertocp, sizeofx*size ); // no overlap
   return buffer;
 }

//=== End of File =====================================================================




