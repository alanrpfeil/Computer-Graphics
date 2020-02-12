/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_light.h>

# include <string.h>

//=============================== GsLight ================================

GsLight::GsLight ()
{
	init ();
}

void GsLight::init ()
{
	ambient = GsColor::darkgray;
	diffuse = GsColor::white;
	specular = GsColor::white;
	position.set ( 0, 0, 1 );
}

float* GsLight::encode_intensities ( float vec[9] ) const
{ 
	ambient.get3 (vec);
	diffuse.get3 (vec+3);
	specular.get3 (vec+6);
	return vec;
}

void encode_positions ( const GsLight* l, int n, float* buffer )
{
	for ( int i=0; i<n; i++ )
	{	memcpy ( buffer, l[i].position.e, sizeof(float)*3 );
		buffer+=sizeof(float)*3;
	}
}

void encode_intensities ( const GsLight* l, int n, float* buffer )
{
	for ( int i=0; i<n; i++ )
	{	l[i].encode_intensities ( buffer );
		buffer+=sizeof(float)*9;
	}
}

// Possible extensions to be added could follow earlier OpenGL specifications:

/*
   spot_exponent = 0;
   spot_direction.set ( 0.0f, 0.0f, -1.0f );
   spot_cutoff = 180;
   constant_attenuation = 1.0f;
   linear_attenuation = 0;
   quadratic_attenuation = 0;
   directional = true;

*/
	/*! Higher spot exponents result in a more focused light source. The default 
		spot exponent is 0, resulting in uniform light distribution. Values must
		be in the range of [0,128]. */
//	float spot_exponent;

	/*! The direction of the light in homogeneous object coordinates. The spot 
		direction is transformed by the inverse of the modelview matrix when 
		glLight is called (just as if it was a normal), and it is stored in 
		eye coordinates. It is significant only when GL_SPOT_CUTOFF is not 
		180, which it is by default. The default direction is (0,0,-1). */
//	GsVec spot_direction;

	/*! Specifies the maximum spread angle. Only values in the range [0,90], 
		and the special value 180, are accepted. If the angle between the 
		direction of the light and the direction from the light to the vertex
		being lighted is greater than the spot cutoff angle, then the light 
		is completely makned. Otherwise, its intensity is controlled by the 
		spot exponent and the attenuation factors. The default spot cutoff 
		is 180, resulting in uniform light distribution. */
//	float spot_cutoff;

	/*! If the light is positional, rather than directional, its intensity 
		is attenuated by the reciprocal of the sum of: the constant factor,
		the linear factor multiplied by the distance between the light and
		the vertex being lighted, and the quadratic factor multiplied by 
		the square of the same distance. The default attenuation factors
		are cte=1, linear=0, quad=0, resulting in no attenuation. Only 
		nonnegative values are accepted. */
//	float constant_attenuation;
//	float linear_attenuation;	 //!< See constant_attenuation. 
//	float quadratic_attenuation;  //!< See constant_attenuation. 

	/*! When true means that the position w coord is 0, otherwise 1. Default is true. */
//	bool directional;

	/*! The position is transformed by the modelview matrix when glLight is
		called (just as if it was a point), and it is stored in eye 
		coordinates. If directional, diffuse and specular lighting calculations
		take the lights direction, but not its actual position, into account,
		and attenuation is disabled. Otherwise, diffuse and specular lighting 
		calculations are based on the actual location of the light in eye
		coordinates, and attenuation is enabled. The default position is (0,0,1). */
//	GsVec position;

//================================ End of File =========================================
