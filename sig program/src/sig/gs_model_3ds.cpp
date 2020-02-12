/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

#include <stdio.h>
#include <stdlib.h>

# include <sig/gs_strings.h>
# include <sig/gs_model.h>

//# define GS_USE_TRACE1	// keyword tracking
# include <sig/gs_trace.h>

enum {
	CHUNK_RGBF	  = 0x0010,
	CHUNK_RGBB	  = 0x0011,
	CHUNK_PERCI	 = 0x0030,
	CHUNK_PERCF	 = 0x0031,
	CHUNK_MAIN	  = 0x4D4D,
	CHUNK_OBJMESH   = 0x3D3D,
	CHUNK_BKGCOLOR  = 0x1200,
	CHUNK_AMBCOLOR  = 0x2100,
	CHUNK_OBJBLOCK  = 0x4000,
	CHUNK_TRIMESH   = 0x4100,
	CHUNK_VERTLIST  = 0x4110,
	CHUNK_FACELIST  = 0x4120,
	CHUNK_FACEMAT   = 0x4130,
	CHUNK_MAPLIST   = 0x4140,
	CHUNK_SMOOLIST  = 0x4150,
	CHUNK_TRMATRIX  = 0x4160,
	CHUNK_LIGHT	 = 0x4600,
	CHUNK_SPOTLIGHT = 0x4610,
	CHUNK_CAMERA	= 0x4700,
	CHUNK_MATERIAL  = 0xAFFF,
	CHUNK_MATNAME   = 0xA000,
	CHUNK_AMBIENT   = 0xA010,
	CHUNK_DIFFUSE   = 0xA020,
	CHUNK_SPECULAR  = 0xA030,
	CHUNK_SHININESS = 0xA040,
	CHUNK_SHININESS1= 0xA041,
	CHUNK_SHININESS2= 0xA042,
	CHUNK_TEXTURE   = 0xA200,
	CHUNK_BUMPMAP   = 0xA230,
	CHUNK_MAPFILE   = 0xA300,
	CHUNK_KEYFRAMER = 0xB000,
	CHUNK_FRAMES	= 0xB008,
};

bool GsModel::load_3ds ( const char* fname )
 {
   init ();

   int i; //Index variable
	
   FILE *filept; //File pointer
	
   unsigned short chunk_id; //Chunk identifier
   unsigned int chunk_len; //Chunk lenght

   unsigned char uchar; //Char variable
   unsigned short qty; //Number of elements in each chunk

   if ((filept=fopen (fname, "rb"))== NULL) return 0; //Open the file

   long fstart = ftell(filept);
   fseek ( filept, 0, SEEK_END );
   long fsize = ftell(filept) - fstart + 1;
   fseek ( filept, fstart, SEEK_SET );

   while ( ftell(filept) < fsize ) //Loop to scan the whole file 
	{
		//getchar(); //Insert this command to debug (waits keypress for each chunck reading)

		if(fread (&chunk_id, 2, 1, filept) != 1)
			return false; //Read the chunk header
		if(fread (&chunk_len, 4, 1, filept) != 1)
			return false; //Read the lenght of the chunk

		# ifdef GS_USE_TRACE1
		printf("\nChunkID:%x ",chunk_id); 
		printf("Len: %d\n",chunk_len);
		# endif

		switch (chunk_id)
		{
		  //----------------- MAIN3DS -----------------
		  // Description: Main chunk, contains all the other chunks
		  // Chunk ID: 4d4d 
		  // Chunk Lenght: 0 + sub chunks
		  //-------------------------------------------
		  case CHUNK_MAIN: 
		   GS_TRACE1("CHUNK_MAIN...");
		   break;	

		  //----------------- EDIT3DS -----------------
		  // Description: 3D Editor chunk, objects layout info 
		  // Chunk ID: 3d3d (hex)
		  // Chunk Lenght: 0 + sub chunks
		  //-------------------------------------------
		  case CHUNK_OBJMESH:
		   GS_TRACE1("CHUNK_OBJMESH...");
		   break;
			
		  //--------------- EDIT_OBJECT ---------------
		  // Description: Object block, info for each object
		  // Chunk ID: 4000 (hex)
		  // Chunk Lenght: len(object name) + sub chunks
		  //-------------------------------------------
		  case CHUNK_OBJBLOCK: 
		   { GS_TRACE1("CHUNK_OBJBLOCK...");
			 char name[32];
			 i=0;
			 do { if(fread (&uchar, 1, 1, filept) != 1)
				 return false;
				  name[i]=uchar;
				  i++;
				} while ( uchar != '\0' && i<30 );
			 filename=name;
		   } break;

		  //--------------- OBJ_TRIMESH ---------------
		  // Description: Triangular mesh, contains chunks for 3d mesh info
		  // Chunk ID: 4100 (hex)
		  // Chunk Lenght: 0 + sub chunks
		  //-------------------------------------------
		  case CHUNK_TRIMESH:
		   GS_TRACE1("CHUNK_TRIMESH...");
		   break;
			
		  //--------------- TRI_VERTEXL ---------------
		  // Description: Vertices list
		  // Chunk ID: 4110 (hex)
		  // Chunk Lenght: 1 x unsigned short (number of vertices) 
		  //			 + 3 x float (vertex coordinates) x (number of vertices)
		  //			 + sub chunks
		  //-------------------------------------------
		  case CHUNK_VERTLIST: 
		   GS_TRACE1("CHUNK_VERTLIST...");
		   if(fread (&qty, sizeof (unsigned short), 1, filept) != 1)
			   return false;
		   V.size(qty);
		   GS_TRACE1 ( "Number of vertices: " << qty );
		   for (i=0; i<qty; i++)
			{
			  if(fread ( &V[i].x, sizeof(float), 3, filept) != 3)
				  return false;
			}
		   break;

		  //--------------- TRI_FACEL1 ----------------
		  // Description: Polygons (faces) list
		  // Chunk ID: 4120 (hex)
		  // Chunk Lenght: 1 x unsigned short (number of polygons) 
		  //			 + 3 x unsigned short (polygon points) x (number of polygons)
		  //			 + sub chunks
		  //-------------------------------------------
		  case CHUNK_FACELIST:
		  { GS_TRACE1("CHUNK_FACELIST...");
			unsigned short flags; //Flag that stores some face information
			if(fread (&qty, sizeof (unsigned short), 1, filept) != 1)
				return false;
			F.size(qty);
			GS_TRACE1 ( "Number of faces:" << qty );
			unsigned short uc[3];
			for (i=0; i<qty; i++)
			 {
			  if(fread ( uc, sizeof(unsigned short), 3, filept) != 3)
				  return false;
			  F[i].set ( (int)uc[0], (int)uc[1], (int)uc[2] );
			  if(fread ( &flags, sizeof (unsigned short), 1, filept) != 1)
				  return false;
			 }
		  } break;

		  //------------- TRI_MAPPINGCOORS ------------
		  // Description: Vertices list
		  // Chunk ID: 4140 (hex)
		  // Chunk Lenght: 1 x unsigned short (number of mapping points) 
		  //			 + 2 x float (mapping coordinates) x (number of mapping points)
		  //			 + sub chunks
		  //-------------------------------------------
		  case CHUNK_MAPLIST:
		   { GS_TRACE1("CHUNK_MAPLIST...");
			 if(fread (&qty, sizeof (unsigned short), 1, filept) != 1)
				 return false;
			 T.size(qty);
			 for (i=0; i<qty; i++)
			  {
				if(fread ( &T[i].x, sizeof(float), 2, filept) != 2)
					return false;
			  }
		   } break;

		  //----------- Skip unknow chunks ------------
		  //We need to skip all the chunks that currently we don't use
		  //We use the chunk lenght information to set the file pointer
		  //to the same level next chunk
		  //-------------------------------------------
		  default:
		   GS_TRACE1("SKIPPING...");
		   fseek(filept, chunk_len-6, SEEK_CUR);
		} 
	}

   fclose (filept); // Closes the file stream

   if ( name.len()==0 )
	{ name=fname;
	  remove_path ( name );
	  remove_extension ( name );
	}

   validate ();
   compress ();

   GS_TRACE1("Ok!");
   return true;
 }

//============================ EOF ===============================
