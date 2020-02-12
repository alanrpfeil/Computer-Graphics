# version 330

in vec3 Norm;
in vec3 Pos;
in vec2 Texc;

out vec4 fColor;

const int ML=4; // max number of lights

uniform vec3[ML] lPos;	   // light position
uniform mat3[ML] lInt;	   // light intensities: ambient, diffuse, and specular
uniform int		 lNum;	  // Number of active lights
uniform vec3[4]  mColors;  // material colors  : ambient, diffuse, specular, and emission 
uniform float[2] mParams;  // material params  : shininess, transparency

uniform int Mode;        // 0:textured, 1:not textured
uniform sampler2D TexId; // diffuse color texture

vec4 shade ( vec3 p, vec3 n, vec3 lp, vec3 la, vec3 ld, vec3 ls, vec3 ka, vec3 kd, vec3 ks, vec3 emi, float sh, float alpha );

void main()
{
	vec3 kd;
	float alpha;

	// When condition is uniform (cte for the entire pass) branching is free
	// because specific versions of the shader should be compiled.
	if ( Mode==0 )
	{	vec4 tex = texture2D ( TexId, Texc );
		kd = tex.rgb;
		alpha = tex.a;
	}
	else
	{	kd = mColors[1];
		alpha = mParams[1];
	}

	// fColor = shade ( Pos, Norm, lPos, lInt[0], lInt[1], lInt[2], mColors[0], kd, mColors[2], mColors[3], mParams[0], alpha );

	fColor = vec4(0,0,0,0);
	int i=0;
	if ( i<lNum ) { fColor += shade ( Pos, Norm, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], kd, mColors[2], mColors[3], mParams[0], alpha ); i++; }
	if ( i<lNum ) { fColor += shade ( Pos, Norm, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], kd, mColors[2], mColors[3], mParams[0], alpha ); i++; }
	if ( i<lNum ) { fColor += shade ( Pos, Norm, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], kd, mColors[2], mColors[3], mParams[0], alpha ); i++; }
	if ( i<lNum ) { fColor += shade ( Pos, Norm, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], kd, mColors[2], mColors[3], mParams[0], alpha ); i++; }
}
