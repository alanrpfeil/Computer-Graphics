/*=======================================================================
   Copyright (c) 2018-2019 Marcelo Kallmann.
   This software is distributed under the Apache License, Version 2.0.
   All copies must contain the full copyright notice licence.txt located
   at the base folder of the distribution. 
  =======================================================================*/

# include <sig/gs_material.h>
# include <sig/gs_string.h>

//# define GS_USE_TRACE1 
# include <sig/gs_trace.h>

//================================ GsMaterial =================================================

// Important: Static initializations cannot use other statically-initialized
// variables (like GsVec::i etc.) because order of initialization is unknown

GsColor GsMaterial::DefaultAmbient  (  51,  51,  51, 255 );
GsColor GsMaterial::DefaultDiffuse  ( 204, 204, 204, 255 );
GsColor GsMaterial::DefaultSpecular ( 127, 127, 127, 255 );
GsColor GsMaterial::DefaultEmission (   0,   0,   0, 255 );
float   GsMaterial::DefaultShininess = 10;

GsMaterial::GsMaterial () :
			ambient  ( DefaultAmbient ),
			diffuse  ( DefaultDiffuse ),
			specular ( DefaultSpecular ),
			emission ( DefaultEmission ),
			shininess ( DefaultShininess )
{
}

GsMaterial::GsMaterial ( GsColor dif ) :
			ambient  ( DefaultAmbient ),
			diffuse  ( dif ),
			specular ( DefaultSpecular ),
			emission ( DefaultEmission ),
			shininess ( DefaultShininess )
{
}

GsMaterial::GsMaterial ( GsColor a, GsColor d, GsColor s, GsColor e, float sh ) :
			ambient  ( a ),
			diffuse  ( d ),
			specular ( s ),
			emission ( e ),
			shininess ( DefaultShininess )
{
}

void GsMaterial::init () 
{ 
	ambient = DefaultAmbient;
	diffuse = DefaultDiffuse;
	specular = DefaultSpecular;
	emission = DefaultEmission;
	shininess = DefaultShininess;
}

void GsMaterial::init ( GsColor a, GsColor d, GsColor s, GsColor e, float sh )
{ 
	ambient = a;
	diffuse = d;
	specular = s;
	emission = e;
	shininess = sh;
}

float* GsMaterial::encode_colors ( float vec[12] ) const
{ 
	ambient.get3 (vec);
	diffuse.get3 (vec+3);
	specular.get3 (vec+6);
	emission.get3 (vec+9);
	return vec;
}

float* GsMaterial::encode_params ( float vec[2] ) const
{
	vec[0] = shininess; 
	vec[1] = float(diffuse.a)/255.0f;
	return vec;
}

bool operator == ( const GsMaterial& m1, const GsMaterial& m2 )
{
	return ( m1.ambient==m2.ambient &&
			 m1.diffuse==m2.diffuse &&
			 m1.specular==m2.specular &&
			 m1.emission==m2.emission &&
			 m1.shininess==m2.shininess );
}

bool operator != ( const GsMaterial& m1, const GsMaterial& m2 )
{
	return m1==m2? false:true;
}

GsOutput& operator<< ( GsOutput& o, const GsMaterial& m )
{
	o << "amb " << m.ambient  << 
		 " dif " << m.diffuse  << 
		 " spe " << m.specular << 
		 " emi " << m.emission << 
		 " shi " << m.shininess;
	return o;
}

GsInput& operator>> ( GsInput& in, GsMaterial& m )
{
	# define CHECKNUMBER if ( in.check()!=GsInput::Number ) in.get()
	CHECKNUMBER; in >> m.ambient;
	CHECKNUMBER; in >> m.diffuse;
	CHECKNUMBER; in >> m.specular;
	CHECKNUMBER; in >> m.emission;
	CHECKNUMBER; in >> m.shininess;
	return in;
}

//============================== end of file ===============================
