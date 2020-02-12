/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

/** \file gs_material.h
 * specifies a material
 */

# ifndef GS_MATERIAL_H
# define GS_MATERIAL_H

# include <sig/gs_input.h> 
# include <sig/gs_output.h> 

# include <sig/gs_color.h>

/*! \class GsMaterial gs_material.h
	\brief specifies a material as defined by OpenGL

	Defines a material. */
class GsMaterial
{  public : // user-modifiable default values for new materials
	static GsColor DefaultAmbient;   //!< default is 51,51,51,255 (equivalent to .2,.2,.2,1)
	static GsColor DefaultDiffuse;   //!< default is 204,204,204,255 (equivalent to: .8,.8,.8,1)
	static GsColor DefaultSpecular;  //!< default is 127,127,127,255 (equivalent to: .5,.5,.5,1)
	static GsColor DefaultEmission;  //!< default is 0,0,0,255 (equivalent to: 0,0,0,1)
	static float   DefaultShininess; //!< default is 10, can be in: [0,128]

   public : // material parameters:
	GsColor ambient;
	GsColor diffuse;
	GsColor specular;
	GsColor emission;
	float   shininess;

   public :
	
	/*! Initializes with the default values. */
	GsMaterial ();

	/*! Initializes with given diffuse color and with default values for al other parameters. */
	GsMaterial ( GsColor dif );

	/*! Initializes with given values. */
	GsMaterial ( GsColor a, GsColor d, GsColor s=DefaultSpecular, GsColor e=DefaultEmission, float sh=DefaultShininess );

	/*! Set parameters as their default values. */
	void init ();

	/*! Set values. */
	void init ( GsColor a, GsColor d, GsColor s=DefaultSpecular, GsColor e=DefaultEmission, float sh=DefaultShininess );

	/*! Encodes the 4 colors of the material as 4 rgb vectors with 
		components in [0,1]. A pointer to vec is then returned. */
	float* encode_colors ( float vec[12] ) const;

	/*! Encodes in vec the shininess and the transparency 
		(diffuse.a in range [0,1]). A pointer to vec is then returned. */
	float* encode_params ( float vec[2] ) const;

	/*! Exact comparison operator == */
	friend bool operator == ( const GsMaterial& m1, const GsMaterial& m2 );

	/*! Exact comparison operator != */
	friend bool operator != ( const GsMaterial& m1, const GsMaterial& m2 );

	/*! Outputs in format: "amb r g b a  dif r g b a  spe r g b a  emi r g b a  shi s" */
	friend GsOutput& operator<< ( GsOutput& o, const GsMaterial& m );

	/*! Inputs from the output format, the keywords may be omitted but all parameters must be defined */
	friend GsInput& operator>> ( GsInput& in, GsMaterial& m );
};

//============================== end of file ===================================

# endif // GS_MATERIAL_H
